# MOR-TECHNO-PROJECT

## ESP32-CAM setup code

This repository includes an Arduino sketch for an AI Thinker ESP32-CAM module:

- `esp32_cam_setup/esp32_cam_setup.ino`

The sketch connects the ESP32-CAM to Wi-Fi, starts a small web server, and provides:

- `/` - test page
- `/capture` - single JPEG snapshot
- `/stream` - live MJPEG stream
- `/analyze` - capture one image and ask Gemini to identify objects
- `/flash/on` and `/flash/off` - onboard flash LED control

### Requirements

- AI Thinker ESP32-CAM board
- USB-to-Serial adapter
- Arduino IDE
- ESP32 board package installed in Arduino IDE

To install the ESP32 board package:

1. Open Arduino IDE.
2. Go to **File > Preferences**.
3. Add this URL to **Additional Boards Manager URLs**:

   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

4. Go to **Tools > Board > Boards Manager**.
5. Search for `esp32` and install the package from Espressif Systems.

### Wiring for upload

Connect the USB-to-Serial adapter to the ESP32-CAM:

| USB-to-Serial adapter | ESP32-CAM |
| --- | --- |
| 5V | 5V |
| GND | GND |
| TX | U0R |
| RX | U0T |
| GND | IO0 |

Power the ESP32-CAM from the adapter's `5V` pin or another stable 5V supply. If your adapter has a separate voltage setting for UART logic, use 3.3V serial logic for `TX` and `RX`.

`IO0` must be connected to `GND` only while uploading. After upload, disconnect `IO0` from `GND` and press the reset button.

### Arduino IDE settings

Use these common settings:

- Board: **AI Thinker ESP32-CAM**
- Upload Speed: **115200**
- Flash Frequency: **40MHz**
- Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)**

### How to use

1. Open `esp32_cam_setup/esp32_cam_setup.ino` in Arduino IDE.
2. Replace these values with your Wi-Fi details:

   ```cpp
   const char *WIFI_SSID = "YOUR_WIFI_NAME";
   const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   ```

3. Optional: add a Gemini API key if you want object analysis:

   ```cpp
   const char *GEMINI_API_KEY = "YOUR_GEMINI_API_KEY";
   ```

4. Connect `IO0` to `GND`.
5. Click **Upload** in Arduino IDE.
6. Disconnect `IO0` from `GND`.
7. Press the ESP32-CAM reset button.
8. Open **Tools > Serial Monitor** at `115200` baud.
9. Copy the printed IP address into your browser.

Example:

```text
http://192.168.1.50
```

Make sure your computer or phone is connected to the same Wi-Fi network as the ESP32-CAM.

### Gemini object analyzer

The `/analyze` route captures one still image and sends it to the Gemini API for object identification.

Configure these values near the top of `esp32_cam_setup.ino`:

```cpp
const char *GEMINI_API_KEY = "YOUR_GEMINI_API_KEY";
const char *GEMINI_MODEL = "gemini-2.0-flash";
const char *GEMINI_PROMPT =
    "Identify the main objects visible in this ESP32-CAM image. "
    "Return a short, clear bullet list and mention anything important or unusual.";
```

To use it:

1. Get a Gemini API key from Google AI Studio.
2. Paste the key into `GEMINI_API_KEY`.
3. Upload the sketch again.
4. Open the ESP32-CAM IP address in your browser.
5. Click **Analyze objects with Gemini** or open:

   ```text
   http://YOUR_ESP32_IP/analyze
   ```

Close the live `/stream` page before running `/analyze`. The ESP32-CAM web server handles one long stream at a time, so an open stream can block analysis requests.

For reliability, Gemini analysis uses a smaller still image than the live stream:

```cpp
const framesize_t GEMINI_ANALYSIS_FRAME_SIZE = FRAMESIZE_QVGA;
```

If Gemini misses details, try `FRAMESIZE_VGA`. If the ESP32 runs out of memory or the request fails, use `FRAMESIZE_QVGA`.

Do not commit a real Gemini API key to a public repository.

### Adjust video size and lag

The sketch uses a larger default frame:

```cpp
const framesize_t CAMERA_FRAME_SIZE = FRAMESIZE_VGA;
const int CAMERA_JPEG_QUALITY = 15;
const int STREAM_FRAME_DELAY_MS = 10;
```

`FRAMESIZE_VGA` is `640x480`. If the stream becomes laggy again:

1. Keep the ESP32-CAM close to the Wi-Fi router.
2. Use the `/stream` page directly instead of keeping multiple browser tabs open.
3. Make sure only one device is watching the stream.
4. Lower the image size to `320x240`:

   ```cpp
   const framesize_t CAMERA_FRAME_SIZE = FRAMESIZE_QVGA;
   ```

5. Lower JPEG quality to make smaller frames:

   ```cpp
   const int CAMERA_JPEG_QUALITY = 18;
   ```

If the stream is smooth and you want even better image quality, try:

```cpp
const int CAMERA_JPEG_QUALITY = 12;
```

Higher resolution needs stronger Wi-Fi and may add delay.

### Fix upload error: `A serial exception error occurred: Write timeout`

This error happens before the code starts running. It usually means the computer cannot reliably talk to the ESP32-CAM over the USB-to-Serial adapter.

Try these checks in order:

1. Close Serial Monitor or any other app using the same port.
2. Confirm **Tools > Port** is the USB-to-Serial adapter port.
3. Use a data USB cable, not a charge-only cable.
4. Confirm `TX` and `RX` are crossed:
   - Adapter `TX` -> ESP32-CAM `U0R`
   - Adapter `RX` -> ESP32-CAM `U0T`
5. Confirm all grounds are connected together.
6. Power the ESP32-CAM from stable 5V. Weak USB power is a common cause of timeouts.
7. Connect `IO0` to `GND` before clicking **Upload**.
8. When Arduino IDE shows `Connecting...`, press and release the ESP32-CAM reset button once.
9. If it still fails, change **Upload Speed** to `57600` or keep it at `115200`.
10. Install or update the driver for your USB-to-Serial chip:
    - CH340/CH341 driver for CH340 adapters
    - CP210x driver for CP2102 adapters

After a successful upload:

1. Disconnect `IO0` from `GND`.
2. Press reset.
3. Open Serial Monitor at `115200` baud.

### If Serial Monitor only shows boot text

Output like this means the ESP32-CAM is booting from flash:

```text
rst:0x3 (SW_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
entry 0x400805b4
```

After those lines, this sketch should print:

```text
ESP32-CAM setup sketch starting...
Initializing camera...
Camera initialized
Connecting to Wi-Fi...
```

If you only see the boot text and none of the sketch messages:

1. Make sure Serial Monitor baud is `115200`.
2. Press reset once after opening Serial Monitor.
3. Confirm `IO0` is disconnected from `GND` after upload.
4. Upload the latest sketch from this repository again.

If it stops at `Initializing camera...`, check the camera ribbon cable, select **AI Thinker ESP32-CAM**, and use a stable 5V supply.

If it keeps printing dots after `Connecting to Wi-Fi`, update `WIFI_SSID` and `WIFI_PASSWORD` in the sketch and make sure the ESP32-CAM can reach that Wi-Fi network.
