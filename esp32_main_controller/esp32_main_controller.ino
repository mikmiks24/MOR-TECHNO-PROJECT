#include <HTTPClient.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>

// Replace these with your Wi-Fi network details.
const char *WIFI_SSID = "YOUR_WIFI_NAME";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Use your laptop/backend Wi-Fi IP, not localhost.
// Example: http://192.168.23.100:8000
const char *BACKEND_URL = "http://YOUR_BACKEND_IP:8000";
const char *STATION_ID = "Station 1";

// Set to true for prototype enrollment through Serial Monitor.
// Set to false once employees are already registered in Supabase.
const bool REGISTER_UNKNOWN_CARDS = true;

// MFRC522 wiring for ESP32.
#define SS_PIN 21
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

String getUIDString(byte *uid, byte size) {
  String uidString = "";
  for (byte i = 0; i < size; i++) {
    uidString += String(uid[i] < 0x10 ? "0" : "");
    uidString += String(uid[i], HEX);
  }
  uidString.toUpperCase();
  return uidString;
}

String readSerialString() {
  while (Serial.available()) {
    Serial.read();
  }

  while (!Serial.available()) {
    delay(10);
  }

  String input = Serial.readStringUntil('\n');
  input.trim();
  return input;
}

String jsonEscape(String value) {
  value.replace("\\", "\\\\");
  value.replace("\"", "\\\"");
  value.replace("\n", "\\n");
  value.replace("\r", "");
  return value;
}

String extractJsonString(const String &json, const String &key) {
  String marker = "\"" + key + "\":\"";
  int start = json.indexOf(marker);
  if (start < 0) {
    return "";
  }

  start += marker.length();
  int end = json.indexOf("\"", start);
  if (end < 0) {
    return "";
  }

  return json.substring(start, end);
}

bool connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (millis() - startedAt > 30000) {
      Serial.println("\nWi-Fi connection timed out");
      return false;
    }
  }

  Serial.println();
  Serial.print("Connected. ESP32 main controller IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

int postJson(const String &path, const String &payload, String &response) {
  if (WiFi.status() != WL_CONNECTED) {
    response = "Wi-Fi is disconnected";
    return -1;
  }

  HTTPClient http;
  String url = String(BACKEND_URL) + path;

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  Serial.print("POST ");
  Serial.println(url);
  Serial.print("Payload: ");
  Serial.println(payload);

  int httpCode = http.POST(payload);
  response = http.getString();
  http.end();

  return httpCode;
}

bool registerEmployee(const String &uid) {
  Serial.println("Status: NEW CARD");
  Serial.println("Register this card through Serial Monitor.");

  Serial.print("Enter employee name: ");
  String name = readSerialString();
  Serial.println(name);

  Serial.print("Enter employee ID or username: ");
  String username = readSerialString();
  Serial.println(username);

  Serial.print("Enter role (staff/admin): ");
  String role = readSerialString();
  Serial.println(role);

  String payload = "{";
  payload += "\"name\":\"" + jsonEscape(name) + "\",";
  payload += "\"username\":\"" + jsonEscape(username) + "\",";
  payload += "\"role\":\"" + jsonEscape(role) + "\",";
  payload += "\"rfid_uid\":\"" + jsonEscape(uid) + "\"";
  payload += "}";

  String response;
  int httpCode = postJson("/api/employees", payload, response);

  Serial.print("Employee registration HTTP code: ");
  Serial.println(httpCode);
  Serial.println("Backend response:");
  Serial.println(response);

  return httpCode == 200;
}

void verifyRFID(const String &uid) {
  String payload = "{";
  payload += "\"rfid_uid\":\"" + jsonEscape(uid) + "\",";
  payload += "\"station_id\":\"" + jsonEscape(STATION_ID) + "\"";
  payload += "}";

  String response;
  int httpCode = postJson("/api/rfid/verify", payload, response);

  Serial.print("RFID verify HTTP code: ");
  Serial.println(httpCode);
  Serial.println("Backend response:");
  Serial.println(response);

  if (httpCode == 200) {
    String name = extractJsonString(response, "name");
    String role = extractJsonString(response, "role");

    Serial.println("ACCESS GRANTED");
    if (name.length() > 0) {
      Serial.print("Employee: ");
      Serial.println(name);
    }
    if (role.length() > 0) {
      Serial.print("Role: ");
      Serial.println(role);
    }

    // TFT integration point:
    // show "Access Granted" and the employee name, then show task menu.
    return;
  }

  if (httpCode == 403) {
    Serial.println("ACCESS DENIED: RFID card is not registered");
    if (REGISTER_UNKNOWN_CARDS) {
      if (registerEmployee(uid)) {
        Serial.println("Registration complete. Tap the card again to verify.");
      }
    }
    return;
  }

  Serial.println("Unexpected server response");
  Serial.println("Check BACKEND_URL, backend terminal logs, and JSON payload.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin();
  rfid.PCD_Init();

  Serial.println("=========================================");
  Serial.println("  TMS RFID Main Controller Starting");
  Serial.println("=========================================");
  Serial.print("Station ID: ");
  Serial.println(STATION_ID);
  Serial.print("Backend URL: ");
  Serial.println(BACKEND_URL);

  connectToWiFi();

  Serial.println("Scan an RFID tag...");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String scannedUID = getUIDString(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  Serial.print("[Card Detected] UID: ");
  Serial.println(scannedUID);

  verifyRFID(scannedUID);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1500);
  Serial.println("\nReady. Scan another card...");
}
