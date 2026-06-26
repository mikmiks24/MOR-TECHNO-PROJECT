# MOR-TECHNO-PROJECT

## ESP32-CAM setup code

This repository includes an Arduino sketch for an AI Thinker ESP32-CAM module:

- `esp32_cam_setup/esp32_cam_setup.ino`

The sketch connects the ESP32-CAM to Wi-Fi, starts a small web server, and provides:

- `/` - test page
- `/capture` - single JPEG snapshot
- `/stream` - live MJPEG stream
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

3. Connect `IO0` to `GND`.
4. Click **Upload** in Arduino IDE.
5. Disconnect `IO0` from `GND`.
6. Press the ESP32-CAM reset button.
7. Open **Tools > Serial Monitor** at `115200` baud.
8. Copy the printed IP address into your browser.

Example:

```text
http://192.168.1.50
```

Make sure your computer or phone is connected to the same Wi-Fi network as the ESP32-CAM.

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
