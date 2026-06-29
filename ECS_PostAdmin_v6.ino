/*
  ECS Post Admin v8
  ─────────────────
  ESP32 + MFRC522 RFID + Web UI
  Workflow: Auto-Login -> Blind Scan -> Verification -> Incident Report
*/

#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "INDEX_HTML.h"
#include "config.h"

#define RFID_SS    21
#define RFID_RST   22
#define SPI_SCK    18
#define SPI_MISO   19
#define SPI_MOSI   23

const char* ssid     = "Miss Ka Ba";
const char* password = "jojoho04";

#define ESP32_CAM_STREAM_URL "http://10.193.115.233/stream"

MFRC522 mfrc522(RFID_SS, RFID_RST);
WebServer server(80);

bool   rfidReady         = false;
bool   newScanAvailable  = false;
String lastScannedUID    = "";
String lastScannedName   = "";
String lastScannedRole   = "";
String lastRawUID        = "";

unsigned long lastHeartbeatMs = 0;

struct StaffEntry {
  const char* uid;
  const char* name;
  const char* role;
};

// Database of registered cards
StaffEntry staffDB[] = {
  { "3696C906", "Joshua Zaide", "Senior Technician" },
  { "AABBCCDD", "Maria Santos", "Field Engineer"    },
};
const int STAFF_COUNT = sizeof(staffDB) / sizeof(staffDB[0]);


String mapStaffId(const String& rfidUid) {
  for (int i = 0; i < STAFF_COUNT; i++) {
    if (rfidUid == String(staffDB[i].uid)) {
      if (strcmp(staffDB[i].name, "Joshua Zaide") == 0) return "STF-JZ";
      if (strcmp(staffDB[i].name, "Maria Santos") == 0) return "STF-MS";
    }
  }
  return "STF-" + rfidUid.substring(0, min(6, (int)rfidUid.length()));
}

bool saveTransactionToBackend(const String& jsonBody) {
#ifdef BACKEND_URL
  if (strlen(BACKEND_URL) < 10 || strstr(BACKEND_URL, "YOUR_BACKEND_IP") != nullptr) {
    Serial.println("Backend URL not properly configured.");
    return false;
  }

  WiFiClient client;
  HTTPClient http;
  String url = String(BACKEND_URL) + "/api/transaction";
  
  if (!http.begin(client, url)) {
    Serial.println("HTTP connection failed");
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  int code = http.POST(jsonBody);
  String response = http.getString();
  http.end();

  Serial.printf("Backend POST %s -> HTTP %d\n", url.c_str(), code);
  if (code < 200 || code >= 300) {
    Serial.println(response);
    return false;
  }
  return true;
#else
  Serial.println("BACKEND_URL not defined.");
  return false;
#endif
}

void serialRule() {
  Serial.println("────────────────────────────────────────");
}

void serialHeader(const char* title) {
  serialRule();
  Serial.println(title);
  serialRule();
}

String uidToString() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void hardResetRfid() {
  pinMode(RFID_RST, OUTPUT);
  digitalWrite(RFID_RST, LOW);
  delay(50);
  digitalWrite(RFID_RST, HIGH);
  delay(50);
}

byte readRfidVersion() {
  return mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
}

void printVersionHelp(byte ver) {
  Serial.printf("Version register : 0x%02X\n", ver);
  if (ver == 0x91 || ver == 0x92) {
    Serial.println("Status           : OK — MFRC522 responding");
    rfidReady = true;
  } else if (ver == 0x88) {
    Serial.println("Status           : OK — clone chip (0x88)");
    rfidReady = true;
  } else if (ver == 0x00) {
    Serial.println("Status           : FAIL — check MISO, 3.3V power, GND");
    rfidReady = false;
  } else if (ver == 0xFF) {
    Serial.println("Status           : FAIL — check MOSI/SCK/SS or 5V damage");
    rfidReady = false;
  } else {
    Serial.println("Status           : UNEXPECTED — recheck all 7 wires");
    rfidReady = false;
  }
}

bool initRfid() {
  serialHeader("RFID INIT");
  Serial.println("Pins  SS=21 RST=22 SCK=18 MOSI=23 MISO=19");
  Serial.println();

  pinMode(RFID_SS, OUTPUT);
  digitalWrite(RFID_SS, HIGH);
  hardResetRfid();

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, RFID_SS);
  SPI.setFrequency(1000000);
  SPI.setDataMode(SPI_MODE0);

  mfrc522.PCD_Init();
  mfrc522.PCD_AntennaOn();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  delay(10);

  byte ver = readRfidVersion();
  printVersionHelp(ver);

  if (rfidReady) {
    Serial.print("Self-test        : ");
    if (mfrc522.PCD_PerformSelfTest()) {
      Serial.println("passed");
      mfrc522.PCD_Init();
      mfrc522.PCD_AntennaOn();
    } else {
      Serial.println("failed — wiring or module fault");
      rfidReady = false;
    }
  }

  Serial.println(rfidReady ? "\nRFID ready — tap a 13.56 MHz card." : "\nRFID NOT ready — fix hardware before continuing.");
  serialRule();
  Serial.println();
  return rfidReady;
}

void printCardEvent(const String& uid, const String& name, const String& role, bool registered) {
  serialHeader("CARD SCANNED");
  Serial.println("UID              : " + uid);
  Serial.println("Registered       : " + String(registered ? "yes" : "no"));
  Serial.println("Name             : " + name);
  Serial.println("Role             : " + role);
  Serial.println("Sent to UI       : yes");
  serialRule();
  Serial.println();
}

void heartbeat() {
  if (millis() - lastHeartbeatMs < 30000) return;
  lastHeartbeatMs = millis();
  Serial.println("[heartbeat] RFID " + String(rfidReady ? "ready" : "OFFLINE") + " — waiting for card…");
}

// ---------------------------------------------------------
// SERVER & ESP SETUP
// ---------------------------------------------------------

void sendProgmemPage(const char* html, bool replaceCam = false) {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  const char* ptr = html;
  size_t remaining = strlen_P(html);
  char chunk[1024];
  while (remaining > 0) {
    size_t toSend = min(remaining, sizeof(chunk) - 1);
    memcpy_P(chunk, ptr, toSend);
    chunk[toSend] = '\0';
    String s = String(chunk);
    if (replaceCam) s.replace("__CAM_URL__", ESP32_CAM_STREAM_URL);
    server.sendContent(s);
    ptr += toSend;
    remaining -= toSend;
  }
  server.sendContent("");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  serialHeader("ECS POST ADMIN v8 (Split-Phase Scanning)");
  Serial.println("Open Serial Monitor at 115200 baud.");
  Serial.println();

  initRfid();

  Serial.print("Wi-Fi connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wi-Fi connected");
  Serial.println("IP (workstation) : http://" + WiFi.localIP().toString());
  Serial.println("IP (register)    : http://" + WiFi.localIP().toString() + "/register");
  serialRule();
  Serial.println();

  server.on("/", HTTP_GET, []() { sendProgmemPage(INDEX_HTML, true); });
  server.on("/register", HTTP_GET, []() { sendProgmemPage(REGISTER_HTML, false); });

  server.on("/api/status", HTTP_GET, []() {
    StaticJsonDocument<256> doc;
    doc["scanned"] = newScanAvailable;
    doc["rfid_ok"] = rfidReady;
    if (newScanAvailable) {
      doc["uid"]  = lastScannedUID;
      doc["name"] = lastScannedName;
      doc["role"] = lastScannedRole;
      newScanAvailable = false;
    }
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });

  server.on("/api/rawuid", HTTP_GET, []() {
    StaticJsonDocument<128> doc;
    doc["uid"] = lastRawUID;
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });

  server.on("/api/scan", HTTP_POST, []() {
    serialHeader("CAMERA ITEM SCAN");
    Serial.println("Triggering backend visual analysis from ESP32...");
    
#ifdef BACKEND_URL
    if (strlen(BACKEND_URL) < 10 || strstr(BACKEND_URL, "YOUR_") != nullptr) {
      server.send(500, "application/json", "{\"error\":\"Backend URL not configured\"}");
      return;
    }

    WiFiClient client;
    HTTPClient http;
    String url = String(BACKEND_URL) + "/api/visual/analyze-from-camera";
    
    if (!http.begin(client, url)) {
      server.send(502, "application/json", "{\"error\":\"Failed to connect to backend server\"}");
      return;
    }
    
    http.addHeader("Content-Type", "application/json");
    
    String body = server.hasArg("plain") ? server.arg("plain") : "{}";
    int code = http.POST(body);
    String response = http.getString();
    http.end();
    
    Serial.printf("Backend scan response -> HTTP %d\n", code);
    if (code <= 0) {
      server.send(502, "application/json", "{\"error\":\"Failed to connect to backend server\"}");
    } else {
      server.send(code, "application/json", response);
    }
#else
    server.send(500, "application/json", "{\"error\":\"BACKEND_URL not defined.\"}");
#endif
    serialRule();
    Serial.println();
  });

  server.on("/api/capture", HTTP_POST, []() {
    serialHeader("CAPTURE FINALIZE");
    Serial.println("Final photo / task completion triggered.");
    serialRule();
    Serial.println();
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/transaction", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
      return;
    }

    serialHeader("TRANSACTION SAVED");
    Serial.println(server.arg("plain"));

    bool synced = saveTransactionToBackend(server.arg("plain"));
    Serial.println(synced ? "Backend sync     : OK" : "Backend sync     : skipped or failed");
    serialRule();
    Serial.println();

    StaticJsonDocument<128> out;
    out["ok"] = true;
    out["synced"] = synced;
    String json;
    serializeJson(out, json);
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Web server running on port 80.");
  if (rfidReady) {
    Serial.println("Ready — tap a card to test.");
  } else {
    Serial.println("WARNING — RFID failed init. Fix wiring and press EN on the board.");
  }
  Serial.println();
}

void loop() {
  server.handleClient();
  heartbeat();

  if (!rfidReady) return;

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String uid = uidToString();
  lastRawUID = uid;

  String name = "Unknown card";
  String role = "Unregistered";
  bool registered = false;

  for (int i = 0; i < STAFF_COUNT; i++) {
    if (uid == String(staffDB[i].uid)) {
      name = staffDB[i].name;
      role = staffDB[i].role;
      registered = true;
      break;
    }
  }

  lastScannedUID   = uid;
  lastScannedName  = name;
  lastScannedRole  = role;
  newScanAvailable = true;

  printCardEvent(uid, name, role, registered);

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(800);
}