#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "mbedtls/base64.h"
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <string.h>

// Replace these with your Wi-Fi network details.
const char *WIFI_SSID = "YOUR_WIFI_NAME";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Optional: add a Gemini API key to enable /analyze object detection.
const char *GEMINI_API_KEY = "YOUR_GEMINI_API_KEY";
const char *GEMINI_MODEL = "gemini-2.0-flash";
const char *GEMINI_PROMPT =
    "Identify the main objects visible in this ESP32-CAM image. "
    "Return a short, clear bullet list and mention anything important or unusual.";

// Balanced defaults. Lower frame size if your Wi-Fi is laggy.
const framesize_t CAMERA_FRAME_SIZE = FRAMESIZE_VGA;   // 640x480
const framesize_t GEMINI_ANALYSIS_FRAME_SIZE = FRAMESIZE_QVGA;  // 320x240 keeps upload smaller
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

String htmlEscape(const String &value) {
  String escaped = value;
  escaped.replace("&", "&amp;");
  escaped.replace("<", "&lt;");
  escaped.replace(">", "&gt;");
  escaped.replace("\"", "&quot;");
  escaped.replace("'", "&#39;");
  return escaped;
}

String jsonEscape(const String &value) {
  String escaped;
  escaped.reserve(value.length() + 16);

  for (size_t i = 0; i < value.length(); i++) {
    char character = value.charAt(i);
    switch (character) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped += character;
        break;
    }
  }

  return escaped;
}

String jsonUnescape(const String &value) {
  String unescaped;
  unescaped.reserve(value.length());

  for (size_t i = 0; i < value.length(); i++) {
    char character = value.charAt(i);
    if (character != '\\' || i + 1 >= value.length()) {
      unescaped += character;
      continue;
    }

    char next = value.charAt(++i);
    switch (next) {
      case 'n':
        unescaped += '\n';
        break;
      case 'r':
        unescaped += '\r';
        break;
      case 't':
        unescaped += '\t';
        break;
      case '"':
      case '\\':
      case '/':
        unescaped += next;
        break;
      default:
        unescaped += next;
        break;
    }
  }

  return unescaped;
}

bool isGeminiConfigured() {
  return strlen(GEMINI_API_KEY) > 0 && strcmp(GEMINI_API_KEY, "YOUR_GEMINI_API_KEY") != 0;
}

String base64EncodeFrame(camera_fb_t *frame) {
  size_t encodedLength = ((frame->len + 2) / 3) * 4;
  char *encoded = nullptr;

  if (psramFound()) {
    encoded = static_cast<char *>(heap_caps_malloc(encodedLength + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  }

  if (!encoded) {
    encoded = static_cast<char *>(malloc(encodedLength + 1));
  }

  if (!encoded) {
    Serial.println("Not enough memory to base64 encode image");
    return "";
  }

  size_t outputLength = 0;
  int result = mbedtls_base64_encode(reinterpret_cast<unsigned char *>(encoded),
                                     encodedLength + 1,
                                     &outputLength,
                                     frame->buf,
                                     frame->len);
  if (result != 0) {
    Serial.printf("Base64 encode failed with error %d\n", result);
    free(encoded);
    return "";
  }

  encoded[outputLength] = '\0';

  String base64Image;
  base64Image.reserve(outputLength);
  base64Image = encoded;
  free(encoded);

  return base64Image;
}

String buildGeminiRequest(const String &base64Image) {
  String request;
  request.reserve(base64Image.length() + 512);
  request += "{\"contents\":[{\"parts\":[{\"text\":\"";
  request += jsonEscape(GEMINI_PROMPT);
  request += "\"},{\"inline_data\":{\"mime_type\":\"image/jpeg\",\"data\":\"";
  request += base64Image;
  request += "\"}}]}],\"generationConfig\":{\"temperature\":0.2,\"maxOutputTokens\":256}}";
  return request;
}

String extractGeminiText(const String &response) {
  int keyIndex = response.indexOf("\"text\"");
  if (keyIndex < 0) {
    return response;
  }

  int colonIndex = response.indexOf(':', keyIndex);
  int textStart = response.indexOf('"', colonIndex + 1);
  if (colonIndex < 0 || textStart < 0) {
    return response;
  }

  String encodedText;
  bool escaped = false;
  for (int i = textStart + 1; i < response.length(); i++) {
    char character = response.charAt(i);
    if (!escaped && character == '"') {
      break;
    }

    encodedText += character;
    escaped = (!escaped && character == '\\');
    if (character != '\\') {
      escaped = false;
    }
  }

  return jsonUnescape(encodedText);
}

String analyzeImageWithGemini(camera_fb_t *frame, int &httpCode) {
  httpCode = 0;

  String base64Image = base64EncodeFrame(frame);
  if (base64Image.length() == 0) {
    return "Failed to encode image. Try lowering GEMINI_ANALYSIS_FRAME_SIZE.";
  }

  String request = buildGeminiRequest(base64Image);
  base64Image = "";

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(30000);

  HTTPClient https;
  String url = "https://generativelanguage.googleapis.com/v1beta/models/";
  url += GEMINI_MODEL;
  url += ":generateContent?key=";
  url += GEMINI_API_KEY;

  Serial.println("Sending image to Gemini...");

  if (!https.begin(client, url)) {
    return "Failed to start HTTPS connection to Gemini.";
  }

  https.setTimeout(30000);
  https.addHeader("Content-Type", "application/json");
  httpCode = https.POST(request);
  request = "";

  String response = https.getString();
  https.end();

  if (httpCode != HTTP_CODE_OK) {
    String error = "Gemini request failed with HTTP ";
    error += httpCode;
    error += "\n\n";
    error += response;
    return error;
  }

  return extractGeminiText(response);
}

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
      "<p><a href='/analyze'>Analyze objects with Gemini</a></p>"
      "<p><a href='/flash/on'>Flash on</a> <a href='/flash/off'>Flash off</a></p>"
      "<p><img src='/capture' alt='ESP32-CAM snapshot'></p>"
      "</body></html>";

  server.send(200, "text/html", page);
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

void handleAnalyze() {
  if (!isGeminiConfigured()) {
    server.send(500,
                "text/html",
                "<!doctype html><html><body>"
                "<h1>Gemini API key is not configured</h1>"
                "<p>Edit <code>GEMINI_API_KEY</code> in the sketch, upload again, then retry.</p>"
                "<p><a href='/'>Back</a></p>"
                "</body></html>");
    return;
  }

  sensor_t *sensor = esp_camera_sensor_get();
  framesize_t originalFrameSize = CAMERA_FRAME_SIZE;
  if (sensor) {
    originalFrameSize = static_cast<framesize_t>(sensor->status.framesize);
    if (originalFrameSize != GEMINI_ANALYSIS_FRAME_SIZE) {
      sensor->set_framesize(sensor, GEMINI_ANALYSIS_FRAME_SIZE);
      delay(300);
      camera_fb_t *staleFrame = esp_camera_fb_get();
      if (staleFrame) {
        esp_camera_fb_return(staleFrame);
      }
    }
  }

  camera_fb_t *frame = esp_camera_fb_get();

  if (sensor && originalFrameSize != GEMINI_ANALYSIS_FRAME_SIZE) {
    sensor->set_framesize(sensor, originalFrameSize);
  }

  if (!frame) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  Serial.printf("Analyzing image with Gemini (%u bytes)\n", frame->len);
  int httpCode = 0;
  String result = analyzeImageWithGemini(frame, httpCode);
  esp_camera_fb_return(frame);

  String page;
  page.reserve(result.length() + 512);
  page += "<!doctype html><html><head>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<title>Gemini Object Analysis</title>";
  page += "<style>body{font-family:Arial;margin:2rem;background:#111;color:#eee;}";
  page += "a{color:#90caf9;}pre{white-space:pre-wrap;background:#222;padding:1rem;border-radius:4px;}</style>";
  page += "</head><body><h1>Gemini Object Analysis</h1>";
  page += "<p><a href='/'>Back</a> | <a href='/analyze'>Analyze again</a></p>";
  if (httpCode == HTTP_CODE_OK) {
    page += "<h2>Detected objects</h2>";
  } else {
    page += "<h2>Analysis error</h2>";
  }
  page += "<pre>";
  page += htmlEscape(result);
  page += "</pre></body></html>";

  server.send(httpCode == HTTP_CODE_OK ? 200 : 500, "text/html", page);
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
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/analyze", HTTP_GET, handleAnalyze);
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
  Serial.print("Gemini analysis frame size: ");
  Serial.println(static_cast<int>(GEMINI_ANALYSIS_FRAME_SIZE));

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
