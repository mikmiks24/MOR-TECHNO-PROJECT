#include "esp_camera.h"
#include <WebServer.h>
#include <WiFi.h>

// Replace these with your Wi-Fi network details.
const char *WIFI_SSID = "YOUR_WIFI_NAME";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Optional labels that the backend can use for traceability.
const char *DEVICE_ID = "ESP32-CAM-01";
const char *STATION_ID = "Station 1";

// Balanced defaults. Lower frame size if your Wi-Fi is laggy.
const framesize_t CAMERA_FRAME_SIZE = FRAMESIZE_VGA;   // 640x480
const int CAMERA_JPEG_QUALITY = 15;                    // 10 = sharper/larger, 20 = faster/smaller
const int STREAM_FRAME_DELAY_MS = 10;                  // Small delay keeps the stream responsive

// AI Thinker ESP32-CAM pin map.
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define FLASH_LED_PIN 4

WebServer server(80);

void handleRoot() {
  String page =
      "<!doctype html><html><head>"
      "<meta name='viewport' content='width=device-width, initial-scale=1'>"
      "<title>ESP32-CAM</title>"
      "<style>body{font-family:Arial;margin:2rem;background:#111;color:#eee;}"
      "a,button{display:inline-block;margin:.5rem 0;padding:.7rem 1rem;"
      "background:#1976d2;color:#fff;text-decoration:none;border:0;border-radius:4px;}"
      "img{max-width:100%;height:auto;border:1px solid #444;}</style>"
      "</head><body>"
      "<h1>ESP32-CAM is running</h1>"
      "<p>Use the links below to test the camera.</p>"
      "<p><a href='/capture'>Take snapshot</a></p>"
      "<p><a href='/stream'>Open live stream</a></p>"
      "<p><a href='/info'>Device info</a></p>"
      "<p><a href='/flash/on'>Flash on</a> <a href='/flash/off'>Flash off</a></p>"
      "<p><img src='/capture' alt='ESP32-CAM snapshot'></p>"
      "<p>AI analysis is handled by the backend using this camera's <code>/capture</code> endpoint.</p>"
      "</body></html>";

  server.send(200, "text/html", page);
}

void handleInfo() {
  String json = "{";
  json += "\"device_id\":\"";
  json += DEVICE_ID;
  json += "\",\"station_id\":\"";
  json += STATION_ID;
  json += "\",\"capture_url\":\"http://";
  json += WiFi.localIP().toString();
  json += "/capture\",\"stream_url\":\"http://";
  json += WiFi.localIP().toString();
  json += "/stream\"}";

  server.send(200, "application/json", json);
}

void handleCapture() {
  camera_fb_t *frame = esp_camera_fb_get();
  if (!frame) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  WiFiClient client = server.client();
  client.setNoDelay(true);
  client.print(
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: image/jpeg\r\n"
      "Content-Disposition: inline; filename=capture.jpg\r\n"
      "Content-Length: ");
  client.print(frame->len);
  client.print("\r\nConnection: close\r\n\r\n");
  client.write(frame->buf, frame->len);

  esp_camera_fb_return(frame);
}

void handleStream() {
  WiFiClient client = server.client();
  client.setNoDelay(true);

  client.print(
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
      "Cache-Control: no-cache\r\n"
      "Connection: close\r\n\r\n");

  while (client.connected()) {
    camera_fb_t *frame = esp_camera_fb_get();
    if (!frame) {
      Serial.println("Camera capture failed while streaming");
      break;
    }

    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Length: ");
    client.print(frame->len);
    client.print("\r\n\r\n");
    client.write(frame->buf, frame->len);
    client.print("\r\n");

    esp_camera_fb_return(frame);

    if (!client.connected()) {
      break;
    }
    delay(STREAM_FRAME_DELAY_MS);
  }
}

void handleFlashOn() {
  digitalWrite(FLASH_LED_PIN, HIGH);
  server.send(200, "text/plain", "Flash is on");
}

void handleFlashOff() {
  digitalWrite(FLASH_LED_PIN, LOW);
  server.send(200, "text/plain", "Flash is off");
}

void startCameraServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/info", HTTP_GET, handleInfo);
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/flash/on", HTTP_GET, handleFlashOn);
  server.on("/flash/off", HTTP_GET, handleFlashOff);
  server.begin();
}

void setupCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  config.frame_size = CAMERA_FRAME_SIZE;
  config.jpeg_quality = CAMERA_JPEG_QUALITY;
  config.fb_count = psramFound() ? 2 : 1;

  Serial.println("Initializing camera...");
  Serial.print("PSRAM found: ");
  Serial.println(psramFound() ? "yes" : "no");
  Serial.print("Camera frame size: ");
  Serial.println(static_cast<int>(CAMERA_FRAME_SIZE));
  Serial.print("JPEG quality: ");
  Serial.println(CAMERA_JPEG_QUALITY);

  esp_err_t error = esp_camera_init(&config);
  if (error != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", error);
    Serial.println("Check the camera ribbon cable, board model, and 5V power supply.");
    Serial.flush();
    while (true) {
      delay(1000);
    }
  }

  Serial.println("Camera initialized");
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected. Open http://");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  delay(1000);

  Serial.println();
  Serial.println("ESP32-CAM setup sketch starting...");
  Serial.print("Device ID: ");
  Serial.println(DEVICE_ID);
  Serial.print("Station ID: ");
  Serial.println(STATION_ID);
  Serial.print("Wi-Fi SSID: ");
  Serial.println(WIFI_SSID);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  setupCamera();
  connectToWiFi();
  startCameraServer();

  Serial.println("Camera server started");
}

void loop() {
  server.handleClient();
}
