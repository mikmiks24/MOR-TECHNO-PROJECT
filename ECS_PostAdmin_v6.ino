/*
  ECS Post Admin v7
  ─────────────────
  ESP32 + MFRC522 RFID + Web UI

  Wiring (MFRC522 → ESP32):
    SDA(SS)=21  SCK=18  MOSI=23  MISO=19  RST=22  3.3V  GND

  Serial Monitor @ 115200 — boot prints RFID health + every card tap.
  Card registration: http://<IP>/register
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

const char* ssid     = "Connect";
const char* password = "passwordd";

#define ESP32_CAM_STREAM_URL "http://172.22.97.233/stream"

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

StaffEntry staffDB[] = {
  { "3696C906", "Joshua Zaide", "Senior Technician" },
  { "AABBCCDD", "Maria Santos", "Field Engineer"    },
};
const int STAFF_COUNT = sizeof(staffDB) / sizeof(staffDB[0]);

bool supabaseConfigured() {
  return strlen(SUPABASE_URL) > 10
      && strstr(SUPABASE_URL, "YOUR_PROJECT") == nullptr
      && strlen(SUPABASE_ANON_KEY) > 20
      && strstr(SUPABASE_ANON_KEY, "YOUR_SUPABASE") == nullptr;
}

String mapStaffId(const String& rfidUid) {
  for (int i = 0; i < STAFF_COUNT; i++) {
    if (rfidUid == String(staffDB[i].uid)) {
      if (staffDB[i].name == String("Joshua Zaide")) return "STF-JZ";
      if (staffDB[i].name == String("Maria Santos")) return "STF-MS";
    }
  }
  return "STF-" + rfidUid.substring(0, min(6, (int)rfidUid.length()));
}

String stationForTask(const char* task) {
  if (strcmp(task, "receive") == 0) return "Receiving";
  if (strcmp(task, "release") == 0) return "Release";
  return "Processing";
}

String terminalStatusForTask(const char* task) {
  if (strcmp(task, "receive") == 0) return "in_queue";
  if (strcmp(task, "release") == 0) return "released";
  return "processing";
}

bool supabasePost(const String& table, const String& jsonBody, const char* prefer = "return=minimal", const char* onConflict = nullptr) {
  if (!supabaseConfigured()) return false;

  WiFiClientSecure client;
  client.setInsecure();

  String url = String(SUPABASE_URL) + "/rest/v1/" + table;
  if (onConflict) url += String("?on_conflict=") + onConflict;
  HTTPClient https;
  if (!https.begin(client, url)) return false;

  https.addHeader("apikey", SUPABASE_ANON_KEY);
  https.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Prefer", prefer);

  int code = https.POST(jsonBody);
  String response = https.getString();
  https.end();

  Serial.printf("Supabase POST %s -> HTTP %d\n", table.c_str(), code);
  if (code < 200 || code >= 300) {
    Serial.println(response);
    return false;
  }
  return true;
}

bool saveTransactionToSupabase(JsonObject doc) {
  const char* terminalId = doc["terminal_id"] | "";
  const char* rfidUid = doc["staff_id"] | "";
  const char* task = doc["task"] | "receive";

  if (!terminalId[0] || !rfidUid[0]) return false;

  String staffId = mapStaffId(String(rfidUid));

  StaticJsonDocument<256> staffDoc;
  staffDoc["staff_id"] = staffId;
  staffDoc["name"] = doc["staff_name"] | "Unknown";
  staffDoc["role"] = doc["staff_role"] | "";
  staffDoc["station"] = stationForTask(task);
  staffDoc["rfid_uid"] = rfidUid;
  String staffJson;
  serializeJson(staffDoc, staffJson);
  supabasePost("staff", staffJson, "resolution=merge-duplicates,return=minimal", "staff_id");

  StaticJsonDocument<256> terminalDoc;
  terminalDoc["terminal_id"] = terminalId;
  terminalDoc["model"] = doc["terminal_model"] | "PAX A920";
  terminalDoc["brand"] = doc["terminal_brand"] | "PAX";
  terminalDoc["status"] = terminalStatusForTask(task);
  String terminalJson;
  serializeJson(terminalDoc, terminalJson);
  supabasePost("terminals", terminalJson, "resolution=merge-duplicates,return=minimal", "terminal_id");

  StaticJsonDocument<2048> logDoc;
  logDoc["terminal_id"] = terminalId;
  logDoc["staff_id"] = staffId;
  logDoc["station"] = stationForTask(task);
  logDoc["task"] = task;
  logDoc["timestamp"] = doc["ended_at"] | "";
  logDoc["photo_url"] = doc["photo_url"] | nullptr;

  JsonObject ai = logDoc.createNestedObject("ai_result");
  ai["staff_name"] = doc["staff_name"] | "";
  ai["staff_role"] = doc["staff_role"] | "";
  ai["rfid_uid"] = rfidUid;
  ai["workstation"] = WORKSTATION_STATION;
  ai["remarks"] = doc["remarks"] | "";
  ai["duration_sec"] = doc["duration_sec"] | 0;
  ai["items_checked"] = doc["items_checked"] | 0;
  ai["items_total"] = doc["items_total"] | 0;
  ai["camera_scan_count"] = doc["camera_scan_count"] | 0;
  ai["started_at"] = doc["started_at"] | "";
  ai["ended_at"] = doc["ended_at"] | "";
  ai["checklist"] = doc["checklist"];

  String logJson;
  serializeJson(logDoc, logJson);
  return supabasePost("logs", logJson);
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

  serialHeader("ECS POST ADMIN v7");
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

  server.on("/api/rfid", HTTP_GET, []() {
    StaticJsonDocument<128> doc;
    doc["ready"] = rfidReady;
    doc["version"] = readRfidVersion();
    doc["last_uid"] = lastRawUID;
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
  });

  server.on("/api/capture", HTTP_POST, []() {
    serialHeader("CAPTURE");
    Serial.println("Photo capture requested from UI.");
    serialRule();
    Serial.println();
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/transaction", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
      return;
    }

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
      server.send(400, "application/json", "{\"ok\":false,\"error\":\"bad json\"}");
      return;
    }

    serialHeader("TRANSACTION");
    Serial.println(server.arg("plain"));

    bool synced = saveTransactionToSupabase(doc.as<JsonObject>());
    Serial.println(synced ? "Cloud sync       : OK" : "Cloud sync       : skipped or failed");
    serialRule();
    Serial.println();

    StaticJsonDocument<128> out;
    out["ok"] = true;
    out["synced"] = synced;
    out["supabase"] = supabaseConfigured();
    String json;
    serializeJson(out, json);
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Web server running on port 80.");
  Serial.println(supabaseConfigured() ? "Supabase         : configured" : "Supabase         : NOT configured (edit config.h)");
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
  Serial.println("Card type        : " + String(mfrc522.PICC_GetTypeName(piccType)));
  Serial.println();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(800);
}
