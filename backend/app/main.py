from __future__ import annotations

import json
import uuid
from pathlib import Path
from typing import Any

import httpx
from fastapi import Depends, FastAPI, File, Form, HTTPException, UploadFile
from fastapi.middleware.cors import CORSMiddleware

from .config import settings
from .database import (
    get_connection,
    hash_password,
    initialize_database,
    row_to_dict,
    rows_to_dicts,
    utc_now,
)
from .gemini_client import GeminiNotConfiguredError, analyze_image
from .schemas import (
    AnalyzeFromCameraRequest,
    AssignmentCreate,
    InspectionCreate,
    LoginRequest,
    ReceiveLogCreate,
    ReleaseLogCreate,
    RfidVerifyRequest,
    VisualAnalysisResponse,
)


app = FastAPI(title=settings.app_name)

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.on_event("startup")
def startup() -> None:
    initialize_database()
    settings.evidence_dir.mkdir(parents=True, exist_ok=True)


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


def save_evidence(image_bytes: bytes, extension: str = ".jpg") -> str:
    settings.evidence_dir.mkdir(parents=True, exist_ok=True)
    safe_extension = extension if extension.startswith(".") else f".{extension}"
    filename = f"{utc_now().replace(':', '').replace('+', 'Z')}_{uuid.uuid4().hex}{safe_extension}"
    path = settings.evidence_dir / filename
    path.write_bytes(image_bytes)
    return str(path)


def recommended_status(parsed_result: dict[str, Any] | None) -> str | None:
    if not parsed_result:
        return None
    status = parsed_result.get("recommended_status")
    if isinstance(status, str):
        return status.lower()
    return None


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
    image_path = save_evidence(image_bytes, extension)

    try:
        result_text, parsed_result = await analyze_image(
            image_bytes,
            mime_type=mime_type,
            prompt=prompt,
        )
    except GeminiNotConfiguredError as error:
        raise HTTPException(status_code=503, detail=str(error)) from error
    except httpx.HTTPStatusError as error:
        raise HTTPException(
            status_code=502,
            detail=f"Gemini API returned HTTP {error.response.status_code}: {error.response.text}",
        ) from error
    except httpx.HTTPError as error:
        raise HTTPException(status_code=502, detail=f"Gemini request failed: {error}") from error

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
    )


@app.get("/")
def root() -> dict[str, str]:
    return {
        "name": settings.app_name,
        "docs": "/docs",
        "health": "/api/health",
    }


@app.get("/api/health")
def health() -> dict[str, Any]:
    return {
        "status": "ok",
        "gemini_configured": bool(settings.gemini_api_key),
        "database_backend": settings.database_backend,
        "database": str(settings.database_path),
        "supabase_configured": bool(settings.supabase_db_url),
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
    received_at = payload.received_at or utc_now()
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
    released_at = payload.released_at or utc_now()
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
            payload.started_at,
            payload.ended_at,
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
    inspected_at = payload.inspected_at or utc_now()
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
    return await run_visual_analysis(
        image_bytes=response.content,
        mime_type=mime_type,
        source="esp32-cam",
        serial_number=payload.serial_number,
        station_id=payload.station_id,
        prompt=payload.prompt,
        connection=connection,
    )
