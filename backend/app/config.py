from __future__ import annotations

import os
from pathlib import Path

from dotenv import load_dotenv


BASE_DIR = Path(__file__).resolve().parents[1]
load_dotenv(BASE_DIR / ".env")


def backend_path(env_name: str, default: Path) -> Path:
    value = os.getenv(env_name)
    if not value:
        return default

    path = Path(value)
    if path.is_absolute():
        return path
    return BASE_DIR / path


class Settings:
    app_name: str = "TMS Traceability Backend"
    database_path: Path = backend_path("TMS_DATABASE_PATH", BASE_DIR / "data" / "tms.db")
    evidence_dir: Path = backend_path("TMS_EVIDENCE_DIR", BASE_DIR / "data" / "evidence")

    admin_username: str = os.getenv("TMS_ADMIN_USERNAME", "admin")
    admin_password: str = os.getenv("TMS_ADMIN_PASSWORD", "admin123")

    gemini_api_key: str = os.getenv("GEMINI_API_KEY", "")
    gemini_model: str = os.getenv("GEMINI_MODEL", "gemini-2.0-flash")
    gemini_timeout_seconds: float = float(os.getenv("GEMINI_TIMEOUT_SECONDS", "45"))

    esp32_capture_url: str = os.getenv("ESP32_CAPTURE_URL", "http://192.168.1.50/capture")

    cors_origins: list[str] = [
        origin.strip()
        for origin in os.getenv("CORS_ORIGINS", "*").split(",")
        if origin.strip()
    ]


settings = Settings()
