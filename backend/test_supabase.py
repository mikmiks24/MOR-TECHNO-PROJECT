import sys
from pathlib import Path

# Add backend directory to system path
backend_dir = Path(__file__).resolve().parent
sys.path.insert(0, str(backend_dir))

from app.config import settings
import psycopg
import httpx

print("=" * 60)
print("Supabase Diagnostic Tool")
print("=" * 60)

# 1. Test database URL connection
print("[-] Testing Supabase DB URL connection...")
try:
    conn = psycopg.connect(settings.supabase_db_url)
    cursor = conn.cursor()
    cursor.execute("SELECT version();")
    ver = cursor.fetchone()
    print(f"    [+] DB connection SUCCESS! PostgreSQL Version: {ver[0]}")
    
    # Check tables
    cursor.execute("SELECT table_name FROM information_schema.tables WHERE table_schema='public';")
    tables = [r[0] for r in cursor.fetchall()]
    print(f"    [+] Existing public tables: {tables}")
    conn.close()
except Exception as e:
    print(f"    [!] DB connection FAILED: {e}")

# 2. Test Supabase API key and Storage Bucket
print("\n[-] Testing Supabase Storage API & Bucket...")
print(f"    - URL: {settings.supabase_url}")
print(f"    - Bucket: {settings.supabase_storage_bucket}")

headers = {
    "Authorization": f"Bearer {settings.supabase_service_role_key}",
    "apikey": settings.supabase_service_role_key,
}

test_file_url = f"{settings.supabase_url}/storage/v1/object/info/{settings.supabase_storage_bucket}"

try:
    with httpx.Client(timeout=10) as client:
        # Check if bucket exists
        resp = client.get(
            f"{settings.supabase_url}/storage/v1/bucket/{settings.supabase_storage_bucket}",
            headers=headers
        )
        if resp.status_code == 200:
            print(f"    [+] Storage bucket info: {resp.json()}")
        else:
            print(f"    [!] Storage bucket query returned status {resp.status_code}: {resp.text}")
            
        # Try uploading a dummy file
        print("[-] Attempting test file upload...")
        dummy_content = b"supabase_test_content"
        upload_resp = client.post(
            f"{settings.supabase_url}/storage/v1/object/{settings.supabase_storage_bucket}/test_diag.txt",
            headers={**headers, "Content-Type": "text/plain"},
            content=dummy_content
        )
        if upload_resp.status_code == 200:
            print("    [+] Upload test SUCCESS!")
            print(f"        Public URL would be: {settings.supabase_url}/storage/v1/object/public/{settings.supabase_storage_bucket}/test_diag.txt")
            
            # Clean up
            client.delete(
                f"{settings.supabase_url}/storage/v1/object/{settings.supabase_storage_bucket}/test_diag.txt",
                headers=headers
            )
        else:
            print(f"    [!] Upload test FAILED: status {upload_resp.status_code} - {upload_resp.text}")
except Exception as e:
    print(f"    [!] Storage request failed: {e}")

print("=" * 60)
