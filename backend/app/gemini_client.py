from __future__ import annotations

import base64
import json
import re
from typing import Any

import httpx

from .config import settings


DEFAULT_VISUAL_PROMPT = """
You are an AI visual verification assistant for an internal POS terminal workflow system.
Inspect the image and identify visible POS terminal devices, accessories, missing items,
and physical issues.

Return strict JSON only with this structure:
{
  "summary": "one short sentence",
  "objects": ["detected object names"],
  "missing_items": ["missing or uncertain accessories"],
  "defects": ["visible damage or condition issues"],
  "recommended_status": "completed | defect | missing | review",
  "notes": "short operational note for the supervisor"
}
""".strip()


class GeminiNotConfiguredError(RuntimeError):
    pass


def _extract_text(response_json: dict[str, Any]) -> str:
    candidates = response_json.get("candidates", [])
    if not candidates:
        return json.dumps(response_json, indent=2)

    parts = candidates[0].get("content", {}).get("parts", [])
    text_parts = [part.get("text", "") for part in parts if part.get("text")]
    return "\n".join(text_parts).strip() or json.dumps(response_json, indent=2)


def parse_json_from_text(text: str) -> dict[str, Any] | None:
    cleaned = text.strip()
    fenced = re.search(r"```(?:json)?\s*(\{.*?\})\s*```", cleaned, re.DOTALL)
    if fenced:
        cleaned = fenced.group(1)

    if not cleaned.startswith("{"):
        first = cleaned.find("{")
        last = cleaned.rfind("}")
        if first >= 0 and last > first:
            cleaned = cleaned[first : last + 1]

    try:
        return json.loads(cleaned)
    except json.JSONDecodeError:
        return None


async def analyze_image(
    image_bytes: bytes,
    *,
    mime_type: str = "image/jpeg",
    prompt: str | None = None,
) -> tuple[str, dict[str, Any] | None]:
    if not settings.gemini_api_key:
        raise GeminiNotConfiguredError("GEMINI_API_KEY is not configured on the backend.")

    encoded_image = base64.b64encode(image_bytes).decode("ascii")
    request_body = {
        "contents": [
            {
                "parts": [
                    {"text": prompt or DEFAULT_VISUAL_PROMPT},
                    {
                        "inline_data": {
                            "mime_type": mime_type,
                            "data": encoded_image,
                        }
                    },
                ]
            }
        ],
        "generationConfig": {
            "temperature": 0.2,
            "maxOutputTokens": 512,
            "responseMimeType": "application/json",
        },
    }

    url = (
        "https://generativelanguage.googleapis.com/v1beta/models/"
        f"{settings.gemini_model}:generateContent"
    )

    async with httpx.AsyncClient(timeout=settings.gemini_timeout_seconds) as client:
        response = await client.post(
            url,
            params={"key": settings.gemini_api_key},
            json=request_body,
        )
        response.raise_for_status()

    response_json = response.json()
    result_text = _extract_text(response_json)
    return result_text, parse_json_from_text(result_text)
