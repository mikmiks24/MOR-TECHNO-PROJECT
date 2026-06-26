from __future__ import annotations

from typing import Any, Literal

from pydantic import BaseModel, Field


class LoginRequest(BaseModel):
    username: str = Field(min_length=1)
    password: str = Field(min_length=1)


class RfidVerifyRequest(BaseModel):
    rfid_uid: str = Field(min_length=1)
    station_id: str | None = None


class EmployeeCreate(BaseModel):
    name: str = Field(min_length=1)
    rfid_uid: str = Field(min_length=1)
    role: str = "staff"
    username: str | None = None
    password: str | None = None
    active: bool = True


class ReceiveLogCreate(BaseModel):
    serial_number: str = Field(min_length=1)
    terminal_model: str = Field(min_length=1)
    brand: str = Field(min_length=1)
    received_by: str = Field(min_length=1)
    source: str | None = None
    station_id: str | None = None
    status: str = "In Queue"
    received_at: str | None = None


class ReleaseLogCreate(BaseModel):
    serial_number: str = Field(min_length=1)
    terminal_model: str = Field(min_length=1)
    brand: str = Field(min_length=1)
    released_by: str = Field(min_length=1)
    destination: str | None = None
    status: str = "Cleared"
    released_at: str | None = None


class AssignmentCreate(BaseModel):
    serial_number: str = Field(min_length=1)
    terminal_model: str = Field(min_length=1)
    brand: str = Field(min_length=1)
    assigned_staff: str = Field(min_length=1)
    task_name: str = Field(min_length=1)
    started_at: str | None = None
    ended_at: str | None = None
    flagged_conditions: str | None = None
    status: str = "Ongoing"


class InspectionCreate(BaseModel):
    serial_number: str = Field(min_length=1)
    terminal_model: str = Field(min_length=1)
    brand: str = Field(min_length=1)
    inspection_task: str = Field(min_length=1)
    inspected_by: str = Field(min_length=1)
    remarks: Literal["Completed", "Defect", "Missing", "Review"] = "Review"
    inspected_at: str | None = None
    evidence_path: str | None = None


class AnalyzeFromCameraRequest(BaseModel):
    capture_url: str | None = None
    serial_number: str | None = None
    station_id: str | None = None
    prompt: str | None = None


class VisualMonitorStartRequest(BaseModel):
    capture_url: str | None = None
    serial_number: str | None = None
    station_id: str | None = None
    prompt: str | None = None
    interval_seconds: int = Field(default=15, ge=5, le=3600)
    max_cycles: int | None = Field(default=None, ge=1)


class VisualAnalysisResponse(BaseModel):
    id: int | None = None
    status: str
    source: str
    model: str
    result_text: str
    parsed_result: dict[str, Any] | None = None
    image_path: str | None = None
    serial_number: str | None = None
    station_id: str | None = None
