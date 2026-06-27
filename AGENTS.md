# MOR-TECHNO-PROJECT

IoT-Based Internal Workflow Traceability System with AI-Powered Visual Verification (TMS Service Center).

## Cursor Cloud specific instructions

### Repository layout (important)

The `main` branch currently contains only `README.md` — the actual application code lives on feature branches / open PRs:

- `cursor/esp32-cam-setup-*` — Python/FastAPI backend in `backend/` plus ESP32 Arduino firmware (`esp32_cam_setup/`, `esp32_main_controller/`).
- `cursor/fix-login-screen-*` — self-contained static frontend dashboard (`index.html`).

Because of this, the update script guards its dependency install on the presence of `backend/requirements.txt`; on `main` it just creates the venv and skips installing. When you check out a branch that has `backend/requirements.txt`, the update script installs the backend deps into `.venv` at the repo root.

### Services

| Service | Location | Run command (dev) | Notes |
| --- | --- | --- | --- |
| Backend API | `backend/` | `cd backend && cp -n .env.example .env && /workspace/.venv/bin/python -m uvicorn app.main:app --reload --host 0.0.0.0 --port 8000` | FastAPI. Docs at `/docs`, health at `/api/health`. Defaults to SQLite mode. |
| Frontend dashboard | `index.html` (repo root on the frontend branch) | `python -m http.server 8080` then open `/index.html` | Single static file; no build step. Client-side login only — not yet wired to the backend. |
| ESP32 firmware | `esp32_*/` | n/a | Arduino sketches; require physical ESP32 hardware + Arduino IDE. Cannot run in the cloud VM. |

Standard setup/run details for the backend are in `backend/README.md`; do not duplicate them.

### Non-obvious gotchas

- **System dependency**: the venv needs the `python3.12-venv` apt package. It is already installed in the VM snapshot, so the update script only does pip installs — do not add apt installs to the update script.
- **Backend runs fully offline in SQLite mode.** No Supabase/Postgres or Gemini key is required to develop the core APIs. SQLite is the default whenever `SUPABASE_DB_URL` is empty. Gemini visual-analysis endpoints (`/api/visual/*`) return 503 without `GEMINI_API_KEY`; everything else works.
- **Admin login password trap**: the seeded admin password comes from `TMS_ADMIN_PASSWORD`. If you `cp .env.example .env`, that value is `change-this-password`, **not** the `admin123` shown in the README/frontend. `admin123` is only the fallback when no `.env`/env var is set. Seeded staff accounts use password `password` (e.g. `ana` / `password`).
- **DB seeding runs once**: the SQLite DB is created at `backend/data/tms.db` and seed data is only inserted when the `terminals` table is empty. To re-seed, delete `backend/data/tms.db` and restart.
- **Frontend auth is independent**: `index.html` checks `admin` / `admin123` purely in client-side JavaScript (`localStorage`) and does not call the backend `/api/auth/login`.
