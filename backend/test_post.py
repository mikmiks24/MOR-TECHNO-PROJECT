import urllib.request
import json

url = "http://127.0.0.1:8000/api/transaction"
data = {
    "terminal_id": "TEST-SERIAL-999",
    "terminal_model": "PAX A920",
    "terminal_brand": "PAX",
    "staff_id": "STF-JZ",
    "staff_name": "Joshua Zaide",
    "staff_role": "Senior Technician",
    "station": "Bay 1",
    "task": "process",
    "remarks": "Checklist looks good, no defect.",
    "duration_sec": 45,
    "items_checked": 9,
    "items_total": 9,
    "camera_scan_count": 1,
    "checklist": [],
    "started_at": "2026-06-28 14:00",
    "ended_at": "2026-06-28 14:01"
}

req = urllib.request.Request(
    url, 
    data=json.dumps(data).encode('utf-8'), 
    headers={'Content-Type': 'application/json'}
)

try:
    with urllib.request.urlopen(req) as response:
        print(response.read().decode('utf-8'))
except Exception as e:
    print(f"Error: {e}")
