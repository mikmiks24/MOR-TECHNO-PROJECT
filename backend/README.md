# TMS Traceability Backend

Python/FastAPI backend for the IoT-Based Internal Workflow Traceability System with AI-Powered Visual Verification.

The backend is the central server for:

- admin dashboard data
- RFID staff verification
- terminal receive and release logs
- staff task assignments
- inspection history
- ESP32-CAM image capture analysis through Gemini

## Architecture

```text
ESP32-CAM / RFID workstation
  -> sends terminal, staff, and image data

FastAPI backend
  -> stores workflow records in Supabase/Postgres or local SQLite
  -> calls Gemini with the backend API key
  -> returns dashboard and analysis results

Frontend dashboard
  -> reads records, metrics, history, and AI results from /api/*
```

Keep the Gemini API key on the backend. Do not store a real API key inside Arduino code or frontend JavaScript.

## Setup

### Recommended: Supabase database

```bash
cd backend
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
cp .env.example .env
```

Create a Supabase project, then open **SQL Editor** and run:

```text
backend/supabase/schema.sql
```

In Supabase, copy your project database connection string:

1. Open **Project Settings**.
2. Go to **Database**.
3. Copy a Postgres connection string or pooler connection string.
4. Use the password you set for the Supabase database.

Edit `.env`:

```env
DATABASE_BACKEND=supabase
SUPABASE_DB_URL=postgresql://postgres.PROJECT_REF:YOUR_DB_PASSWORD@aws-0-region.pooler.supabase.com:6543/postgres?sslmode=require
GEMINI_API_KEY=your-real-gemini-api-key
ESP32_CAPTURE_URL=http://YOUR_ESP32_IP/capture
```

Use the backend server as the only code that connects to Supabase. Do not put the Supabase database password, service role key, or Gemini API key in Arduino code or frontend JavaScript.

### Local-only fallback: SQLite

For quick local testing without Supabase, keep:

```env
DATABASE_BACKEND=sqlite
SUPABASE_DB_URL=
```

The SQLite database is created automatically at `backend/data/tms.db`.

Run the backend:

```bash
python3 -m uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

Open:

```text
http://localhost:8000/docs
```

Check the active database mode:

```text
http://localhost:8000/api/health
```

The response includes `database_backend` and `supabase_configured`.

## Main API routes

### System

| Method | Route | Purpose |
| --- | --- | --- |
| GET | `/api/health` | Check backend, database, and Gemini configuration |
| GET | `/api/dashboard` | Dashboard metrics, charts, and recent activity |

### Authentication and RFID

| Method | Route | Purpose |
| --- | --- | --- |
| POST | `/api/auth/login` | Basic dashboard login for prototype use |
| POST | `/api/rfid/verify` | Verify staff RFID UID from the workstation |

### Workflow records

| Method | Route | Purpose |
| --- | --- | --- |
| GET/POST | `/api/receive-logs` | Terminal receiving records |
| GET/POST | `/api/release-logs` | Terminal release records |
| GET/POST | `/api/assignments` | Staff task assignments |
| GET/POST | `/api/inspection-records` | Inspection/checklist records |
| GET | `/api/history` | Combined receive/release audit trail |
| GET | `/api/events` | Real-time activity log |

Every list route supports `?q=searchText`.

### Gemini visual verification

Upload an image from a browser, phone, or ESP32 client:

```bash
curl -X POST http://localhost:8000/api/visual/analyze \
  -F "file=@sample-terminal.jpg" \
  -F "serial_number=PAX-007-2026" \
  -F "station_id=Station 1"
```

Ask the backend to fetch the latest still image from the ESP32-CAM `/capture` endpoint:

```bash
curl -X POST http://localhost:8000/api/visual/analyze-from-camera \
  -H "Content-Type: application/json" \
  -d '{
    "capture_url": "http://YOUR_ESP32_IP/capture",
    "serial_number": "PAX-007-2026",
    "station_id": "Station 1"
  }'
```

The response includes:

- Gemini result text
- parsed JSON result when Gemini returns valid JSON
- recommended status such as `completed`, `defect`, `missing`, or `review`
- saved evidence image path

## Frontend integration

Your current frontend has hard-coded table rows and metric values. Replace those static values with calls to:

- `/api/dashboard` for metric cards, charts, and recent activity
- `/api/receive-logs` for the receive table
- `/api/release-logs` for the release table
- `/api/assignments` for the task table
- `/api/history` for the terminal history view
- `/api/inspection-records` for inspection history
- `/api/visual/analyze-from-camera` for the Gemini analyzer button

Example:

```js
const API_BASE = "http://localhost:8000";

async function loadDashboard() {
  const response = await fetch(`${API_BASE}/api/dashboard`);
  const dashboard = await response.json();
  console.log(dashboard.metrics);
}
```

## Prototype login

Default local login:

```text
username: admin
password: admin123
```

Change these in `.env` for demos:

```env
TMS_ADMIN_USERNAME=admin
TMS_ADMIN_PASSWORD=change-this-password
```
