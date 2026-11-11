/*
  ========================================================
  ECS POST ADMIN - HTTP POLLING MVP v3
  Hardware: ESP32 + MFRC522 RFID
  UI Flow: Lock → Welcome → Task Select → AI Scan + Checklist → Remarks → Confirm → Auto-Lock
  Camera: ESP32-CAM stream via IP (configured below)
  AI: Gemini Vision auto-checks detected peripherals
  ========================================================
*/

#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

// --- PIN DEFINITIONS ---
#define RFID_SS   21
#define RFID_RST  22

// --- NETWORK CONFIG ---
const char* ssid     = "Connect";
const char* password = "passwordd";

// ⚠️ Set this to your ESP32-CAM's IP address + stream port
#define ESP32_CAM_STREAM_URL "http://172.22.97.233:81/stream"

// --- INSTANCES ---
MFRC522 mfrc522(RFID_SS, RFID_RST);
WebServer server(80);

// --- GLOBAL STATE ---
bool   newScanAvailable = false;
String lastScannedUID   = "";
String lastScannedName  = "";
String lastScannedRole  = "";

// ==========================================
// STAFF DATABASE — add cards here
// ==========================================
struct StaffEntry {
  const char* uid;
  const char* name;
  const char* role;
};

StaffEntry staffDB[] = {
  { "3696C906", "Joshua Zaide",   "Senior Technician" },
  { "AABBCCDD", "Maria Santos",   "Field Engineer"    },
};
const int STAFF_COUNT = sizeof(staffDB) / sizeof(staffDB[0]);

// ==========================================
// HTML / CSS / JS
// ==========================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>ECS Post Admin</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    :root {
      --bg:          #07090f;
      --surface:     #10151f;
      --surface-2:   #161d2b;
      --border:      rgba(255,255,255,0.07);
      --border-hi:   rgba(255,255,255,0.14);
      --text:        #e2e8f0;
      --text-muted:  #64748b;
      --green:       #22c55e;
      --green-dim:   rgba(34,197,94,0.12);
      --yellow:      #f59e0b;
      --yellow-dim:  rgba(245,158,11,0.12);
      --blue:        #3b82f6;
      --blue-dim:    rgba(59,130,246,0.12);
      --red:         #ef4444;
      --purple:      #a855f7;
      --purple-dim:  rgba(168,85,247,0.12);
      --font:        'SF Pro Display', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
      --radius:      16px;
      --transition:  0.25s cubic-bezier(0.4,0,0.2,1);
    }

    html, body {
      height: 100%; width: 100%;
      font-family: var(--font);
      background: var(--bg);
      color: var(--text);
      overflow: hidden;
    }

    #app {
      height: 100vh;
      display: flex;
      flex-direction: column;
    }

    #topbar {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 14px 24px;
      background: var(--surface);
      border-bottom: 1px solid var(--border);
      flex-shrink: 0;
    }
    #topbar .brand { font-size: 13px; font-weight: 700; letter-spacing: 2px; color: var(--text-muted); text-transform: uppercase; }
    #topbar .user-pill {
      display: flex; align-items: center; gap: 10px;
      background: var(--surface-2); border: 1px solid var(--border-hi);
      padding: 7px 14px; border-radius: 40px;
      font-size: 14px; font-weight: 600;
    }
    #topbar .user-pill .dot {
      width: 8px; height: 8px; border-radius: 50%; background: var(--green);
      box-shadow: 0 0 8px var(--green);
    }
    #topbar .system-badge {
      font-size: 12px; letter-spacing: 1px; font-weight: 700;
      color: var(--green); background: var(--green-dim);
      border: 1px solid rgba(34,197,94,0.3);
      padding: 6px 14px; border-radius: 40px;
    }

    .screen { display: none; flex: 1; overflow: hidden; }
    .screen.active { display: flex; }

    @keyframes fadeUp {
      from { opacity: 0; transform: translateY(14px); }
      to   { opacity: 1; transform: translateY(0); }
    }
    .screen.active { animation: fadeUp 0.3s ease-out; }

    /* =====================
       SCREEN 1: LOCK
    ===================== */
    #screen-lock {
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 24px;
    }
    .lock-ring {
      width: 140px; height: 140px; border-radius: 50%;
      border: 2px solid var(--border-hi);
      display: flex; align-items: center; justify-content: center;
      position: relative;
      animation: ring-pulse 3s ease-in-out infinite;
    }
    .lock-ring::before {
      content: '';
      position: absolute; inset: -10px; border-radius: 50%;
      border: 1px solid rgba(255,255,255,0.04);
      animation: ring-pulse 3s ease-in-out infinite 0.5s;
    }
    @keyframes ring-pulse {
      0%, 100% { box-shadow: 0 0 0 0 rgba(255,255,255,0.05); }
      50%       { box-shadow: 0 0 0 20px rgba(255,255,255,0); }
    }
    .lock-icon { font-size: 52px; }
    .lock-title { font-size: 28px; font-weight: 700; letter-spacing: -0.5px; }
    .lock-sub {
      font-size: 15px; color: var(--text-muted);
      animation: blink 2.5s ease-in-out infinite;
    }
    @keyframes blink { 0%,100%{opacity:0.4} 50%{opacity:1} }
    .scan-indicator {
      display: flex; align-items: center; gap: 12px;
      background: var(--surface); border: 1px solid var(--border-hi);
      padding: 14px 24px; border-radius: 12px; font-size: 14px; color: var(--text-muted);
    }
    .scan-indicator .rfid-icon { font-size: 22px; }

    /* =====================
       SCREEN 1.5: WELCOME
    ===================== */
    #screen-welcome {
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 0;
      position: relative;
    }
    .welcome-glow {
      position: absolute;
      width: 400px; height: 400px; border-radius: 50%;
      background: radial-gradient(circle, rgba(34,197,94,0.08) 0%, transparent 70%);
      pointer-events: none;
    }
    .welcome-avatar {
      width: 96px; height: 96px; border-radius: 50%;
      background: var(--green-dim);
      border: 2px solid rgba(34,197,94,0.4);
      display: flex; align-items: center; justify-content: center;
      font-size: 40px;
      margin-bottom: 24px;
      box-shadow: 0 0 32px rgba(34,197,94,0.15);
    }
    .welcome-tag {
      font-size: 12px; font-weight: 700; letter-spacing: 2.5px;
      text-transform: uppercase; color: var(--green);
      margin-bottom: 12px;
    }
    .welcome-name {
      font-size: 42px; font-weight: 800; letter-spacing: -1.5px;
      line-height: 1; margin-bottom: 10px; text-align: center;
    }
    .welcome-role {
      font-size: 16px; color: var(--text-muted); margin-bottom: 40px;
    }
    .welcome-continue-btn {
      padding: 16px 48px;
      background: var(--green);
      color: #000; font-size: 16px; font-weight: 800;
      border: none; border-radius: 14px;
      cursor: pointer; letter-spacing: 0.5px;
      transition: var(--transition);
      box-shadow: 0 8px 24px rgba(34,197,94,0.25);
    }
    .welcome-continue-btn:hover { filter: brightness(1.08); transform: translateY(-2px); }
    .welcome-time {
      margin-top: 20px; font-size: 13px; color: var(--text-muted);
    }

    /* =====================
       SCREEN 2: TASK SELECT
    ===================== */
    #screen-menu {
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 32px;
      gap: 28px;
    }
    .menu-greeting { text-align: center; }
    .menu-greeting .welcome { font-size: 15px; color: var(--text-muted); margin-bottom: 6px; }
    .menu-greeting .name { font-size: 30px; font-weight: 700; letter-spacing: -0.5px; }
    .menu-greeting .role { font-size: 14px; color: var(--text-muted); margin-top: 4px; }
    .menu-prompt { font-size: 13px; color: var(--text-muted); letter-spacing: 1.5px; text-transform: uppercase; }

    .task-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 20px;
      width: 100%; max-width: 860px;
    }
    .task-card {
      background: var(--surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 32px 24px;
      cursor: pointer;
      transition: var(--transition);
      display: flex; flex-direction: column; align-items: center;
      gap: 16px; text-align: center;
      position: relative; overflow: hidden;
    }
    .task-card::after {
      content: ''; position: absolute;
      bottom: 0; left: 0; right: 0; height: 3px;
    }
    .task-card.receive::after  { background: var(--green); }
    .task-card.process::after  { background: var(--yellow); }
    .task-card.release::after  { background: var(--blue); }
    .task-card:hover {
      background: var(--surface-2);
      border-color: var(--border-hi);
      transform: translateY(-4px);
      box-shadow: 0 16px 40px rgba(0,0,0,0.4);
    }
    .task-card .task-num { font-size: 11px; font-weight: 700; letter-spacing: 2px; color: var(--text-muted); text-transform: uppercase; }
    .task-card .task-emoji { font-size: 40px; }
    .task-card .task-name { font-size: 17px; font-weight: 700; }
    .task-card .task-desc { font-size: 13px; color: var(--text-muted); line-height: 1.5; }
    .task-card.receive .task-name  { color: var(--green); }
    .task-card.process .task-name  { color: var(--yellow); }
    .task-card.release .task-name  { color: var(--blue); }

    .menu-footer { font-size: 13px; color: var(--text-muted); }
    .logout-btn {
      background: transparent; border: 1px solid rgba(239,68,68,0.3);
      color: var(--red); padding: 8px 20px; border-radius: 8px;
      cursor: pointer; font-size: 13px; font-weight: 600;
      transition: var(--transition);
    }
    .logout-btn:hover { background: rgba(239,68,68,0.1); }

    /* =====================
       SCREEN 3: TASK
    ===================== */
    #screen-task { flex-direction: column; }
    .task-header {
      display: flex; align-items: center; justify-content: space-between;
      padding: 16px 24px;
      background: var(--surface);
      border-bottom: 1px solid var(--border);
      flex-shrink: 0;
    }
    .task-header-left { display: flex; align-items: center; gap: 14px; }
    .task-badge { padding: 6px 14px; border-radius: 8px; font-size: 12px; font-weight: 700; letter-spacing: 1px; }
    .task-badge.receive { background: var(--green-dim); color: var(--green); border: 1px solid rgba(34,197,94,0.3); }
    .task-badge.process { background: var(--yellow-dim); color: var(--yellow); border: 1px solid rgba(245,158,11,0.3); }
    .task-badge.release { background: var(--blue-dim); color: var(--blue); border: 1px solid rgba(59,130,246,0.3); }
    .task-header-title { font-size: 18px; font-weight: 700; }
    .task-header-sub { font-size: 13px; color: var(--text-muted); margin-top: 2px; }
    .cancel-btn {
      background: transparent; border: 1px solid rgba(239,68,68,0.3);
      color: var(--red); padding: 8px 18px; border-radius: 8px;
      cursor: pointer; font-size: 13px; font-weight: 600;
      transition: var(--transition);
    }
    .cancel-btn:hover { background: rgba(239,68,68,0.1); }

    .task-body {
      display: grid; grid-template-columns: 1fr 1fr;
      flex: 1; overflow: hidden;
    }

    /* Checklist panel */
    .checklist-panel {
      display: flex; flex-direction: column;
      border-right: 1px solid var(--border);
      overflow: hidden;
    }
    .panel-label {
      padding: 14px 20px;
      font-size: 11px; font-weight: 700; letter-spacing: 2px;
      color: var(--text-muted); text-transform: uppercase;
      border-bottom: 1px solid var(--border);
      flex-shrink: 0;
      display: flex; justify-content: space-between; align-items: center;
    }
    .progress-text { font-size: 12px; font-weight: 600; color: var(--text); letter-spacing: 0; }

    .checklist-items { flex: 1; overflow-y: auto; padding: 8px 0; }
    .checklist-items::-webkit-scrollbar { width: 4px; }
    .checklist-items::-webkit-scrollbar-thumb { background: var(--border-hi); border-radius: 4px; }

    .check-row {
      display: flex; align-items: center; gap: 14px;
      padding: 12px 20px;
      border-bottom: 1px solid var(--border);
      cursor: pointer;
      transition: background var(--transition);
      user-select: none;
    }
    .check-row:last-child { border-bottom: none; }
    .check-row:hover { background: var(--surface-2); }
    .check-row.checked { background: rgba(34,197,94,0.04); }
    .check-row.ai-detected { background: rgba(168,85,247,0.04); }
    .check-row.manual-needed { background: rgba(245,158,11,0.04); }

    .check-box {
      width: 22px; height: 22px; border-radius: 6px; flex-shrink: 0;
      border: 2px solid var(--border-hi);
      display: flex; align-items: center; justify-content: center;
      transition: var(--transition); font-size: 13px;
    }
    .check-row.checked .check-box { background: var(--green); border-color: var(--green); }
    .check-row.ai-detected .check-box { background: var(--purple); border-color: var(--purple); }

    .check-label-col { flex: 1; }
    .check-label { font-size: 14px; }
    .check-row.checked .check-label { color: var(--text-muted); text-decoration: line-through; }
    .check-ai-tag {
      font-size: 10px; font-weight: 700; letter-spacing: 1px;
      padding: 2px 7px; border-radius: 4px; margin-top: 4px; display: inline-block;
    }
    .check-ai-tag.auto { background: var(--purple-dim); color: var(--purple); }
    .check-ai-tag.manual { background: var(--yellow-dim); color: var(--yellow); }
    .check-required { font-size: 10px; font-weight: 700; color: var(--red); letter-spacing: 1px; flex-shrink: 0; }

    /* Manual confirm prompt (SIM cards) */
    .manual-confirm-row {
      display: flex; gap: 6px; margin-top: 6px;
    }
    .mc-btn {
      flex: 1; padding: 5px 8px; border-radius: 6px; border: 1px solid;
      font-size: 11px; font-weight: 700; cursor: pointer; letter-spacing: 0.5px;
      transition: var(--transition);
    }
    .mc-btn.yes { border-color: rgba(34,197,94,0.4); color: var(--green); background: transparent; }
    .mc-btn.yes:hover, .mc-btn.yes.active { background: var(--green); color: #000; border-color: var(--green); }
    .mc-btn.no  { border-color: rgba(239,68,68,0.4); color: var(--red); background: transparent; }
    .mc-btn.no:hover, .mc-btn.no.active  { background: var(--red); color: #fff; border-color: var(--red); }

    /* AI scan banner */
    .ai-scan-banner {
      margin: 10px 20px 0;
      padding: 10px 14px;
      background: var(--purple-dim);
      border: 1px solid rgba(168,85,247,0.3);
      border-radius: 10px;
      font-size: 12px; color: var(--purple);
      display: flex; align-items: center; gap: 8px;
      flex-shrink: 0;
    }
    .ai-scan-banner.hidden { display: none; }

    .checklist-footer {
      padding: 16px 20px;
      border-top: 1px solid var(--border);
      flex-shrink: 0;
    }
    .progress-bar-wrap {
      background: var(--surface-2); border-radius: 4px; height: 4px; margin-bottom: 14px; overflow: hidden;
    }
    .progress-bar-fill {
      height: 100%; background: var(--green);
      border-radius: 4px; transition: width 0.4s ease;
    }
    .footer-btns { display: flex; gap: 10px; }
    .ai-scan-btn {
      flex: 0 0 auto;
      padding: 14px 18px;
      border: 1px solid rgba(168,85,247,0.4);
      background: var(--purple-dim); color: var(--purple);
      border-radius: 12px; font-size: 14px; font-weight: 700;
      cursor: pointer; transition: var(--transition);
      display: flex; align-items: center; gap: 8px;
    }
    .ai-scan-btn:hover { background: rgba(168,85,247,0.2); }
    .ai-scan-btn:disabled { opacity: 0.4; cursor: not-allowed; }
    .capture-btn {
      flex: 1;
      padding: 14px;
      border: none; border-radius: 12px;
      font-size: 15px; font-weight: 700;
      cursor: pointer; transition: var(--transition);
      display: flex; align-items: center; justify-content: center; gap: 10px;
    }
    .capture-btn.ready { background: var(--green); color: #000; }
    .capture-btn.ready:hover { filter: brightness(1.1); transform: translateY(-1px); }
    .capture-btn.disabled { background: var(--surface-2); color: var(--text-muted); cursor: not-allowed; }
    .capture-btn.processing { background: var(--yellow-dim); color: var(--yellow); border: 1px solid rgba(245,158,11,0.3); }

    /* Camera panel */
    .camera-panel { display: flex; flex-direction: column; overflow: hidden; }
    .cam-wrap {
      flex: 1; background: #000;
      display: flex; align-items: center; justify-content: center;
      position: relative; overflow: hidden;
    }
    .cam-wrap img { width: 100%; height: 100%; object-fit: cover; }
    .cam-offline {
      display: none;
      flex-direction: column; align-items: center; gap: 12px;
      color: var(--text-muted); font-size: 14px; text-align: center; padding: 24px;
    }
    .cam-offline .cam-offline-icon { font-size: 40px; }
    .cam-footer {
      padding: 12px 20px;
      border-top: 1px solid var(--border);
      font-size: 12px; color: var(--text-muted);
      display: flex; align-items: center; gap: 8px; flex-shrink: 0;
    }
    .cam-dot { width: 7px; height: 7px; border-radius: 50%; background: var(--red); }
    .cam-dot.live { background: var(--green); animation: blink 1.5s infinite; }

    /* AI scanning overlay on cam */
    .ai-overlay {
      position: absolute; inset: 0;
      background: rgba(168,85,247,0.08);
      display: none; align-items: center; justify-content: center;
      flex-direction: column; gap: 12px;
    }
    .ai-overlay.active { display: flex; }
    .ai-spinner {
      width: 48px; height: 48px; border-radius: 50%;
      border: 3px solid rgba(168,85,247,0.2);
      border-top-color: var(--purple);
      animation: spin 0.8s linear infinite;
    }
    @keyframes spin { to { transform: rotate(360deg); } }
    .ai-overlay-text { color: var(--purple); font-size: 14px; font-weight: 600; }

    /* =====================
       SCREEN 3.5: REMARKS
    ===================== */
    #screen-remarks {
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 20px;
      padding: 40px 32px;
    }
    .remarks-card {
      background: var(--surface);
      border: 1px solid var(--border-hi);
      border-radius: 20px;
      padding: 36px 40px;
      width: 100%; max-width: 560px;
    }
    .remarks-title { font-size: 22px; font-weight: 700; margin-bottom: 6px; }
    .remarks-sub { font-size: 14px; color: var(--text-muted); margin-bottom: 24px; }
    .remarks-label {
      font-size: 11px; font-weight: 700; letter-spacing: 2px;
      text-transform: uppercase; color: var(--text-muted); margin-bottom: 10px;
    }
    .remarks-textarea {
      width: 100%; height: 130px;
      background: var(--surface-2); border: 1px solid var(--border-hi);
      border-radius: 12px; padding: 14px 16px;
      color: var(--text); font-family: var(--font); font-size: 15px;
      resize: none; outline: none; transition: border-color var(--transition);
    }
    .remarks-textarea:focus { border-color: rgba(59,130,246,0.5); }
    .remarks-textarea::placeholder { color: var(--text-muted); }
    .remarks-char { font-size: 11px; color: var(--text-muted); text-align: right; margin-top: 6px; }
    .remarks-hint {
      font-size: 12px; color: var(--text-muted); margin-top: 12px; margin-bottom: 24px;
      padding: 10px 14px; background: var(--surface-2); border-radius: 8px;
      line-height: 1.6;
    }
    .remarks-actions { display: flex; gap: 12px; }
    .remarks-skip-btn {
      flex: 1; padding: 15px;
      background: transparent; border: 1px solid var(--border-hi);
      color: var(--text-muted); border-radius: 12px;
      font-size: 15px; font-weight: 600; cursor: pointer;
      transition: var(--transition);
    }
    .remarks-skip-btn:hover { background: var(--surface-2); }
    .remarks-confirm-btn {
      flex: 2; padding: 15px;
      background: var(--green); color: #000;
      border: none; border-radius: 12px;
      font-size: 15px; font-weight: 800; cursor: pointer;
      transition: var(--transition);
    }
    .remarks-confirm-btn:hover { filter: brightness(1.08); transform: translateY(-1px); }

    /* =====================
       SCREEN 4: CONFIRM
    ===================== */
    #screen-confirm {
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 24px;
      padding: 32px;
    }
    .confirm-card {
      background: var(--surface);
      border: 1px solid rgba(34,197,94,0.3);
      border-radius: 20px;
      padding: 40px 48px;
      text-align: center;
      max-width: 520px;
      width: 100%;
    }
    .confirm-icon { font-size: 56px; margin-bottom: 16px; }
    .confirm-title { font-size: 24px; font-weight: 700; color: var(--green); margin-bottom: 8px; }
    .confirm-sub { font-size: 14px; color: var(--text-muted); margin-bottom: 24px; }
    .confirm-summary {
      background: var(--surface-2); border-radius: 12px;
      padding: 20px; text-align: left;
      font-size: 14px; color: var(--text-muted);
      line-height: 2; margin-bottom: 24px;
    }
    .confirm-summary .s-row { display: flex; justify-content: space-between; gap: 16px; }
    .confirm-summary .s-val { color: var(--text); font-weight: 600; }
    .confirm-divider { border: none; border-top: 1px solid var(--border); margin: 8px 0; }
    .countdown-ring {
      width: 80px; height: 80px; border-radius: 50%;
      border: 3px solid var(--green);
      display: flex; align-items: center; justify-content: center;
      font-size: 24px; font-weight: 800; color: var(--green);
      margin: 0 auto;
      box-shadow: 0 0 20px rgba(34,197,94,0.2);
    }
    .countdown-label { font-size: 13px; color: var(--text-muted); text-align: center; }
  </style>
</head>
<body>
<div id="app">

  <div id="topbar">
    <div class="brand">ECS · Post Admin</div>
    <div id="user-pill" class="user-pill" style="display:none;">
      <div class="dot"></div>
      <span id="user-pill-text">—</span>
    </div>
    <div class="system-badge">● SYSTEM ONLINE</div>
  </div>

  <!-- SCREEN 1: LOCK -->
  <div id="screen-lock" class="screen active">
    <div class="lock-ring"><span class="lock-icon">🔒</span></div>
    <div class="lock-title">System Locked</div>
    <div class="lock-sub">Tap your RFID card to continue</div>
    <div class="scan-indicator">
      <span class="rfid-icon">📡</span>
      Waiting for card scan...
    </div>
  </div>

  <!-- SCREEN 1.5: WELCOME -->
  <div id="screen-welcome" class="screen">
    <div class="welcome-glow"></div>
    <div class="welcome-avatar">👤</div>
    <div class="welcome-tag">✓ Identity Verified</div>
    <div class="welcome-name" id="welcome-name">—</div>
    <div class="welcome-role" id="welcome-role">—</div>
    <button class="welcome-continue-btn" onclick="showMenuScreen()">Continue →</button>
    <div class="welcome-time" id="welcome-time">—</div>
  </div>

  <!-- SCREEN 2: TASK SELECT -->
  <div id="screen-menu" class="screen">
    <div class="menu-greeting">
      <div class="welcome">What are you working on today,</div>
      <div class="name" id="menu-name">—</div>
      <div class="role" id="menu-role">—</div>
    </div>
    <div class="menu-prompt">Select your task route</div>
    <div class="task-grid">
      <div class="task-card receive" onclick="openTask('receive')">
        <div class="task-num">Step 01</div>
        <div class="task-emoji">📦</div>
        <div class="task-name">Receive Terminal</div>
        <div class="task-desc">Log incoming POS units, verify physical condition, and record chain of custody.</div>
      </div>
      <div class="task-card process" onclick="openTask('process')">
        <div class="task-num">Step 02</div>
        <div class="task-emoji">⚙️</div>
        <div class="task-name">Process / Check</div>
        <div class="task-desc">Inspect, test, and update terminal status. Document any findings or issues found.</div>
      </div>
      <div class="task-card release" onclick="openTask('release')">
        <div class="task-num">Step 03</div>
        <div class="task-emoji">📤</div>
        <div class="task-name">Release Terminal</div>
        <div class="task-desc">Confirm all items are complete and authorize the terminal for field deployment.</div>
      </div>
    </div>
    <div style="display:flex;gap:16px;align-items:center;">
      <div class="menu-footer" id="menu-time">—</div>
      <button class="logout-btn" onclick="resetToLock()">Log Out</button>
    </div>
  </div>

  <!-- SCREEN 3: TASK -->
  <div id="screen-task" class="screen">
    <div class="task-header">
      <div class="task-header-left">
        <div id="task-badge" class="task-badge"></div>
        <div>
          <div class="task-header-title" id="task-title">—</div>
          <div class="task-header-sub" id="task-staff">—</div>
        </div>
      </div>
      <button class="cancel-btn" onclick="resetToLock()">✕ Cancel</button>
    </div>
    <div class="task-body">
      <div class="checklist-panel">
        <div class="panel-label">
          Physical Item Checklist
          <span class="progress-text" id="check-progress">0 / 0</span>
        </div>
        <!-- AI scan result banner -->
        <div class="ai-scan-banner hidden" id="ai-banner">
          🤖 AI detected <span id="ai-detected-count">0</span> items automatically.
          Purple = AI confirmed · Yellow = manual confirmation needed.
        </div>
        <div class="checklist-items" id="checklist-items"></div>
        <div class="checklist-footer">
          <div class="progress-bar-wrap">
            <div class="progress-bar-fill" id="progress-fill" style="width:0%"></div>
          </div>
          <div class="footer-btns">
            <button class="ai-scan-btn" id="ai-scan-btn" onclick="triggerAiScan()">
              🤖 AI Scan
            </button>
            <button class="capture-btn disabled" id="capture-btn" onclick="triggerCapture()">
              <span id="capture-btn-icon">📸</span>
              <span id="capture-btn-text">Check all required items first</span>
            </button>
          </div>
        </div>
      </div>
      <div class="camera-panel">
        <div class="panel-label">
          Live Camera Feed
          <div style="display:flex;align-items:center;gap:6px;">
            <div class="cam-dot" id="cam-dot"></div>
            <span id="cam-status-label" style="font-size:12px;letter-spacing:0;color:var(--text-muted);">Connecting…</span>
          </div>
        </div>
        <div class="cam-wrap">
          <img id="cam-img" src="__CAM_URL__" alt="" onload="camOnline()" onerror="camOffline()">
          <div class="cam-offline" id="cam-offline">
            <div class="cam-offline-icon">📷</div>
            <div><strong>Camera Offline</strong></div>
            <div>Make sure the ESP32-CAM is powered<br>and connected to the same Wi-Fi network.</div>
          </div>
          <!-- AI scanning overlay -->
          <div class="ai-overlay" id="ai-overlay">
            <div class="ai-spinner"></div>
            <div class="ai-overlay-text">Gemini is scanning the frame…</div>
          </div>
        </div>
        <div class="cam-footer">
          Align the POS terminal and all items inside the frame before scanning or capturing.
        </div>
      </div>
    </div>
  </div>

  <!-- SCREEN 3.5: REMARKS -->
  <div id="screen-remarks" class="screen">
    <div class="remarks-card">
      <div class="remarks-title">📝 Add Remarks</div>
      <div class="remarks-sub">Optional — any notes, observations, or issues to record.</div>
      <div class="remarks-label">Remarks / Notes</div>
      <textarea
        class="remarks-textarea"
        id="remarks-input"
        placeholder="e.g. Minor scratch on back panel, SIM card slot appears loose, unit was restarted during testing…"
        maxlength="400"
        oninput="updateCharCount()"
      ></textarea>
      <div class="remarks-char"><span id="char-count">0</span> / 400</div>
      <div class="remarks-hint">
        💡 This note will be logged alongside the photo, staff identity, and checklist results.
        Leave blank if there's nothing to add.
      </div>
      <div class="remarks-actions">
        <button class="remarks-skip-btn" onclick="showConfirm()">Skip</button>
        <button class="remarks-confirm-btn" onclick="showConfirm()">Confirm & Proceed →</button>
      </div>
    </div>
  </div>

  <!-- SCREEN 4: CONFIRM -->
  <div id="screen-confirm" class="screen">
    <div class="confirm-card">
      <div class="confirm-icon">✅</div>
      <div class="confirm-title">Task Completed</div>
      <div class="confirm-sub">Image captured and chain of custody recorded.</div>
      <div class="confirm-summary" id="confirm-summary"></div>
      <div class="countdown-ring" id="countdown-ring">5</div>
      <div class="countdown-label">Locking system automatically…</div>
    </div>
  </div>

</div>

<script>
  const POLL_INTERVAL_MS = 500;
  let currentUser    = null;
  let currentTask    = null;
  let checkStates    = [];
  let checkSources   = []; // 'manual' | 'ai' | 'manual-confirm'
  let manualAnswers  = {}; // index -> true/false for SIM-card-style manual confirm
  let aiScanned      = false;
  let countdownTimer = null;

  // ==========================================
  // CHECKLIST DEFINITIONS
  // Items with manualConfirm:true require YES/NO from staff after AI scan
  // ==========================================
  const CHECKLISTS = {
    receive: [
      { label: 'POS Terminal (PAX A920)',    required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'POS Battery',               required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'LAN Cable',                 required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'POS Power Supply',          required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'Adaptor Plug',              required: false, aiDetectable: true,  manualConfirm: false },
      { label: 'POS SIM Card (Globe/Smart)',required: false, aiDetectable: false, manualConfirm: true  },
      { label: 'Screen Condition Intact',   required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'No Physical Damage',        required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'Serial Number Verified',    required: true,  aiDetectable: false, manualConfirm: false },
    ],
    process: [
      { label: 'Software Version Checked',  required: true,  aiDetectable: false, manualConfirm: false },
      { label: 'Network Connectivity Test', required: true,  aiDetectable: false, manualConfirm: false },
      { label: 'Payment Function Test',     required: true,  aiDetectable: false, manualConfirm: false },
      { label: 'Receipt Printer Test',      required: false, aiDetectable: true,  manualConfirm: false },
      { label: 'Screen Responsiveness',     required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'Battery Health Check',      required: true,  aiDetectable: false, manualConfirm: false },
      { label: 'Security Seal Intact',      required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'POS SIM Card Present',      required: false, aiDetectable: false, manualConfirm: true  },
    ],
    release: [
      { label: 'POS Terminal (PAX A920)',   required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'POS Battery',              required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'LAN Cable',               required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'POS Power Supply',         required: true,  aiDetectable: true,  manualConfirm: false },
      { label: 'Adaptor Plug',             required: false, aiDetectable: true,  manualConfirm: false },
      { label: 'POS SIM Card (Globe/Smart)',required: false, aiDetectable: false, manualConfirm: true  },
      { label: 'Packaging / Box',          required: false, aiDetectable: true,  manualConfirm: false },
      { label: 'Release Form Signed',      required: true,  aiDetectable: false, manualConfirm: false },
      { label: 'Terminal Config Confirmed',required: true,  aiDetectable: false, manualConfirm: false },
    ],
  };

  const TASK_LABELS = {
    receive: 'Receive Terminal',
    process: 'Process / Check',
    release: 'Release Terminal',
  };

  // ==========================================
  // SCREEN HELPERS
  // ==========================================
  function showScreen(id) {
    document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));
    document.getElementById(id).classList.add('active');
  }

  function setUser(name, role) {
    document.getElementById('user-pill-text').textContent = name + ' · ' + role;
    document.getElementById('user-pill').style.display = 'flex';
  }

  function clearUser() {
    document.getElementById('user-pill').style.display = 'none';
  }

  function resetToLock() {
    if (countdownTimer) { clearInterval(countdownTimer); countdownTimer = null; }
    currentUser = null; currentTask = null;
    aiScanned = false; manualAnswers = {};
    clearUser();
    showScreen('screen-lock');
  }

  // ==========================================
  // RFID POLLING
  // ==========================================
  function pollRFID() {
    fetch('/api/status')
      .then(r => r.json())
      .then(data => {
        if (data.scanned) {
          currentUser = { name: data.name, role: data.role, uid: data.uid };
          setUser(data.name, data.role);
          showWelcomeScreen();
        }
      })
      .catch(() => {});
  }
  setInterval(pollRFID, POLL_INTERVAL_MS);

  // ==========================================
  // WELCOME SCREEN (new!)
  // ==========================================
  function showWelcomeScreen() {
    document.getElementById('welcome-name').textContent = currentUser.name;
    document.getElementById('welcome-role').textContent = currentUser.role;
    const now = new Date();
    const timeStr = now.toLocaleString('en-PH', { dateStyle: 'medium', timeStyle: 'short' });
    document.getElementById('welcome-time').textContent = timeStr;
    showScreen('screen-welcome');
  }

  // ==========================================
  // MENU SCREEN
  // ==========================================
  function showMenuScreen() {
    document.getElementById('menu-name').textContent = currentUser.name;
    document.getElementById('menu-role').textContent = currentUser.role;
    document.getElementById('menu-time').textContent =
      new Date().toLocaleString('en-PH', { dateStyle: 'medium', timeStyle: 'short' });
    showScreen('screen-menu');
  }

  // ==========================================
  // TASK SCREEN
  // ==========================================
  function openTask(type) {
    currentTask = type;
    aiScanned   = false;
    manualAnswers = {};
    const items = CHECKLISTS[type];
    checkStates  = items.map(() => false);
    checkSources = items.map(() => 'manual');

    const badge = document.getElementById('task-badge');
    badge.className = 'task-badge ' + type;
    badge.textContent = TASK_LABELS[type].toUpperCase();
    document.getElementById('task-title').textContent = TASK_LABELS[type];
    document.getElementById('task-staff').textContent = currentUser.name + ' · ' + currentUser.role;

    document.getElementById('ai-banner').classList.add('hidden');
    document.getElementById('ai-scan-btn').disabled = false;

    renderChecklist(items);
    updateProgress(items);
    showScreen('screen-task');
  }

  function renderChecklist(items) {
    const container = document.getElementById('checklist-items');
    container.innerHTML = '';
    items.forEach((item, i) => {
      const isAI    = checkSources[i] === 'ai';
      const isMC    = item.manualConfirm && aiScanned;
      const checked = checkStates[i];

      let rowClass = 'check-row';
      if (isAI)   rowClass += ' ai-detected';
      else if (checked) rowClass += ' checked';
      if (isMC && !checked) rowClass += ' manual-needed';

      const row = document.createElement('div');
      row.className = rowClass;
      row.id = 'row-' + i;

      // Box
      let boxContent = '';
      if (checked) boxContent = isAI ? '🤖' : '✓';

      // AI tag
      let aiTag = '';
      if (aiScanned && item.aiDetectable && !item.manualConfirm) {
        aiTag = checked
          ? '<span class="check-ai-tag auto">AI DETECTED</span>'
          : '<span class="check-ai-tag manual">NOT DETECTED</span>';
      }
      if (aiScanned && item.manualConfirm) {
        aiTag = '<span class="check-ai-tag manual">MANUAL CONFIRM</span>';
      }

      // Manual confirm buttons (SIM cards)
      let mcHTML = '';
      if (aiScanned && item.manualConfirm) {
        const yesActive = manualAnswers[i] === true  ? 'active' : '';
        const noActive  = manualAnswers[i] === false ? 'active' : '';
        mcHTML = `
          <div class="manual-confirm-row">
            <button class="mc-btn yes ${yesActive}" onclick="setManualAnswer(${i}, true, event)">✓ Yes, present</button>
            <button class="mc-btn no ${noActive}"  onclick="setManualAnswer(${i}, false, event)">✕ Not present</button>
          </div>
        `;
      }

      row.innerHTML = `
        <div class="check-box" id="box-${i}">${boxContent}</div>
        <div class="check-label-col">
          <div class="check-label">${item.label}</div>
          ${aiTag}
          ${mcHTML}
        </div>
        ${item.required ? '<span class="check-required">REQUIRED</span>' : ''}
      `;

      // Only toggle on row click if NOT a manual-confirm item (those use buttons)
      if (!item.manualConfirm) {
        row.onclick = () => toggleCheck(i);
      }
      container.appendChild(row);
    });
  }

  function toggleCheck(i) {
    checkStates[i]  = !checkStates[i];
    checkSources[i] = 'manual';
    renderChecklist(CHECKLISTS[currentTask]);
    updateProgress(CHECKLISTS[currentTask]);
  }

  function setManualAnswer(i, val, event) {
    event.stopPropagation();
    manualAnswers[i] = val;
    checkStates[i]   = val;
    checkSources[i]  = 'manual';
    renderChecklist(CHECKLISTS[currentTask]);
    updateProgress(CHECKLISTS[currentTask]);
  }

  function updateProgress(items) {
    const total   = items.length;
    const checked = checkStates.filter(Boolean).length;

    // Required items that are still unchecked AND whose manual confirm (if any) hasn't been answered
    const pendingRequired = items.filter((it, i) => {
      if (!it.required) return false;
      if (it.manualConfirm) return manualAnswers[i] === undefined;
      return !checkStates[i];
    }).length;

    document.getElementById('check-progress').textContent = checked + ' / ' + total;
    document.getElementById('progress-fill').style.width = Math.round(checked / total * 100) + '%';

    const btn     = document.getElementById('capture-btn');
    const btnIcon = document.getElementById('capture-btn-icon');
    const btnText = document.getElementById('capture-btn-text');

    if (pendingRequired === 0) {
      btn.className = 'capture-btn ready';
      btnIcon.textContent = '📸';
      btnText.textContent = 'Capture & Proceed';
    } else {
      btn.className = 'capture-btn disabled';
      btnIcon.textContent = '⚠️';
      btnText.textContent = pendingRequired + ' required item' + (pendingRequired > 1 ? 's' : '') + ' pending';
    }
  }

  // ==========================================
  // AI SCAN (simulated Gemini call)
  // In production: call your backend which passes
  // the ESP32-CAM frame to Gemini Vision API.
  // ==========================================
  function triggerAiScan() {
    const btn = document.getElementById('ai-scan-btn');
    btn.disabled = true;
    btn.innerHTML = '⏳ Scanning…';

    document.getElementById('ai-overlay').classList.add('active');

    // Simulate ~2s Gemini processing time
    setTimeout(() => {
      document.getElementById('ai-overlay').classList.remove('active');
      applyAiResults();
    }, 2200);
  }

  function applyAiResults() {
    const items = CHECKLISTS[currentTask];
    aiScanned = true;
    let detected = 0;

    items.forEach((item, i) => {
      if (item.manualConfirm) return; // skip; handled by manual buttons
      if (item.aiDetectable) {
        // Simulate: ~85% detection rate for detectable items
        const found = Math.random() > 0.15;
        checkStates[i]  = found;
        checkSources[i] = 'ai';
        if (found) detected++;
      }
    });

    document.getElementById('ai-banner').classList.remove('hidden');
    document.getElementById('ai-detected-count').textContent = detected;

    document.getElementById('ai-scan-btn').innerHTML = '✓ Scanned';

    renderChecklist(items);
    updateProgress(items);
  }

  // ==========================================
  // CAPTURE → REMARKS
  // ==========================================
  function triggerCapture() {
    const btn = document.getElementById('capture-btn');
    if (!btn.classList.contains('ready')) return;

    btn.className = 'capture-btn processing';
    document.getElementById('capture-btn-icon').textContent = '⏳';
    document.getElementById('capture-btn-text').textContent = 'Capturing…';

    fetch('/api/capture', { method: 'POST' }).catch(() => {});
    setTimeout(() => {
      document.getElementById('remarks-input').value = '';
      updateCharCount();
      showScreen('screen-remarks');
    }, 1200);
  }

  // ==========================================
  // REMARKS
  // ==========================================
  function updateCharCount() {
    const val = document.getElementById('remarks-input').value;
    document.getElementById('char-count').textContent = val.length;
  }

  // ==========================================
  // CONFIRM SCREEN
  // ==========================================
  function showConfirm() {
    const items   = CHECKLISTS[currentTask];
    const checked = checkStates.filter(Boolean).length;
    const remarks = (document.getElementById('remarks-input').value || '').trim();
    const now     = new Date();
    const aiItems = checkSources.filter(s => s === 'ai').length;

    document.getElementById('confirm-summary').innerHTML = `
      <div class="s-row"><span>Staff</span><span class="s-val">${currentUser.name}</span></div>
      <div class="s-row"><span>Role</span><span class="s-val">${currentUser.role}</span></div>
      <hr class="confirm-divider">
      <div class="s-row"><span>Task</span><span class="s-val">${TASK_LABELS[currentTask]}</span></div>
      <div class="s-row"><span>Items Checked</span><span class="s-val">${checked} / ${items.length}</span></div>
      <div class="s-row"><span>AI Auto-Detected</span><span class="s-val" style="color:var(--purple)">${aiItems} item${aiItems !== 1 ? 's' : ''}</span></div>
      <div class="s-row"><span>Photo Captured</span><span class="s-val" style="color:var(--green)">✓ Yes</span></div>
      <hr class="confirm-divider">
      ${remarks ? `<div class="s-row"><span>Remarks</span><span class="s-val" style="font-size:13px;text-align:right;max-width:260px;">${remarks}</span></div><hr class="confirm-divider">` : ''}
      <div class="s-row"><span>Date / Time</span><span class="s-val">${now.toLocaleString('en-PH', { dateStyle: 'medium', timeStyle: 'short' })}</span></div>
    `;

    showScreen('screen-confirm');
    startCountdown(5);
  }

  function startCountdown(sec) {
    const ring = document.getElementById('countdown-ring');
    ring.textContent = sec;
    if (countdownTimer) clearInterval(countdownTimer);
    countdownTimer = setInterval(() => {
      sec--;
      ring.textContent = sec;
      if (sec <= 0) {
        clearInterval(countdownTimer);
        countdownTimer = null;
        resetToLock();
      }
    }, 1000);
  }

  // ==========================================
  // CAMERA
  // ==========================================
  function camOnline() {
    document.getElementById('cam-dot').classList.add('live');
    document.getElementById('cam-status-label').textContent = 'Live';
    document.getElementById('cam-offline').style.display = 'none';
  }

  function camOffline() {
    document.getElementById('cam-dot').classList.remove('live');
    document.getElementById('cam-status-label').textContent = 'Offline';
    document.getElementById('cam-img').style.display = 'none';
    document.getElementById('cam-offline').style.display = 'flex';
  }
</script>
</body>
</html>
)rawliteral";

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n[ECS Post Admin v3] Booting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n✅ Wi-Fi Connected! IP: " + WiFi.localIP().toString());

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("✅ RFID Reader Online.");

  // Routes
  server.on("/", HTTP_GET, []() {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    const char* ptr = INDEX_HTML;
    size_t remaining = strlen_P(INDEX_HTML);
    char chunk[1024];

    while (remaining > 0) {
      size_t toSend = min(remaining, sizeof(chunk) - 1);
      memcpy_P(chunk, ptr, toSend);
      chunk[toSend] = '\0';

      String s = String(chunk);
      s.replace("__CAM_URL__", ESP32_CAM_STREAM_URL);
      server.sendContent(s);

      ptr       += toSend;
      remaining -= toSend;
    }

    server.sendContent("");
  });

  server.on("/api/status", HTTP_GET, []() {
    StaticJsonDocument<256> doc;
    doc["scanned"] = newScanAvailable;
    if (newScanAvailable) {
      doc["uid"]  = lastScannedUID;
      doc["name"] = lastScannedName;
      doc["role"] = lastScannedRole;
      newScanAvailable = false;
    }
    String json; serializeJson(doc, json);
    server.send(200, "application/json", json);
  });

  server.on("/api/capture", HTTP_POST, []() {
    Serial.println(">>> CAPTURE COMMAND from tablet");
    server.send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("✅ Web Server running on port 80");
}

// ==========================================
// LOOP
// ==========================================
void loop() {
  server.handleClient();

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    Serial.println("RFID SCANNED: " + uid);

    String name = "Unknown Card";
    String role = "Unregistered";
    for (int i = 0; i < STAFF_COUNT; i++) {
      if (uid == String(staffDB[i].uid)) {
        name = staffDB[i].name;
        role = staffDB[i].role;
        break;
      }
    }

    lastScannedUID   = uid;
    lastScannedName  = name;
    lastScannedRole  = role;
    newScanAvailable = true;

    Serial.println("Staff: " + name + " (" + role + ")");

    mfrc522.PICC_HaltA();
    delay(1000);
  }
}
