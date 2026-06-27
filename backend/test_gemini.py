import asyncio
import os
import sys
import base64
from pathlib import Path

# Add backend directory to system path to import app package
backend_dir = Path(__file__).resolve().parent
sys.path.insert(0, str(backend_dir))

from app.gemini_client import analyze_image
from app.config import settings

# A tiny 1x1 red PNG image base64
TINY_PNG_B64 = (
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg=="
)

async def test_gemini():
    print("=" * 60)
    print("Gemini API Diagnostic Tool for Image Analysis")
    print("=" * 60)
    
    # 1. Check Configuration
    print(f"[-] Checking configuration...")
    if not settings.gemini_api_key:
        print("\n[!] Error: GEMINI_API_KEY is not configured!")
        print("    Please open your 'backend/.env' file and configure the API key:")
        print("    GEMINI_API_KEY=your_actual_api_key_here\n")
        return
    else:
        # Mask the API key for display
        key_masked = settings.gemini_api_key[:4] + "..." + settings.gemini_api_key[-4:] if len(settings.gemini_api_key) > 8 else "configured"
        print(f"    - GEMINI_API_KEY: {key_masked}")
        print(f"    - GEMINI_MODEL: {settings.gemini_model}")
        print(f"    - Timeout: {settings.gemini_timeout_seconds}s")
    
    # 2. Select Image Source
    image_bytes = None
    mime_type = "image/png"
    
    if len(sys.argv) > 1:
        arg = sys.argv[1]
        if arg.startswith("http://") or arg.startswith("https://"):
            print(f"[-] Fetching camera image from: {arg}")
            try:
                import httpx
                async with httpx.AsyncClient(timeout=20) as client:
                    resp = await client.get(arg)
                    resp.raise_for_status()
                    image_bytes = resp.content
                    mime_type = resp.headers.get("content-type", "image/jpeg").split(";")[0]
                print(f"    - Downloaded {len(image_bytes)} bytes successfully (Mime-type: {mime_type})")
                
                # Save the image locally for the user
                save_path = Path("latest_capture.jpg")
                save_path.write_bytes(image_bytes)
                print(f"    - Saved latest camera frame to: {save_path.resolve()}")
            except Exception as e:
                print(f"\n[!] Error fetching from camera URL: {e}")
                return
        else:
            img_path = Path(arg)
            if img_path.exists():
                print(f"[-] Loading image from: {img_path}")
                image_bytes = img_path.read_bytes()
                # Detect mime type based on extension
                ext = img_path.suffix.lower()
                if ext in [".jpg", ".jpeg"]:
                    mime_type = "image/jpeg"
                elif ext == ".png":
                    mime_type = "image/png"
                elif ext == ".webp":
                    mime_type = "image/webp"
                else:
                    mime_type = "image/jpeg"  # Fallback
                print(f"    - Detected Mime-Type: {mime_type}")
                print(f"    - Image Size: {len(image_bytes)} bytes")
            else:
                print(f"\n[!] Error: Provided image path '{img_path}' does not exist.")
                return
    else:
        print("[-] No image file or URL specified in arguments.")
        print("    Using a built-in 1x1 pixel red image for testing API connectivity.")
        image_bytes = base64.b64decode(TINY_PNG_B64)
        mime_type = "image/png"
    
    # 3. Call Gemini API
    print("\n[-] Sending request to Gemini API...")
    try:
        raw_text, parsed_json = await analyze_image(
            image_bytes=image_bytes,
            mime_type=mime_type,
            prompt="Analyze this image. Describe what you see. If it is a solid red color, state that you see a solid red square."
        )
        print("\n[+] SUCCESS! Response received from Gemini API.")
        print("-" * 60)
        print("Raw text response:")
        print("-" * 60)
        print(raw_text)
        print("-" * 60)
        print("Parsed JSON representation:")
        print("-" * 60)
        if parsed_json:
            import json
            print(json.dumps(parsed_json, indent=2))
        else:
            print("Failed to parse response as JSON. This is expected if the prompt was not requesting JSON or returned plain text.")
        print("-" * 60)
        
    except Exception as e:
        print(f"\n[!] Error calling Gemini API: {e}")
        print("    Please verify:")
        print("    1. Your API key in 'backend/.env' is valid.")
        print("    2. Your internet connection is active.")
        print("    3. You have not hit rate limits on your Gemini API account.")
    print("=" * 60)

if __name__ == "__main__":
    # If on Windows, set selector event loop policy if needed
    if sys.platform == 'win32':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    asyncio.run(test_gemini())
