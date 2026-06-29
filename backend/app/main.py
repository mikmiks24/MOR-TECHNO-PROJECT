from __future__ import annotations

import asyncio
import json
import uuid
from typing import Any

import httpx
from fastapi import Depends, FastAPI, File, Form, HTTPException, UploadFile, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import HTMLResponse
import cv2
import numpy as np
import os
from pathlib import Path

from .config import settings
from .database import (
    get_connection,
    hash_password,
    initialize_database,
    row_to_dict,
    rows_to_dicts,
    utc_now,
    to_local_iso,
)
from .gemini_client import GeminiNotConfiguredError, analyze_image
from .schemas import (
    AnalyzeFromCameraRequest,
    AssignmentCreate,
    EmployeeCreate,
    InspectionCreate,
    LoginRequest,
    ReceiveLogCreate,
    ReleaseLogCreate,
    RfidVerifyRequest,
    VisualAnalysisResponse,
    VisualMonitorStartRequest,
    TransactionCreate,
)


app = FastAPI(title=settings.app_name)

visual_monitor_state: dict[str, Any] = {
    "running": False,
    "task": None,
    "started_at": None,
    "stopped_at": None,
    "last_run_at": None,
    "last_success_at": None,
    "last_error": None,
    "cycles_completed": 0,
    "last_analysis_id": None,
    "config": None,
}

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origins,
    allow_credentials=False,
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.middleware("http")
async def add_private_network_headers(request: Request, call_next):
    if request.method == "OPTIONS":
        response = await call_next(request)
        response.headers["Access-Control-Allow-Private-Network"] = "true"
        response.headers["Access-Control-Allow-Headers"] = "*"
        return response
    response = await call_next(request)
    return response


@app.on_event("startup")
def startup() -> None:
    initialize_database()
    settings.evidence_dir.mkdir(parents=True, exist_ok=True)


@app.on_event("shutdown")
async def shutdown() -> None:
    task = visual_monitor_state.get("task")
    visual_monitor_state["running"] = False
    if task:
        task.cancel()
        try:
            await task
        except asyncio.CancelledError:
            pass


def get_db():
    connection = get_connection()
    try:
        yield connection
    finally:
        connection.close()


def scalar(connection, query: str, params: tuple[Any, ...] = ()) -> Any:
    row = connection.execute(query, params).fetchone()
    if row is None:
        return None
    return row[0]


def add_event(
    connection,
    *,
    event_type: str,
    message: str,
    actor: str | None = None,
    serial_number: str | None = None,
    station_id: str | None = None,
) -> None:
    connection.execute(
        """
        INSERT INTO events (event_type, message, actor, serial_number, station_id, created_at)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        (event_type, message, actor, serial_number, station_id, utc_now()),
    )


def upsert_terminal(
    connection,
    *,
    serial_number: str,
    terminal_model: str,
    brand: str,
    status: str,
    current_station: str | None,
    last_handled_by: str | None,
) -> None:
    now = utc_now()
    connection.execute(
        """
        INSERT INTO terminals (
            serial_number, terminal_model, brand, status, current_station,
            last_handled_by, created_at, updated_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(serial_number) DO UPDATE SET
            terminal_model = excluded.terminal_model,
            brand = excluded.brand,
            status = excluded.status,
            current_station = excluded.current_station,
            last_handled_by = excluded.last_handled_by,
            updated_at = excluded.updated_at
        """,
        (serial_number, terminal_model, brand, status, current_station, last_handled_by, now, now),
    )


def filter_rows(rows: list[dict[str, Any]], query: str | None) -> list[dict[str, Any]]:
    if not query:
        return rows
    needle = query.lower()
    return [row for row in rows if needle in json.dumps(row, default=str).lower()]


def normalize_supabase_url(url: str) -> str:
    normalized = url.strip().rstrip("/")
    if normalized.endswith("/rest/v1"):
        normalized = normalized[: -len("/rest/v1")]
    return normalized.rstrip("/")


def supabase_storage_enabled() -> bool:
    return bool(settings.supabase_url and settings.supabase_service_role_key and settings.supabase_storage_bucket)


def save_evidence(image_bytes: bytes, extension: str = ".jpg") -> str:
    settings.evidence_dir.mkdir(parents=True, exist_ok=True)
    safe_extension = extension if extension.startswith(".") else f".{extension}"
    filename = f"{utc_now().replace(':', '').replace('+', 'Z')}_{uuid.uuid4().hex}{safe_extension}"
    path = settings.evidence_dir / filename
    path.write_bytes(image_bytes)
    return str(path)


async def upload_evidence_to_supabase_storage(
    image_bytes: bytes,
    *,
    extension: str,
    mime_type: str,
) -> str:
    safe_extension = extension if extension.startswith(".") else f".{extension}"
    object_path = f"visual-analyses/{utc_now()[:10]}/{uuid.uuid4().hex}{safe_extension}"
    base_url = normalize_supabase_url(settings.supabase_url)
    upload_url = f"{base_url}/storage/v1/object/{settings.supabase_storage_bucket}/{object_path}"

    headers = {
        "apikey": settings.supabase_service_role_key,
        "Authorization": f"Bearer {settings.supabase_service_role_key}",
        "Content-Type": mime_type,
        "x-upsert": "false",
    }

    try:
        async with httpx.AsyncClient(timeout=30) as client:
            response = await client.post(upload_url, content=image_bytes, headers=headers)
            response.raise_for_status()
    except httpx.HTTPStatusError as error:
        raise HTTPException(
            status_code=502,
            detail=(
                "Supabase Storage upload failed. Check SUPABASE_URL, "
                "SUPABASE_SERVICE_ROLE_KEY, bucket name, and bucket policies. "
                f"Supabase returned HTTP {error.response.status_code}: {error.response.text}"
            ),
        ) from error
    except httpx.HTTPError as error:
        raise HTTPException(status_code=502, detail=f"Supabase Storage upload failed: {error}") from error

    if settings.supabase_storage_public:
        return f"{base_url}/storage/v1/object/public/{settings.supabase_storage_bucket}/{object_path}"

    return f"supabase://{settings.supabase_storage_bucket}/{object_path}"


async def store_evidence(image_bytes: bytes, *, extension: str, mime_type: str) -> str:
    if supabase_storage_enabled():
        return await upload_evidence_to_supabase_storage(
            image_bytes,
            extension=extension,
            mime_type=mime_type,
        )

    return save_evidence(image_bytes, extension)


def recommended_status(parsed_result: dict[str, Any] | None) -> str | None:
    if not parsed_result:
        return None
    status = parsed_result.get("recommended_status")
    if isinstance(status, str):
        return status.lower()
    return None


def visual_monitor_status() -> dict[str, Any]:
    task = visual_monitor_state.get("task")
    return {
        "running": visual_monitor_state["running"],
        "started_at": visual_monitor_state["started_at"],
        "stopped_at": visual_monitor_state["stopped_at"],
        "last_run_at": visual_monitor_state["last_run_at"],
        "last_success_at": visual_monitor_state["last_success_at"],
        "last_error": visual_monitor_state["last_error"],
        "cycles_completed": visual_monitor_state["cycles_completed"],
        "last_analysis_id": visual_monitor_state["last_analysis_id"],
        "config": visual_monitor_state["config"],
        "task_done": task.done() if task else True,
    }


def enhance_image(image_bytes: bytes) -> bytes:
    """
    Applies OpenCV CLAHE contrast enhancement and detail sharpening.
    Saves both the original and enhanced images locally to `scans/`
    for comparison and verification.
    """
    # 1. Decode JPEG bytes into OpenCV BGR image
    nparr = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    if img is None:
        return image_bytes

    # 2. Convert to LAB color space to isolate brightness (L channel)
    lab = cv2.cvtColor(img, cv2.COLOR_BGR2LAB)
    l_channel, a_channel, b_channel = cv2.split(lab)

    # 3. Apply CLAHE (Contrast Limited Adaptive Histogram Equalization)
    clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8, 8))
    cl = clahe.apply(l_channel)

    # 4. Merge enhanced L channel back and convert to BGR
    limg = cv2.merge((cl, a_channel, b_channel))
    enhanced_img = cv2.cvtColor(limg, cv2.COLOR_LAB2BGR)

    # 5. Apply details sharpening filter
    # Sharpening kernel
    kernel = np.array([
        [ 0, -1,  0],
        [-1,  5, -1],
        [ 0, -1,  0]
    ])
    sharpened_img = cv2.filter2D(enhanced_img, -1, kernel)

    # 6. Save original and enhanced versions to scans/ directory
    try:
        scans_dir = Path("scans")
        scans_dir.mkdir(parents=True, exist_ok=True)
        scan_id = uuid.uuid4().hex[:8]
        cv2.imwrite(str(scans_dir / f"scan_{scan_id}_original.jpg"), img)
        cv2.imwrite(str(scans_dir / f"scan_{scan_id}_enhanced.jpg"), sharpened_img)
    except Exception as e:
        print(f"Error saving debug scans: {e}")

    # 7. Re-encode as JPEG bytes
    success, encoded_img = cv2.imencode(".jpg", sharpened_img, [cv2.IMWRITE_JPEG_QUALITY, 95])
    if success:
        return encoded_img.tobytes()
    return image_bytes


COCO_CLASSES = [
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat",
    "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat",
    "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack",
    "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball",
    "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket",
    "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake",
    "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop",
    "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
    "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"
]


def download_yolo_files() -> tuple[str, str]:
    models_dir = Path("models")
    models_dir.mkdir(parents=True, exist_ok=True)
    
    cfg_path = models_dir / "yolov4-tiny.cfg"
    weights_path = models_dir / "yolov4-tiny.weights"
    
    cfg_url = "https://raw.githubusercontent.com/AlexeyAB/darknet/master/cfg/yolov4-tiny.cfg"
    weights_url = "https://github.com/AlexeyAB/darknet/releases/download/darknet_yolo_v4_pre/yolov4-tiny.weights"
    
    import urllib.request
    if not cfg_path.exists():
        print("[YOLO] Downloading yolov4-tiny.cfg...")
        urllib.request.urlretrieve(cfg_url, str(cfg_path))
    if not weights_path.exists():
        print("[YOLO] Downloading yolov4-tiny.weights (approx. 23MB)...")
        urllib.request.urlretrieve(weights_url, str(weights_path))
        
    return str(cfg_path), str(weights_path)


def detect_objects_yolo(image_bytes: bytes) -> tuple[list[str], bytes | None]:
    try:
        cfg_path, weights_path = download_yolo_files()
    except Exception as e:
        print(f"[YOLO] Error downloading YOLO model files: {e}")
        return [], None

    # Decode image bytes
    nparr = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    if img is None:
        return [], None

    # Load YOLO network
    try:
        net = cv2.dnn.readNetFromDarknet(cfg_path, weights_path)
        net.setPreferableBackend(cv2.dnn.DNN_BACKEND_OPENCV)
        net.setPreferableTarget(cv2.dnn.DNN_TARGET_CPU)
    except Exception as e:
        print(f"[YOLO] Error loading YOLO network: {e}")
        return [], None

    # Determine output layer names
    ln = net.getLayerNames()
    try:
        out_layers = [ln[i - 1] for i in net.getUnconnectedOutLayers()]
    except Exception:
        out_layers = [ln[i[0] - 1] for i in net.getUnconnectedOutLayers()]

    # Construct a blob from the image
    h, w = img.shape[:2]
    blob = cv2.dnn.blobFromImage(img, 1.0 / 255.0, (416, 416), swapRB=True, crop=False)
    net.setInput(blob)
    layer_outputs = net.forward(out_layers)

    boxes = []
    confidences = []
    class_ids = []

    # Parse detections
    for output in layer_outputs:
        for detection in output:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]
            if confidence > 0.25:
                # Scale box coordinates back to image dimensions
                box = detection[0:4] * np.array([w, h, w, h])
                (centerX, centerY, width, height) = box.astype("int")
                x = int(centerX - (width / 2))
                y = int(centerY - (height / 2))
                boxes.append([x, y, int(width), int(height)])
                confidences.append(float(confidence))
                class_ids.append(class_id)

    # Apply Non-Maximum Suppression (NMS)
    indices = cv2.dnn.NMSBoxes(boxes, confidences, 0.25, 0.4)
    detected_objects = []

    def map_yolo_to_checklist(label: str) -> list[str]:
        if label in ("cell phone", "remote"):
            return ["POS Terminal", "POS Battery"]
        if label in ("tv", "laptop", "keyboard"):
            return ["POS Terminal"]
        if label in ("mouse", "scissors"):
            return ["LAN Cable", "POS Power Supply"]
        if label in ("backpack", "handbag", "suitcase", "book"):
            return ["Packaging / box"]
        return [label]

    def get_display_label(checklist_label: str) -> str:
        return checklist_label

    # Compile detected class names and draw debug bounding boxes
    if len(indices) > 0:
        indices_flat = indices.flatten() if hasattr(indices, "flatten") else indices
        for i in indices_flat:
            class_id = class_ids[i]
            label = COCO_CLASSES[class_id]
            
            # Map COCO label to checklist labels
            mapped_items = map_yolo_to_checklist(label)
            detected_objects.extend(mapped_items)
            
            # Form display label for box drawing
            display_label = get_display_label(mapped_items[0]) if mapped_items else label
            
            # Draw box on image
            (x, y) = (boxes[i][0], boxes[i][1])
            (w_box, h_box) = (boxes[i][2], boxes[i][3])
            cv2.rectangle(img, (x, y), (x + w_box, y + h_box), (0, 255, 0), 2)
            cv2.putText(img, f"{display_label}: {confidences[i]:.2f}", (x, y - 5),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

    # Save the local detection debug image to scans/
    annotated_bytes = None
    try:
        scans_dir = Path("scans")
        scans_dir.mkdir(parents=True, exist_ok=True)
        cv2.imwrite(str(scans_dir / f"local_detect_yolo.jpg"), img)
        success, encoded_img = cv2.imencode(".jpg", img, [cv2.IMWRITE_JPEG_QUALITY, 90])
        if success:
            annotated_bytes = encoded_img.tobytes()
    except Exception as e:
        print(f"[YOLO] Error saving local detection debug image: {e}")

    return list(set(detected_objects)), annotated_bytes


def detect_objects_yolo_boxes(image_bytes: bytes) -> list[dict[str, Any]]:
    try:
        cfg_path, weights_path = download_yolo_files()
    except Exception as e:
        print(f"[YOLO] Error downloading YOLO model files: {e}")
        return []

    # Decode image bytes
    nparr = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    if img is None:
        return []

    # Load YOLO network
    try:
        net = cv2.dnn.readNetFromDarknet(cfg_path, weights_path)
        net.setPreferableBackend(cv2.dnn.DNN_BACKEND_OPENCV)
        net.setPreferableTarget(cv2.dnn.DNN_TARGET_CPU)
    except Exception as e:
        print(f"[YOLO] Error loading YOLO network: {e}")
        return []

    # Determine output layer names
    ln = net.getLayerNames()
    try:
        out_layers = [ln[i - 1] for i in net.getUnconnectedOutLayers()]
    except Exception:
        out_layers = [ln[i[0] - 1] for i in net.getUnconnectedOutLayers()]

    # Construct a blob from the image
    h, w = img.shape[:2]
    blob = cv2.dnn.blobFromImage(img, 1.0 / 255.0, (416, 416), swapRB=True, crop=False)
    net.setInput(blob)
    layer_outputs = net.forward(out_layers)

    boxes = []
    confidences = []
    class_ids = []

    # Parse detections
    for output in layer_outputs:
        for detection in output:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]
            if confidence > 0.25:
                # Scale box coordinates back to image dimensions
                box = detection[0:4] * np.array([w, h, w, h])
                (centerX, centerY, width, height) = box.astype("int")
                x = int(centerX - (width / 2))
                y = int(centerY - (height / 2))
                boxes.append([x, y, int(width), int(height)])
                confidences.append(float(confidence))
                class_ids.append(class_id)

    # Apply Non-Maximum Suppression (NMS)
    indices = cv2.dnn.NMSBoxes(boxes, confidences, 0.25, 0.4)
    results = []

    def map_yolo_to_checklist(label: str) -> list[str]:
        if label in ("cell phone", "remote"):
            return ["POS Terminal", "POS Battery"]
        if label in ("tv", "laptop", "keyboard"):
            return ["POS Terminal"]
        if label in ("mouse", "scissors"):
            return ["LAN Cable", "POS Power Supply"]
        if label in ("backpack", "handbag", "suitcase", "book"):
            return ["Packaging / box"]
        return [label]

    def get_display_label(checklist_label: str) -> str:
        return checklist_label

    if len(indices) > 0:
        indices_flat = indices.flatten() if hasattr(indices, "flatten") else indices
        for i in indices_flat:
            class_id = class_ids[i]
            label = COCO_CLASSES[class_id]
            mapped_items = map_yolo_to_checklist(label)
            display_label = get_display_label(mapped_items[0]) if mapped_items else label
            
            box = boxes[i]
            results.append({
                "label": display_label,
                "confidence": confidences[i],
                "box": [box[0], box[1], box[2], box[3]],
                "image_width": w,
                "image_height": h
            })

    return results


async def fetch_camera_image(capture_url: str) -> tuple[bytes, str]:
    try:
        async with httpx.AsyncClient(timeout=20) as client:
            response = await client.get(capture_url)
            response.raise_for_status()
    except httpx.HTTPError as error:
        raise HTTPException(
            status_code=502,
            detail=f"Could not fetch image from ESP32-CAM capture URL: {error}",
        ) from error

    mime_type = response.headers.get("content-type", "image/jpeg").split(";")[0]
    return response.content, mime_type


async def run_visual_analysis(
    *,
    image_bytes: bytes,
    mime_type: str,
    source: str,
    serial_number: str | None,
    station_id: str | None,
    prompt: str | None,
    connection,
) -> VisualAnalysisResponse:
    extension = ".jpg" if "jpeg" in mime_type or "jpg" in mime_type else ".png"
    image_path = await store_evidence(image_bytes, extension=extension, mime_type=mime_type)

    # 1. Run local object detection
    local_objects = []
    annotated_bytes = None
    try:
        local_objects, annotated_bytes = detect_objects_yolo(image_bytes)
        print(f"[YOLO] Local objects detected: {local_objects}")
    except Exception as e:
        print(f"[YOLO] Error running local YOLO: {e}")

    annotated_base64 = None
    if annotated_bytes:
        import base64
        annotated_base64 = "data:image/jpeg;base64," + base64.b64encode(annotated_bytes).decode("ascii")

    # 2. Try Gemini analysis
    try:
        result_text, parsed_result = await analyze_image(
            image_bytes,
            mime_type=mime_type,
            prompt=prompt,
        )
        
        # Combine local YOLO objects with Gemini objects for maximum reliability
        if parsed_result and "objects" in parsed_result:
            gemini_objs = parsed_result["objects"] or []
            parsed_result["objects"] = list(set(gemini_objs + local_objects))

    except Exception as error:
        # 3. Fallback to local YOLO detection if Gemini API fails
        print(f"[Gemini] API failed: {error}. Falling back to local YOLO.")
        parsed_result = {
            "objects": list(set(local_objects)),
            "recommended_status": "review",
            "notes": f"Offline local detection fallback used. Gemini API error: {str(error)}"
        }
        result_text = f"Local YOLO Offline Fallback (Gemini API was down)"

    now = utc_now()
    status = recommended_status(parsed_result)
    cursor = connection.execute(
        """
        INSERT INTO visual_analyses (
            serial_number, station_id, source, image_path, model, prompt,
            result_text, result_json, recommended_status, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """,
        (
            serial_number,
            station_id,
            source,
            image_path,
            settings.gemini_model,
            prompt or "",
            result_text,
            json.dumps(parsed_result) if parsed_result else None,
            status,
            now,
        ),
    )
    add_event(
        connection,
        event_type="visual_analysis",
        message=f"AI visual analysis completed for {serial_number or 'unregistered terminal'}",
        actor="Gemini",
        serial_number=serial_number,
        station_id=station_id,
    )
    connection.commit()

    return VisualAnalysisResponse(
        id=cursor.lastrowid,
        status=status or "review",
        source=source,
        model=settings.gemini_model,
        result_text=result_text,
        parsed_result=parsed_result,
        image_path=image_path,
        serial_number=serial_number,
        station_id=station_id,
        annotated_image=annotated_base64,
    )


async def visual_monitor_loop(config: VisualMonitorStartRequest) -> None:
    capture_url = config.capture_url or settings.esp32_capture_url
    max_cycles = config.max_cycles

    try:
        while visual_monitor_state["running"]:
            visual_monitor_state["last_run_at"] = utc_now()

            try:
                image_bytes, mime_type = await fetch_camera_image(capture_url)
                connection = get_connection()
                try:
                    result = await run_visual_analysis(
                        image_bytes=image_bytes,
                        mime_type=mime_type,
                        source="esp32-cam-auto",
                        serial_number=config.serial_number,
                        station_id=config.station_id,
                        prompt=config.prompt,
                        connection=connection,
                    )
                finally:
                    connection.close()

                visual_monitor_state["last_success_at"] = utc_now()
                visual_monitor_state["last_error"] = None
                visual_monitor_state["last_analysis_id"] = result.id
            except Exception as error:  # Keep the monitor alive after transient camera/network failures.
                visual_monitor_state["last_error"] = str(error)

            visual_monitor_state["cycles_completed"] += 1
            if max_cycles and visual_monitor_state["cycles_completed"] >= max_cycles:
                break

            await asyncio.sleep(config.interval_seconds)
    finally:
        visual_monitor_state["running"] = False
        visual_monitor_state["task"] = None
        visual_monitor_state["stopped_at"] = utc_now()


@app.get("/")
def root() -> dict[str, str]:
    return {
        "name": settings.app_name,
        "docs": "/docs",
        "health": "/api/health",
        "workstation": "/workstation",
        "dashboard": "/dashboard",
    }


@app.get("/workstation", response_class=HTMLResponse)
def serve_workstation():
    path = Path(__file__).resolve().parents[2] / "INDEX_HTML.h"
    if path.exists():
        content = path.read_text(encoding="utf-8")
        start = content.find('R"rawliteral(')
        end = content.rfind(')rawliteral"')
        if start != -1 and end != -1:
            html = content[start + len('R"rawliteral('):end]
            cam_url = settings.esp32_capture_url.replace('/capture', '/stream')
            html = html.replace('__CAM_URL__', cam_url)
            return HTMLResponse(content=html)
    return HTMLResponse(content="<h1>Workstation HTML not found</h1>")


@app.get("/dashboard", response_class=HTMLResponse)
def serve_dashboard():
    path = Path(__file__).resolve().parents[2] / "dashboard.html"
    if path.exists():
        return HTMLResponse(content=path.read_text(encoding="utf-8"))
    return HTMLResponse(content="<h1>Dashboard HTML not found</h1>")


@app.get("/api/health")
def health() -> dict[str, Any]:
    return {
        "status": "ok",
        "gemini_configured": bool(settings.gemini_api_key),
        "database_backend": settings.database_backend,
        "database": str(settings.database_path),
        "supabase_configured": bool(settings.supabase_db_url),
        "supabase_storage_configured": supabase_storage_enabled(),
        "supabase_storage_bucket": settings.supabase_storage_bucket,
    }


@app.post("/api/auth/login")
def login(payload: LoginRequest, connection=Depends(get_db)) -> dict[str, Any]:
    user = connection.execute(
        "SELECT * FROM employees WHERE username = ? AND active = 1",
        (payload.username,),
    ).fetchone()

    if not user or user["password_hash"] != hash_password(payload.password):
        raise HTTPException(status_code=401, detail="Invalid username or password")

    return {
        "status": "ok",
        "user": {
            "id": user["id"],
            "name": user["name"],
            "role": user["role"],
            "username": user["username"],
        },
    }


@app.post("/api/rfid/verify")
def verify_rfid(payload: RfidVerifyRequest, connection=Depends(get_db)) -> dict[str, Any]:
    employee = connection.execute(
        "SELECT id, name, role, rfid_uid FROM employees WHERE rfid_uid = ? AND active = 1",
        (payload.rfid_uid,),
    ).fetchone()

    if not employee:
        add_event(
            connection,
            event_type="rfid_denied",
            message=f"Access denied for RFID {payload.rfid_uid}",
            station_id=payload.station_id,
        )
        connection.commit()
        raise HTTPException(status_code=403, detail="RFID card is not registered or inactive")

    add_event(
        connection,
        event_type="rfid_verified",
        message=f"{employee['name']} authenticated via RFID",
        actor=employee["name"],
        station_id=payload.station_id,
    )
    connection.commit()
    return {"status": "verified", "employee": row_to_dict(employee)}


@app.get("/api/employees")
def list_employees(q: str | None = None, connection=Depends(get_db)) -> list[dict[str, Any]]:
    rows = rows_to_dicts(
        connection.execute(
            """
            SELECT id, name, role, username, rfid_uid, active, created_at
            FROM employees
            ORDER BY name ASC
            """
        ).fetchall()
    )
    return filter_rows(rows, q)


@app.post("/api/employees")
def create_employee(payload: EmployeeCreate, connection=Depends(get_db)) -> dict[str, Any]:
    existing = connection.execute(
        "SELECT id, name, rfid_uid FROM employees WHERE rfid_uid = ?",
        (payload.rfid_uid,),
    ).fetchone()
    if existing:
        raise HTTPException(
            status_code=409,
            detail=f"RFID UID is already registered to {existing['name']}",
        )

    if payload.username:
        username_exists = connection.execute(
            "SELECT id FROM employees WHERE username = ?",
            (payload.username,),
        ).fetchone()
        if username_exists:
            raise HTTPException(status_code=409, detail="Username is already registered")

    password_hash = hash_password(payload.password) if payload.password else None
    cursor = connection.execute(
        """
        INSERT INTO employees (name, role, username, password_hash, rfid_uid, active, created_at)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        """,
        (
            payload.name,
            payload.role,
            payload.username,
            password_hash,
            payload.rfid_uid,
            payload.active,
            utc_now(),
        ),
    )
    add_event(
        connection,
        event_type="employee_registered",
        message=f"{payload.name} registered with RFID {payload.rfid_uid}",
        actor=payload.name,
    )
    connection.commit()
    return {"id": cursor.lastrowid, "status": "created"}


@app.get("/api/dashboard")
def dashboard(connection=Depends(get_db)) -> dict[str, Any]:
    total_received = scalar(connection, "SELECT COUNT(*) FROM receive_logs") or 0
    total_released = scalar(connection, "SELECT COUNT(*) FROM release_logs") or 0
    in_queue = (
        scalar(
            connection,
            """
            SELECT COUNT(*) FROM terminals
            WHERE status IN ('In Queue', 'Inspecting', 'Inspection', 'Received')
            """,
        )
        or 0
    )
    defects_today = (
        scalar(
            connection,
            """
            SELECT COUNT(*) FROM inspection_records
            WHERE lower(remarks) = 'defect' AND date(inspected_at) = date('now')
            """,
        )
        or 0
    )

    received_volume = rows_to_dicts(
        connection.execute(
            """
            SELECT substr(received_at, 1, 10) AS day, COUNT(*) AS count
            FROM receive_logs
            GROUP BY day
            ORDER BY day DESC
            LIMIT 6
            """
        ).fetchall()
    )
    released_volume = rows_to_dicts(
        connection.execute(
            """
            SELECT substr(released_at, 1, 10) AS day, COUNT(*) AS count
            FROM release_logs
            GROUP BY day
            ORDER BY day DESC
            LIMIT 6
            """
        ).fetchall()
    )
    all_days = sorted({row["day"] for row in received_volume + released_volume})
    received_by_day = {row["day"]: row["count"] for row in received_volume}
    released_by_day = {row["day"]: row["count"] for row in released_volume}

    anomalies = rows_to_dicts(
        connection.execute(
            """
            SELECT remarks AS label, COUNT(*) AS count
            FROM inspection_records
            WHERE remarks IN ('Missing', 'Defect')
            GROUP BY remarks
            ORDER BY remarks
            """
        ).fetchall()
    )

    recent_activity = rows_to_dicts(
        connection.execute(
            "SELECT * FROM events ORDER BY datetime(created_at) DESC, id DESC LIMIT 8"
        ).fetchall()
    )

    return {
        "metrics": {
            "total_received": total_received,
            "in_queue": in_queue,
            "defects_today": defects_today,
            "total_released": total_released,
        },
        "charts": {
            "processing_volume": {
                "labels": all_days,
                "received": [received_by_day.get(day, 0) for day in all_days],
                "released": [released_by_day.get(day, 0) for day in all_days],
            },
            "anomaly_detection": {
                "labels": [row["label"] for row in anomalies],
                "values": [row["count"] for row in anomalies],
            },
        },
        "recent_activity": recent_activity,
    }


@app.get("/api/terminals")
def list_terminals(q: str | None = None, connection=Depends(get_db)) -> list[dict[str, Any]]:
    rows = rows_to_dicts(
        connection.execute("SELECT * FROM terminals ORDER BY datetime(updated_at) DESC").fetchall()
    )
    return filter_rows(rows, q)


@app.get("/api/receive-logs")
def list_receive_logs(q: str | None = None, connection=Depends(get_db)) -> list[dict[str, Any]]:
    rows = rows_to_dicts(
        connection.execute("SELECT * FROM receive_logs ORDER BY datetime(received_at) DESC").fetchall()
    )
    return filter_rows(rows, q)


@app.post("/api/receive-logs")
def create_receive_log(payload: ReceiveLogCreate, connection=Depends(get_db)) -> dict[str, Any]:
    received_at = to_local_iso(payload.received_at)
    now = utc_now()
    upsert_terminal(
        connection,
        serial_number=payload.serial_number,
        terminal_model=payload.terminal_model,
        brand=payload.brand,
        status=payload.status,
        current_station=payload.station_id,
        last_handled_by=payload.received_by,
    )
    cursor = connection.execute(
        """
        INSERT INTO receive_logs (
            serial_number, terminal_model, brand, received_by, source, station_id,
            status, received_at, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """,
        (
            payload.serial_number,
            payload.terminal_model,
            payload.brand,
            payload.received_by,
            payload.source,
            payload.station_id,
            payload.status,
            received_at,
            now,
        ),
    )
    add_event(
        connection,
        event_type="receive",
        message=f"Terminal {payload.brand} {payload.terminal_model} ({payload.serial_number}) received by {payload.received_by}",
        actor=payload.received_by,
        serial_number=payload.serial_number,
        station_id=payload.station_id,
    )
    connection.commit()
    return {"id": cursor.lastrowid, "status": "created"}


@app.get("/api/release-logs")
def list_release_logs(q: str | None = None, connection=Depends(get_db)) -> list[dict[str, Any]]:
    rows = rows_to_dicts(
        connection.execute("SELECT * FROM release_logs ORDER BY datetime(released_at) DESC").fetchall()
    )
    return filter_rows(rows, q)


@app.post("/api/release-logs")
def create_release_log(payload: ReleaseLogCreate, connection=Depends(get_db)) -> dict[str, Any]:
    released_at = to_local_iso(payload.released_at)
    now = utc_now()
    upsert_terminal(
        connection,
        serial_number=payload.serial_number,
        terminal_model=payload.terminal_model,
        brand=payload.brand,
        status="Released",
        current_station="Release",
        last_handled_by=payload.released_by,
    )
    cursor = connection.execute(
        """
        INSERT INTO release_logs (
            serial_number, terminal_model, brand, released_by, destination,
            status, released_at, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """,
        (
            payload.serial_number,
            payload.terminal_model,
            payload.brand,
            payload.released_by,
            payload.destination,
            payload.status,
            released_at,
            now,
        ),
    )
    add_event(
        connection,
        event_type="release",
        message=f"Terminal {payload.brand} {payload.terminal_model} ({payload.serial_number}) released by {payload.released_by}",
        actor=payload.released_by,
        serial_number=payload.serial_number,
        station_id="Release",
    )
    connection.commit()
    return {"id": cursor.lastrowid, "status": "created"}


@app.get("/api/assignments")
def list_assignments(q: str | None = None, connection=Depends(get_db)) -> list[dict[str, Any]]:
    rows = rows_to_dicts(
        connection.execute("SELECT * FROM assignments ORDER BY datetime(created_at) DESC").fetchall()
    )
    return filter_rows(rows, q)


@app.post("/api/assignments")
def create_assignment(payload: AssignmentCreate, connection=Depends(get_db)) -> dict[str, Any]:
    now = utc_now()
    upsert_terminal(
        connection,
        serial_number=payload.serial_number,
        terminal_model=payload.terminal_model,
        brand=payload.brand,
        status=payload.status,
        current_station="Inspection",
        last_handled_by=payload.assigned_staff,
    )
    started_at = to_local_iso(payload.started_at)
    ended_at = to_local_iso(payload.ended_at)
    cursor = connection.execute(
        """
        INSERT INTO assignments (
            serial_number, terminal_model, brand, assigned_staff, task_name,
            started_at, ended_at, flagged_conditions, status, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """,
        (
            payload.serial_number,
            payload.terminal_model,
            payload.brand,
            payload.assigned_staff,
            payload.task_name,
            started_at,
            ended_at,
            payload.flagged_conditions,
            payload.status,
            now,
        ),
    )
    add_event(
        connection,
        event_type="assignment",
        message=f"{payload.assigned_staff} assigned to {payload.task_name} for {payload.serial_number}",
        actor=payload.assigned_staff,
        serial_number=payload.serial_number,
        station_id="Inspection",
    )
    connection.commit()
    return {"id": cursor.lastrowid, "status": "created"}


@app.get("/api/inspection-records")
def list_inspection_records(q: str | None = None, connection=Depends(get_db)) -> list[dict[str, Any]]:
    rows = rows_to_dicts(
        connection.execute(
            "SELECT * FROM inspection_records ORDER BY datetime(inspected_at) DESC"
        ).fetchall()
    )
    return filter_rows(rows, q)


@app.post("/api/inspection-records")
def create_inspection_record(payload: InspectionCreate, connection=Depends(get_db)) -> dict[str, Any]:
    inspected_at = to_local_iso(payload.inspected_at)
    now = utc_now()
    upsert_terminal(
        connection,
        serial_number=payload.serial_number,
        terminal_model=payload.terminal_model,
        brand=payload.brand,
        status=payload.remarks,
        current_station="Inspection",
        last_handled_by=payload.inspected_by,
    )
    cursor = connection.execute(
        """
        INSERT INTO inspection_records (
            serial_number, terminal_model, brand, inspection_task, inspected_by,
            remarks, inspected_at, evidence_path, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """,
        (
            payload.serial_number,
            payload.terminal_model,
            payload.brand,
            payload.inspection_task,
            payload.inspected_by,
            payload.remarks,
            inspected_at,
            payload.evidence_path,
            now,
        ),
    )
    add_event(
        connection,
        event_type="inspection",
        message=f"{payload.inspected_by} logged {payload.remarks} for {payload.serial_number}",
        actor=payload.inspected_by,
        serial_number=payload.serial_number,
        station_id="Inspection",
    )
    connection.commit()
    return {"id": cursor.lastrowid, "status": "created"}


@app.get("/api/history")
def history(kind: str = "all", q: str | None = None, connection=Depends(get_db)) -> dict[str, Any]:
    received = []
    released = []
    if kind in ("all", "received"):
        received = rows_to_dicts(
            connection.execute(
                "SELECT * FROM receive_logs ORDER BY datetime(received_at) DESC"
            ).fetchall()
        )
    if kind in ("all", "released"):
        released = rows_to_dicts(
            connection.execute(
                "SELECT * FROM release_logs ORDER BY datetime(released_at) DESC"
            ).fetchall()
        )
    return {
        "received": filter_rows(received, q),
        "released": filter_rows(released, q),
    }


@app.get("/api/events")
def events(q: str | None = None, connection=Depends(get_db)) -> list[dict[str, Any]]:
    rows = rows_to_dicts(
        connection.execute("SELECT * FROM events ORDER BY datetime(created_at) DESC, id DESC").fetchall()
    )
    return filter_rows(rows, q)


@app.get("/api/visual-analyses")
def visual_analyses(q: str | None = None, connection=Depends(get_db)) -> list[dict[str, Any]]:
    rows = rows_to_dicts(
        connection.execute(
            "SELECT * FROM visual_analyses ORDER BY datetime(created_at) DESC, id DESC"
        ).fetchall()
    )
    return filter_rows(rows, q)


@app.post("/api/visual/analyze", response_model=VisualAnalysisResponse)
async def analyze_uploaded_image(
    file: UploadFile = File(...),
    serial_number: str | None = Form(None),
    station_id: str | None = Form(None),
    prompt: str | None = Form(None),
    connection=Depends(get_db),
) -> VisualAnalysisResponse:
    image_bytes = await file.read()
    if not image_bytes:
        raise HTTPException(status_code=400, detail="Uploaded image is empty")

    mime_type = file.content_type or "image/jpeg"
    image_bytes = enhance_image(image_bytes)
    return await run_visual_analysis(
        image_bytes=image_bytes,
        mime_type=mime_type,
        source="upload",
        serial_number=serial_number,
        station_id=station_id,
        prompt=prompt,
        connection=connection,
    )


@app.post("/api/visual/analyze-from-camera", response_model=VisualAnalysisResponse)
async def analyze_from_camera(
    payload: AnalyzeFromCameraRequest,
    connection=Depends(get_db),
) -> VisualAnalysisResponse:
    capture_url = payload.capture_url or settings.esp32_capture_url

    image_bytes, mime_type = await fetch_camera_image(capture_url)
    image_bytes = enhance_image(image_bytes)
    return await run_visual_analysis(
        image_bytes=image_bytes,
        mime_type=mime_type,
        source="esp32-cam",
        serial_number=payload.serial_number,
        station_id=payload.station_id,
        prompt=payload.prompt,
        connection=connection,
    )


@app.post("/api/visual/detect-live")
async def detect_live(file: UploadFile = File(...)) -> dict[str, Any]:
    image_bytes = await file.read()
    if not image_bytes:
        raise HTTPException(status_code=400, detail="Uploaded image is empty")
    boxes = detect_objects_yolo_boxes(image_bytes)
    return {"boxes": boxes}


@app.post("/api/visual/scan-serial")
async def scan_serial(file: UploadFile = File(...)) -> dict[str, Any]:
    image_bytes = await file.read()
    if not image_bytes:
        raise HTTPException(status_code=400, detail="Uploaded image is empty")
    
    prompt = (
        "Analyze the image and extract: \n"
        "1. The terminal's serial number or barcode text.\n"
        "2. The terminal's brand, model, or manufacturer (e.g., Castles, PAX, Verifone, Ingenico) if visible or known.\n\n"
        "Return strict JSON with this structure:\n"
        '{"serial_number": "extracted_serial_or_barcode", "brand": "detected_brand_or_unknown", "reason": "short explanation"}\n\n'
        "For example, if the barcode text is 'VEGA 3000' and it is a Castles terminal, return "
        '{"serial_number": "VEGA 3000", "brand": "Castles"}.'
    )
    
    try:
        result_text, parsed_result = await analyze_image(
            image_bytes,
            mime_type="image/jpeg",
            prompt=prompt
        )
        serial_number = None
        brand = None
        if parsed_result:
            serial_number = parsed_result.get("serial_number")
            brand = parsed_result.get("brand")
        return {"serial_number": serial_number, "brand": brand}
    except Exception as e:
        print(f"[Gemini] Error scanning serial: {e}")
        return {"serial_number": None, "brand": None, "error": str(e)}


@app.post("/api/visual/monitor/start")
async def start_visual_monitor(payload: VisualMonitorStartRequest) -> dict[str, Any]:
    if visual_monitor_state["running"]:
        raise HTTPException(status_code=409, detail="Visual monitor is already running")

    visual_monitor_state.update(
        {
            "running": True,
            "started_at": utc_now(),
            "stopped_at": None,
            "last_run_at": None,
            "last_success_at": None,
            "last_error": None,
            "cycles_completed": 0,
            "last_analysis_id": None,
            "config": payload.model_dump(),
        }
    )
    visual_monitor_state["task"] = asyncio.create_task(visual_monitor_loop(payload))
    return visual_monitor_status()


@app.post("/api/visual/monitor/stop")
async def stop_visual_monitor() -> dict[str, Any]:
    task = visual_monitor_state.get("task")
    visual_monitor_state["running"] = False

    if task:
        task.cancel()
        try:
            await task
        except asyncio.CancelledError:
            pass

    visual_monitor_state["task"] = None
    visual_monitor_state["stopped_at"] = visual_monitor_state["stopped_at"] or utc_now()
    return visual_monitor_status()


@app.get("/api/visual/monitor/status")
def get_visual_monitor_status() -> dict[str, Any]:
    return visual_monitor_status()


@app.post("/api/transaction")
def create_transaction(payload: TransactionCreate, connection=Depends(get_db)) -> dict[str, Any]:
    task = payload.task.lower()
    
    if task == "receive":
        status = "In Queue"
        current_station = payload.station or "Receiving"
    elif task == "release":
        status = "Released"
        current_station = "Release"
    elif task == "process":
        notes = (payload.remarks or "").lower()
        if "defect" in notes:
            status = "Defect"
        elif "missing" in notes:
            status = "Missing"
        else:
            status = "Inspection"
        current_station = "Inspection"
    else:
        status = "Processing"
        current_station = payload.station or "Processing"

    upsert_terminal(
        connection,
        serial_number=payload.terminal_id,
        terminal_model=payload.terminal_model,
        brand=payload.terminal_brand,
        status=status,
        current_station=current_station,
        last_handled_by=payload.staff_name,
    )

    if task == "receive":
        connection.execute(
            """
            INSERT INTO receive_logs (
                serial_number, terminal_model, brand, received_by, source, station_id,
                status, received_at, created_at
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                payload.terminal_id,
                payload.terminal_model,
                payload.terminal_brand,
                payload.staff_name,
                "ESP32 Workstation",
                payload.station,
                status,
                to_local_iso(payload.ended_at),
                utc_now(),
            ),
        )
    elif task == "release":
        connection.execute(
            """
            INSERT INTO release_logs (
                serial_number, terminal_model, brand, released_by, destination,
                status, released_at, created_at
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                payload.terminal_id,
                payload.terminal_model,
                payload.terminal_brand,
                payload.staff_name,
                "Outbound",
                status,
                to_local_iso(payload.ended_at),
                utc_now(),
            ),
        )
    elif task == "process":
        remarks_outcome = "Completed"
        if status == "Defect":
            remarks_outcome = "Defect"
        elif status == "Missing":
            remarks_outcome = "Missing"
            
        connection.execute(
            """
            INSERT INTO inspection_records (
                serial_number, terminal_model, brand, inspection_task, inspected_by,
                remarks, inspected_at, evidence_path, created_at
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                payload.terminal_id,
                payload.terminal_model,
                payload.terminal_brand,
                "Diagnostic Servicing Check",
                payload.staff_name,
                remarks_outcome,
                to_local_iso(payload.ended_at),
                None,
                utc_now(),
            ),
        )

    action_verb = "received" if task == "receive" else ("released" if task == "release" else "inspected")
    message = f"Terminal {payload.terminal_brand} {payload.terminal_model} ({payload.terminal_id}) {action_verb} by {payload.staff_name}"
    
    add_event(
        connection,
        event_type=task,
        message=message,
        actor=payload.staff_name,
        serial_number=payload.terminal_id,
        station_id=payload.station,
    )
    
    connection.commit()
    return {"status": "success", "synced": True}

