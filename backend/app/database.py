from __future__ import annotations

import hashlib
import re
import sqlite3
from datetime import datetime, timezone
from typing import Any

from .config import settings


def utc_now() -> str:
    return datetime.now().astimezone().isoformat(timespec="seconds")


def to_local_iso(iso_str: str | None) -> str:
    if not iso_str:
        return utc_now()
    try:
        clean_str = iso_str.replace("Z", "+00:00")
        dt = datetime.fromisoformat(clean_str)
        return dt.astimezone().isoformat(timespec="seconds")
    except Exception:
        return iso_str


def hash_password(password: str) -> str:
    return hashlib.sha256(password.encode("utf-8")).hexdigest()


class DictRow:
    def __init__(self, data: dict[str, Any]):
        self.data = data
        self.values = list(data.values())

    def __getitem__(self, key: str | int) -> Any:
        if isinstance(key, int):
            return self.values[key]
        return self.data[key]

    def keys(self):
        return self.data.keys()


class CursorAdapter:
    def __init__(self, cursor, lastrowid: int | None = None):
        self.cursor = cursor
        self.lastrowid = lastrowid

    def fetchone(self):
        row = self.cursor.fetchone()
        if row is None:
            return None
        if isinstance(row, dict):
            return DictRow(row)
        return row

    def fetchall(self):
        rows = self.cursor.fetchall()
        return [DictRow(row) if isinstance(row, dict) else row for row in rows]


class PostgresConnectionAdapter:
    def __init__(self):
        try:
            import psycopg
            from psycopg.rows import dict_row
        except ImportError as error:
            raise RuntimeError(
                "psycopg is required for Supabase/Postgres. Run: pip install -r backend/requirements.txt"
            ) from error

        self.connection = psycopg.connect(
            settings.supabase_db_url,
            row_factory=dict_row,
            connect_timeout=10,
        )

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if exc_type:
            self.connection.rollback()
        else:
            self.connection.commit()
        self.close()

    def execute(self, query: str, params: tuple[Any, ...] = ()):
        translated_query = self._translate_query(query)
        cursor = self.connection.cursor()
        cursor.execute(translated_query, params)

        lastrowid = None
        if translated_query.rstrip().upper().endswith("RETURNING ID"):
            returned = cursor.fetchone()
            if returned:
                lastrowid = returned["id"]

        return CursorAdapter(cursor, lastrowid=lastrowid)

    def executemany(self, query: str, params: list[tuple[Any, ...]]):
        translated_query = self._translate_query(query, add_returning=False)
        cursor = self.connection.cursor()
        cursor.executemany(translated_query, params)
        return CursorAdapter(cursor)

    def executescript(self, script: str) -> None:
        statements = [statement.strip() for statement in script.split(";") if statement.strip()]
        with self.connection.cursor() as cursor:
            for statement in statements:
                cursor.execute(statement)

    def commit(self) -> None:
        self.connection.commit()

    def rollback(self) -> None:
        self.connection.rollback()

    def close(self) -> None:
        self.connection.close()

    def _translate_query(self, query: str, *, add_returning: bool = True) -> str:
        translated = query.strip()
        translated = translated.replace("?", "%s")
        translated = translated.replace("active = 1", "active = true")
        translated = translated.replace("datetime(created_at)", "created_at")
        translated = translated.replace("datetime(updated_at)", "updated_at")
        translated = translated.replace("datetime(received_at)", "received_at")
        translated = translated.replace("datetime(released_at)", "released_at")
        translated = translated.replace("datetime(inspected_at)", "inspected_at")

        if add_returning and self._needs_returning_id(translated):
            translated = f"{translated} RETURNING id"

        return translated

    def _needs_returning_id(self, query: str) -> bool:
        if "RETURNING" in query.upper():
            return False

        match = re.match(r"INSERT\s+INTO\s+([a-z_]+)", query, re.IGNORECASE)
        if not match:
            return False

        return match.group(1).lower() in {
            "employees",
            "receive_logs",
            "release_logs",
            "assignments",
            "inspection_records",
            "visual_analyses",
            "events",
        }


def use_supabase() -> bool:
    return settings.database_backend.lower() == "supabase" and bool(settings.supabase_db_url)


def get_connection():
    if use_supabase():
        return PostgresConnectionAdapter()

    settings.database_path.parent.mkdir(parents=True, exist_ok=True)
    connection = sqlite3.connect(settings.database_path)
    connection.row_factory = sqlite3.Row
    connection.execute("PRAGMA foreign_keys = ON")
    return connection


def row_to_dict(row: Any | None) -> dict[str, Any] | None:
    if row is None:
        return None
    if isinstance(row, dict):
        return row
    if isinstance(row, DictRow):
        return dict(row.data)
    return {key: row[key] for key in row.keys()}


def rows_to_dicts(rows: list[Any]) -> list[dict[str, Any]]:
    return [row_to_dict(row) for row in rows if row is not None]


SCHEMA = """
CREATE TABLE IF NOT EXISTS employees (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    role TEXT NOT NULL DEFAULT 'staff',
    username TEXT UNIQUE,
    password_hash TEXT,
    rfid_uid TEXT UNIQUE,
    active INTEGER NOT NULL DEFAULT 1,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS terminals (
    serial_number TEXT PRIMARY KEY,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    status TEXT NOT NULL DEFAULT 'In Queue',
    current_station TEXT,
    last_handled_by TEXT,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS receive_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    serial_number TEXT NOT NULL,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    received_by TEXT NOT NULL,
    source TEXT,
    station_id TEXT,
    status TEXT NOT NULL,
    received_at TEXT NOT NULL,
    created_at TEXT NOT NULL,
    FOREIGN KEY (serial_number) REFERENCES terminals(serial_number)
        ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS release_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    serial_number TEXT NOT NULL,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    released_by TEXT NOT NULL,
    destination TEXT,
    status TEXT NOT NULL,
    released_at TEXT NOT NULL,
    created_at TEXT NOT NULL,
    FOREIGN KEY (serial_number) REFERENCES terminals(serial_number)
        ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS assignments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    serial_number TEXT NOT NULL,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    assigned_staff TEXT NOT NULL,
    task_name TEXT NOT NULL,
    started_at TEXT,
    ended_at TEXT,
    flagged_conditions TEXT,
    status TEXT NOT NULL,
    created_at TEXT NOT NULL,
    FOREIGN KEY (serial_number) REFERENCES terminals(serial_number)
        ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS inspection_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    serial_number TEXT NOT NULL,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    inspection_task TEXT NOT NULL,
    inspected_by TEXT NOT NULL,
    remarks TEXT NOT NULL,
    inspected_at TEXT NOT NULL,
    evidence_path TEXT,
    created_at TEXT NOT NULL,
    FOREIGN KEY (serial_number) REFERENCES terminals(serial_number)
        ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS visual_analyses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    serial_number TEXT,
    station_id TEXT,
    source TEXT NOT NULL,
    -- Local fallback path or Supabase Storage public URL/object path.
    image_path TEXT,
    model TEXT NOT NULL,
    prompt TEXT NOT NULL,
    result_text TEXT NOT NULL,
    result_json TEXT,
    recommended_status TEXT,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    event_type TEXT NOT NULL,
    message TEXT NOT NULL,
    actor TEXT,
    serial_number TEXT,
    station_id TEXT,
    created_at TEXT NOT NULL
);
"""


POSTGRES_SCHEMA = """
CREATE TABLE IF NOT EXISTS employees (
    id BIGINT GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
    name TEXT NOT NULL,
    role TEXT NOT NULL DEFAULT 'staff',
    username TEXT UNIQUE,
    password_hash TEXT,
    rfid_uid TEXT UNIQUE,
    active BOOLEAN NOT NULL DEFAULT true,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS terminals (
    serial_number TEXT PRIMARY KEY,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    status TEXT NOT NULL DEFAULT 'In Queue',
    current_station TEXT,
    last_handled_by TEXT,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS receive_logs (
    id BIGINT GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
    serial_number TEXT NOT NULL REFERENCES terminals(serial_number)
        ON UPDATE CASCADE ON DELETE CASCADE,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    received_by TEXT NOT NULL,
    source TEXT,
    station_id TEXT,
    status TEXT NOT NULL,
    received_at TEXT NOT NULL,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS release_logs (
    id BIGINT GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
    serial_number TEXT NOT NULL REFERENCES terminals(serial_number)
        ON UPDATE CASCADE ON DELETE CASCADE,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    released_by TEXT NOT NULL,
    destination TEXT,
    status TEXT NOT NULL,
    released_at TEXT NOT NULL,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS assignments (
    id BIGINT GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
    serial_number TEXT NOT NULL REFERENCES terminals(serial_number)
        ON UPDATE CASCADE ON DELETE CASCADE,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    assigned_staff TEXT NOT NULL,
    task_name TEXT NOT NULL,
    started_at TEXT,
    ended_at TEXT,
    flagged_conditions TEXT,
    status TEXT NOT NULL,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS inspection_records (
    id BIGINT GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
    serial_number TEXT NOT NULL REFERENCES terminals(serial_number)
        ON UPDATE CASCADE ON DELETE CASCADE,
    terminal_model TEXT NOT NULL,
    brand TEXT NOT NULL,
    inspection_task TEXT NOT NULL,
    inspected_by TEXT NOT NULL,
    remarks TEXT NOT NULL,
    inspected_at TEXT NOT NULL,
    evidence_path TEXT,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS visual_analyses (
    id BIGINT GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
    serial_number TEXT,
    station_id TEXT,
    source TEXT NOT NULL,
    image_path TEXT,
    model TEXT NOT NULL,
    prompt TEXT NOT NULL,
    result_text TEXT NOT NULL,
    result_json TEXT,
    recommended_status TEXT,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS events (
    id BIGINT GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
    event_type TEXT NOT NULL,
    message TEXT NOT NULL,
    actor TEXT,
    serial_number TEXT,
    station_id TEXT,
    created_at TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_receive_logs_received_at ON receive_logs(received_at DESC);
CREATE INDEX IF NOT EXISTS idx_release_logs_released_at ON release_logs(released_at DESC);
CREATE INDEX IF NOT EXISTS idx_events_created_at ON events(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_visual_analyses_created_at ON visual_analyses(created_at DESC);
"""


SEED_TERMINALS = [
    ("ING-002-2024", "iCT220", "Ingenico", "In Queue", "Receiving", "Juan dela Cruz"),
    ("VFN-003-2024", "VX680", "Verifone", "Inspecting", "Inspection", "Ana Reyes"),
    ("ING-005-2024", "iWL250", "Ingenico", "Received", "Receiving", "Jose Garcia"),
    ("PAX-007-2026", "A920", "PAX", "Released", "Release", "Gerson Barrientos"),
    ("VFN-006-2024", "VX820", "Verifone", "Released", "Release", "Joy Retuba"),
    ("VFN-001-2024", "VX520", "Verifone", "Released", "Release", "Gerson Barrientos"),
    ("CST-002-2026", "VEGA 3000", "Castles", "Inspection", "Inspection", "Joshua Zaide"),
    ("ING-003-2026", "iWL250", "Ingenico", "Inspection", "Inspection", "Joy Retuba"),
    ("CST-004-2026", "MP200", "Castles", "Inspection", "Inspection", "Ana Reyes"),
]


def initialize_database() -> None:
    with get_connection() as connection:
        connection.executescript(POSTGRES_SCHEMA if use_supabase() else SCHEMA)
        seed_database(connection)


def seed_database(connection: sqlite3.Connection) -> None:
    existing = connection.execute("SELECT COUNT(*) AS count FROM terminals").fetchone()["count"]
    if existing:
        return

    now = utc_now()
    employees = [
        ("Admin User", "superuser", settings.admin_username, hash_password(settings.admin_password), "ADMIN-RFID"),
        ("Ana Reyes", "staff", "ana", hash_password("password"), "RFID-ANA-001"),
        ("Juan dela Cruz", "staff", "juan", hash_password("password"), "RFID-JUAN-002"),
        ("Jose Garcia", "staff", "jose", hash_password("password"), "RFID-JOSE-003"),
        ("Joy Retuba", "staff", "joy", hash_password("password"), "RFID-JOY-004"),
        ("Joshua Zaide", "staff", "joshua", hash_password("password"), "RFID-JOSHUA-005"),
        ("Gerson Barrientos", "staff", "gerson", hash_password("password"), "RFID-GERSON-006"),
    ]
    connection.executemany(
        """
        INSERT INTO employees (name, role, username, password_hash, rfid_uid, created_at)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        [(*employee, now) for employee in employees],
    )

    connection.executemany(
        """
        INSERT INTO terminals (
            serial_number, terminal_model, brand, status, current_station,
            last_handled_by, created_at, updated_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """,
        [(*terminal, now, now) for terminal in SEED_TERMINALS],
    )

    receive_logs = [
        ("ING-002-2024", "iCT220", "Ingenico", "Juan dela Cruz", "Received via Courier", "Station 1", "In Queue", "2026-06-02T09:15:00+08:00"),
        ("VFN-003-2024", "VX680", "Verifone", "Ana Reyes", "Received via Field Eng", "Station 2", "Inspecting", "2026-06-03T10:00:00+08:00"),
        ("ING-005-2024", "iWL250", "Ingenico", "Jose Garcia", "Counter Drop-off", "Station 1", "Received", "2026-06-05T08:45:00+08:00"),
        ("PAX-007-2026", "A920", "PAX", "Joshua Zaide", "Counter Drop-off", "Station 1", "Received", "2026-03-03T08:14:00+08:00"),
        ("VFN-006-2024", "VX820", "Verifone", "Joy Retuba", "Warehouse Transfer", "Station 2", "Received", "2026-03-04T09:41:00+08:00"),
    ]
    connection.executemany(
        """
        INSERT INTO receive_logs (
            serial_number, terminal_model, brand, received_by, source, station_id,
            status, received_at, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """,
        [(*log, now) for log in receive_logs],
    )

    release_logs = [
        ("VFN-001-2024", "VX520", "Verifone", "Gerson Barrientos", "Warehouse", "Cleared", "2026-06-03T14:00:00+08:00"),
        ("VFN-006-2024", "VX820", "Verifone", "Joy Retuba", "Prep Team", "Cleared", "2026-06-07T09:00:00+08:00"),
        ("PAX-007-2026", "A920", "PAX", "Gerson Barrientos", "Warehouse", "Cleared", "2026-03-03T10:02:00+08:00"),
    ]
    connection.executemany(
        """
        INSERT INTO release_logs (
            serial_number, terminal_model, brand, released_by, destination,
            status, released_at, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """,
        [(*log, now) for log in release_logs],
    )

    assignments = [
        ("VFN-001-2024", "VX520", "Verifone", "Carlos Mendoza", "Full diagnostic inspection", "2026-06-03T09:00:00+08:00", "2026-06-03T11:30:00+08:00", "Screen Scratched; Keypad Stiff", "Done"),
        ("ING-002-2024", "iCT220", "Ingenico", "Liza Bautista", "SIM and accessory checklist", "2026-06-02T10:00:00+08:00", None, "Pending Review", "Ongoing"),
    ]
    connection.executemany(
        """
        INSERT INTO assignments (
            serial_number, terminal_model, brand, assigned_staff, task_name,
            started_at, ended_at, flagged_conditions, status, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """,
        [(*assignment, now) for assignment in assignments],
    )

    inspections = [
        ("CST-002-2026", "VEGA 3000", "Castles", "Verify SIM card is installed", "Joshua Zaide", "Defect", "2026-06-07T09:41:00+08:00", None),
        ("ING-003-2026", "iWL250", "Ingenico", "Check for scratches on screen", "Joy Retuba", "Completed", "2026-06-06T13:20:00+08:00", None),
        ("CST-004-2026", "MP200", "Castles", "Check power adapter", "Ana Reyes", "Missing", "2026-06-05T10:14:00+08:00", None),
    ]
    connection.executemany(
        """
        INSERT INTO inspection_records (
            serial_number, terminal_model, brand, inspection_task, inspected_by,
            remarks, inspected_at, evidence_path, created_at
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """,
        [(*inspection, now) for inspection in inspections],
    )

    events = [
        ("receive", "Terminal PAX A920 (PAX-007) received at Station 1 by Ana Reyes", "Ana Reyes", "PAX-007-2026", "Station 1"),
        ("receive", "Terminal Verifone VX820 (VFN-006) received at Station 2 by Juan dela Cruz", "Juan dela Cruz", "VFN-006-2024", "Station 2"),
        ("receive", "Terminal Ingenico iWL250 (ING-005) received at Station 1 by Jose Garcia", "Jose Garcia", "ING-005-2024", "Station 1"),
    ]
    connection.executemany(
        """
        INSERT INTO events (event_type, message, actor, serial_number, station_id, created_at)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        [(*event, now) for event in events],
    )
