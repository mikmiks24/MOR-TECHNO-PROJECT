#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>ECS Post Admin</title>
<link href="https://fonts.googleapis.com/css2?family=Source+Sans+3:wght@400;500;600;700&display=swap" rel="stylesheet">
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
:root{
  --navy:#04080f;--navy-2:#071525;--navy-3:#0c1a2e;
  --surface:rgba(10,20,38,.92);--surface-2:rgba(14,28,50,.95);
  --border:rgba(56,120,200,.12);--border-hi:rgba(96,165,250,.22);
  --text:#e8f0fa;--muted:#8ba3be;--muted-2:#5a7390;
  --blue-900:#1e40af;--blue-600:#3b82f6;--blue-400:#38bdf8;--cyan:#22d3ee;
  --accent:#3b82f6;--accent-2:#2563eb;--accent-glow:rgba(59,130,246,.35);
  --accent-soft:rgba(59,130,246,.14);--cyan-soft:rgba(34,211,238,.12);
  --grad:linear-gradient(135deg,var(--blue-900),var(--blue-600),var(--blue-400),var(--cyan));
  --grad-h:linear-gradient(90deg,var(--blue-900),var(--blue-600),var(--blue-400),var(--cyan));
  --danger:#ef4444;--danger-soft:rgba(239,68,68,.12);
  --warn:#f59e0b;--warn-soft:rgba(245,158,11,.1);
  --radius:12px;--radius-lg:16px;
  --font:'Source Sans 3',system-ui,sans-serif;
  --ease:cubic-bezier(.25,.46,.45,.94);
  --touch:48px;
}
html,body{height:100%;font-family:var(--font);background:var(--navy);color:var(--text);overflow:hidden;-webkit-font-smoothing:antialiased;-webkit-tap-highlight-color:transparent}
body::before,body::after{content:'';position:fixed;border-radius:50%;filter:blur(100px);z-index:0;pointer-events:none}
body::before{width:50vw;height:50vw;top:-15%;left:-10%;background:rgba(30,64,175,.18)}
body::after{width:40vw;height:40vw;bottom:-18%;right:-8%;background:rgba(34,211,238,.08)}
#app{height:100vh;display:flex;flex-direction:column;position:relative;z-index:1}
header{display:flex;align-items:center;justify-content:space-between;padding:14px 28px;background:rgba(4,8,15,.88);backdrop-filter:blur(14px);border-bottom:1px solid var(--border);flex-shrink:0;gap:20px}
.hdr-left{display:flex;align-items:center;gap:20px;min-width:0}
.brand{display:flex;align-items:center;gap:11px;font-size:14px;font-weight:700;letter-spacing:-.01em;white-space:nowrap}
.brand-mark{width:34px;height:34px;border-radius:8px;background:var(--grad);display:flex;align-items:center;justify-content:center;font-size:10px;font-weight:700;color:#fff}
.station{font-size:13px;font-weight:500;color:var(--muted);padding-left:16px;border-left:1px solid var(--border)}
.hdr-right{display:flex;align-items:center;gap:12px;flex-shrink:0}
.user-chip{display:none;align-items:center;gap:10px;padding:8px 16px 8px 12px;background:var(--surface-2);border:1px solid var(--border-hi);border-radius:999px;font-size:13px;font-weight:600}
.user-chip .dot{width:8px;height:8px;border-radius:50%;background:var(--cyan);box-shadow:0 0 10px rgba(34,211,238,.5)}
.status-pill{font-size:10px;font-weight:700;letter-spacing:.07em;text-transform:uppercase;color:var(--blue-400);background:var(--accent-soft);border:1px solid rgba(59,130,246,.28);padding:7px 14px;border-radius:999px}
.flow-bar{display:flex;align-items:center;justify-content:center;gap:0;padding:10px 28px 12px;background:rgba(7,21,37,.6);border-bottom:1px solid var(--border);flex-shrink:0}
.flow-bar.hidden{display:none}
.flow-step{display:flex;align-items:center;gap:8px;font-size:11px;font-weight:600;text-transform:uppercase;letter-spacing:.06em;color:var(--muted-2);padding:0 4px}
.flow-step .step-dot{width:22px;height:22px;border-radius:50%;border:2px solid var(--border-hi);background:rgba(4,8,15,.6);flex-shrink:0;position:relative;transition:border-color .25s,background .25s}
.flow-step.active{color:var(--blue-400)}
.flow-step.active .step-dot{border-color:var(--blue-600);background:var(--accent-soft);box-shadow:0 0 12px var(--accent-glow)}
.flow-step.done{color:var(--muted)}
.flow-step.done .step-dot{border-color:var(--blue-600);background:var(--grad)}
.flow-step.done .step-dot::after{content:'';position:absolute;left:50%;top:50%;width:5px;height:9px;margin:-6px 0 0 -3px;border:solid #fff;border-width:0 2px 2px 0;transform:rotate(45deg)}
.flow-connector{width:36px;height:2px;background:var(--border);margin:0 6px;flex-shrink:0;transition:background .25s}
.flow-connector.done{background:var(--grad-h)}
.screen{display:none;flex:1;overflow:hidden;position:relative}
.screen.active{display:flex;animation:enter .35s var(--ease)}
@keyframes enter{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:none}}
#screen-lock{padding:0}
.lock-split{display:grid;grid-template-columns:1fr 1fr;width:100%;height:100%;min-height:0}
.lock-brand{display:flex;flex-direction:column;justify-content:center;padding:48px 56px;background:linear-gradient(160deg,var(--navy-2),var(--navy));border-right:1px solid var(--border)}
.lock-brand .brand-large{font-size:13px;font-weight:700;letter-spacing:.12em;text-transform:uppercase;color:var(--blue-400);margin-bottom:16px}
.lock-brand h1{font-size:38px;font-weight:700;letter-spacing:-.03em;line-height:1.15;margin-bottom:12px}
.lock-brand .sub{font-size:15px;color:var(--muted);line-height:1.6;max-width:340px;margin-bottom:48px}
.lock-time{font-size:56px;font-weight:700;letter-spacing:-.04em;line-height:1;font-variant-numeric:tabular-nums;background:var(--grad);-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text}
.lock-date{font-size:16px;color:var(--muted);margin-top:10px;font-weight:500}
.lock-prompt{display:flex;flex-direction:column;align-items:center;justify-content:center;padding:48px;background:linear-gradient(180deg,rgba(7,21,37,.5),rgba(4,8,15,.8))}
.card-prompt{position:relative;width:100%;max-width:380px;padding:48px 40px;text-align:center;background:var(--surface);border-radius:var(--radius-lg)}
.card-prompt::before{content:'';position:absolute;inset:-2px;border-radius:calc(var(--radius-lg) + 2px);background:var(--grad);z-index:-1;opacity:.55;animation:border-glow 4s ease-in-out infinite}
.card-prompt::after{content:'';position:absolute;inset:0;border-radius:var(--radius-lg);background:var(--surface);z-index:-1}
@keyframes border-glow{0%,100%{opacity:.4}50%{opacity:.75}}
.card-icon{width:72px;height:72px;margin:0 auto 28px;border-radius:16px;background:rgba(30,64,175,.2);border:1px solid var(--border-hi);display:flex;align-items:center;justify-content:center;color:var(--blue-400)}
.card-prompt h2{font-size:24px;font-weight:700;margin-bottom:10px}
.card-prompt p{font-size:15px;color:var(--muted);line-height:1.55;margin-bottom:32px}
.scan-hint{display:inline-flex;align-items:center;gap:12px;padding:14px 24px;min-height:var(--touch);background:rgba(4,8,15,.5);border:1px solid var(--border-hi);border-radius:999px;font-size:14px;font-weight:500;color:var(--muted)}
.pulse{width:9px;height:9px;border-radius:50%;background:var(--cyan);box-shadow:0 0 12px rgba(34,211,238,.5);animation:pulse 2.5s ease-in-out infinite}
@keyframes pulse{0%,100%{opacity:.45}50%{opacity:1}}
#screen-welcome{align-items:center;justify-content:center;padding:32px 40px}
.welcome-card{display:flex;align-items:center;gap:40px;width:100%;max-width:820px;background:linear-gradient(145deg,var(--surface-2),var(--surface));border:1px solid var(--border-hi);border-radius:var(--radius-lg);padding:44px 48px;box-shadow:0 24px 56px rgba(0,0,0,.35)}
.welcome-left{flex-shrink:0;text-align:center}
.welcome-avatar-wrap{width:110px;height:110px;border-radius:50%;padding:4px;background:var(--grad);margin:0 auto 16px;overflow:hidden}
.welcome-label{font-size:11px;font-weight:700;text-transform:uppercase;letter-spacing:.12em;color:var(--blue-400);background:var(--accent-soft);padding:6px 12px;border-radius:999px;}
.welcome-right{flex:1;min-width:0}
.welcome-name{font-size:34px;font-weight:700;letter-spacing:-.03em;margin-bottom:6px;line-height:1.15}
.welcome-role{font-size:16px;color:var(--muted);margin-bottom:12px;font-weight:500}
.redirect-msg{display:flex;align-items:center;gap:10px;font-size:14px;color:var(--blue-400);font-weight:600;margin-top:24px;}
.spinner{width:18px;height:18px;border-radius:50%;border:2px solid var(--border-hi);border-top-color:var(--blue-400);animation:spin .8s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}
#screen-menu{flex-direction:column;align-items:center;justify-content:center;padding:32px 36px;gap:32px}
.menu-head{text-align:center}
.menu-head p{font-size:14px;color:var(--muted);margin-bottom:4px}
.menu-head h2{font-size:30px;font-weight:700;letter-spacing:-.03em}
.task-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:18px;width:100%;max-width:960px}
.task-card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius-lg);padding:28px 24px 30px;cursor:pointer;transition:.25s;text-align:left;position:relative;overflow:hidden;min-height:220px;box-shadow:0 8px 32px rgba(0,0,0,.22);touch-action:manipulation}
.task-card::before{content:'';position:absolute;top:0;left:0;right:0;height:3px}
.task-card.receive::before{background:linear-gradient(90deg,#1e3a8a,#3b82f6)}
.task-card.process::before{background:linear-gradient(90deg,#1e40af,#38bdf8)}
.task-card.release::before{background:linear-gradient(90deg,#2563eb,#22d3ee)}
.task-card:hover{border-color:var(--border-hi);box-shadow:0 16px 40px rgba(0,0,0,.3);transform:translateY(-2px)}
.task-icon{width:48px;height:48px;border-radius:12px;margin-bottom:18px;display:flex;align-items:center;justify-content:center}
.task-card.receive .task-icon{background:rgba(30,64,175,.2);color:#60a5fa}
.task-card.process .task-icon{background:rgba(37,99,235,.18);color:#38bdf8}
.task-card.release .task-icon{background:rgba(34,211,238,.12);color:#22d3ee}
.task-step{font-size:11px;font-weight:700;text-transform:uppercase;letter-spacing:.1em;color:var(--blue-400);margin-bottom:8px}
.task-card h3{font-size:17px;font-weight:700;margin-bottom:8px}
.task-card p{font-size:13px;color:var(--muted);line-height:1.6}
.menu-foot{display:flex;align-items:center;gap:20px}
.menu-time{font-size:13px;color:var(--muted-2)}
#screen-task{flex-direction:column;padding:18px 24px 22px;gap:14px}
.task-bar{display:flex;align-items:center;justify-content:space-between;padding:16px 22px;background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);flex-shrink:0}
.task-bar-left{display:flex;align-items:center;gap:14px}
.tag{font-size:10px;font-weight:700;text-transform:uppercase;letter-spacing:.07em;padding:6px 12px;border-radius:6px}
.tag.receive{background:rgba(30,64,175,.2);color:#60a5fa;border:1px solid rgba(59,130,246,.25)}
.tag.process{background:rgba(37,99,235,.18);color:#38bdf8;border:1px solid rgba(56,189,248,.25)}
.tag.release{background:rgba(34,211,238,.1);color:#22d3ee;border:1px solid rgba(34,211,238,.25)}
.task-bar h2{font-size:17px;font-weight:700}
.task-bar .sub{font-size:13px;color:var(--muted);margin-top:2px}
.task-body{display:grid;grid-template-columns:1fr 1fr;flex:1;overflow:hidden;gap:14px;min-height:0}
.panel{display:flex;flex-direction:column;overflow:hidden;background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);box-shadow:0 8px 28px rgba(0,0,0,.2)}
.panel-hd{padding:14px 20px;border-bottom:1px solid var(--border);display:flex;justify-content:space-between;align-items:center;flex-shrink:0;font-size:11px;font-weight:700;text-transform:uppercase;letter-spacing:.07em;color:var(--muted)}
.panel-hd .count{font-size:13px;color:var(--text);text-transform:none;letter-spacing:0;font-weight:700;background:rgba(59,130,246,.1);padding:4px 12px;border-radius:999px}
.checklist{flex:1;overflow-y:auto;padding:12px;display:flex;flex-direction:column;gap:10px}
.check-row{display:flex;align-items:flex-start;gap:14px;padding:16px 18px;min-height:var(--touch);background:rgba(4,8,15,.45);border:1px solid var(--border);border-radius:var(--radius);cursor:pointer;transition:.2s;box-shadow:0 2px 8px rgba(0,0,0,.12);touch-action:manipulation}
.check-row.done{border-color:rgba(59,130,246,.28);background:rgba(30,64,175,.1)}
.check-row.auto{border-color:rgba(34,211,238,.3);background:var(--cyan-soft)}
.check-row.needs-input{border-color:rgba(239,68,68,.3);background:rgba(239,68,68,.08);box-shadow:inset 0 0 10px rgba(239,68,68,.1)}
.cb{width:24px;height:24px;border:2px solid var(--border-hi);border-radius:7px;flex-shrink:0;display:flex;align-items:center;justify-content:center;margin-top:1px}
.check-row.done .cb,.check-row.auto .cb{background:var(--grad);border-color:transparent}
.check-row.done .cb::after,.check-row.auto .cb::after{content:'';width:5px;height:9px;border:solid #fff;border-width:0 2px 2px 0;transform:rotate(45deg);margin-top:-2px}
.check-body{flex:1;min-width:0}
.check-label{font-size:14px;font-weight:600;line-height:1.45}
.check-row.done .check-label{color:var(--muted);text-decoration:line-through}
.check-meta{display:flex;gap:6px;margin-top:6px;flex-wrap:wrap}
.badge{font-size:9px;font-weight:700;padding:3px 8px;border-radius:5px;text-transform:uppercase;letter-spacing:.05em}
.badge.req{background:var(--warn-soft);color:#fcd34d}
.badge.auto{background:rgba(34,211,238,.15);color:#67e8f9}
.badge.manual{background:rgba(59,130,246,.15);color:#93c5fd}
.badge.miss{background:var(--danger-soft);color:#fca5a5}
.panel-ft{padding:16px 18px;border-top:1px solid var(--border);flex-shrink:0;background:rgba(4,8,15,.3)}
.ft-btns{display:flex;gap:10px}
.btn{min-height:var(--touch);padding:12px 20px;border-radius:10px;font-size:14px;font-weight:600;cursor:pointer;border:none;font-family:var(--font);transition:.2s;touch-action:manipulation}
.btn-ghost{background:rgba(255,255,255,.04);border:1px solid var(--border-hi);color:var(--text)}
.btn-ghost:disabled{opacity:.4;cursor:not-allowed}
.btn-primary{flex:1;color:#fff;background:var(--grad);box-shadow:0 6px 20px var(--accent-glow)}
.btn-primary:disabled{background:rgba(255,255,255,.06);color:var(--muted-2);box-shadow:none;cursor:not-allowed}
.btn-danger-ghost{background:transparent;border:1px solid rgba(239,68,68,.25);color:#fca5a5;font-size:13px;padding:10px 18px;min-height:var(--touch)}
.cam-wrap{flex:1;background:#000;position:relative;overflow:hidden;margin:12px;border-radius:var(--radius);border:2px solid rgba(59,130,246,.35);min-height:0;box-shadow:0 0 0 1px rgba(34,211,238,.1),inset 0 0 24px rgba(30,64,175,.15)}
.cam-wrap img{width:100%;height:100%;object-fit:cover}
.cam-off{position:absolute;inset:0;display:none;flex-direction:column;align-items:center;justify-content:center;gap:12px;background:linear-gradient(160deg,var(--navy-3),var(--navy));color:var(--muted);font-size:14px;text-align:center;padding:28px}
.cam-live{display:flex;align-items:center;gap:8px;font-size:12px;font-weight:600;text-transform:none;color:var(--muted)}
.live-dot{width:8px;height:8px;border-radius:50%;background:var(--danger)}
.live-dot.on{background:var(--cyan);box-shadow:0 0 10px rgba(34,211,238,.5);animation:pulse 2.5s ease-in-out infinite}
.overlay{position:absolute;inset:0;background:rgba(4,8,15,.85);backdrop-filter:blur(4px);display:none;align-items:center;justify-content:center;flex-direction:column;gap:14px;z-index:10}
.overlay.on{display:flex}
#incident-report-block{display:none;padding:16px;background:rgba(239,68,68,.1);border-top:1px solid rgba(239,68,68,.3);flex-shrink:0}
.incident-title{color:#fca5a5;font-weight:bold;margin-bottom:8px;font-size:12px;text-transform:uppercase;letter-spacing:0.05em;display:flex;align-items:center;gap:6px}
textarea.incident{height:60px;width:100%;background:rgba(0,0,0,.2);border:1px solid rgba(239,68,68,.4);color:#fca5a5;border-radius:8px;padding:10px;font-size:13px;resize:none;font-family:var(--font)}
textarea.incident::placeholder{color:rgba(252,165,165,.5)}
textarea.incident:focus{outline:none;border-color:#ef4444;box-shadow:0 0 0 2px rgba(239,68,68,.2)}
#screen-remarks,#screen-confirm{align-items:center;justify-content:center;padding:32px}
.card{background:linear-gradient(145deg,var(--surface-2),var(--surface));border:1px solid var(--border-hi);border-radius:var(--radius-lg);padding:40px 44px;box-shadow:0 24px 56px rgba(0,0,0,.35);width:100%;max-width:540px}
.card h2{font-size:24px;font-weight:700;margin-bottom:8px}
.card .lead{font-size:15px;color:var(--muted);margin-bottom:28px;line-height:1.6}
.field-label{font-size:11px;font-weight:700;color:var(--muted);margin-bottom:10px;text-transform:uppercase;letter-spacing:.07em}
textarea.std-notes{width:100%;height:140px;padding:16px 18px;border:1px solid var(--border-hi);border-radius:var(--radius);font-family:var(--font);font-size:15px;resize:none;outline:none;background:rgba(4,8,15,.5);color:var(--text)}
.serial-field{width:100%;padding:16px 18px;border:1px solid var(--border-hi);border-radius:var(--radius);font-family:var(--font);font-size:15px;outline:none;background:rgba(4,8,15,.5);color:var(--text);margin-bottom:18px}
.serial-field:focus,textarea.std-notes:focus{border-color:rgba(59,130,246,.5);box-shadow:0 0 0 3px var(--accent-soft)}
.char{font-size:12px;color:var(--muted-2);text-align:right;margin-top:8px}
.actions{display:flex;gap:12px;margin-top:28px}
.actions .btn-ghost{flex:1}
.actions .btn-primary{flex:2}
.confirm-card{text-align:center;max-width:500px}
.confirm-icon{width:64px;height:64px;border-radius:50%;margin:0 auto 24px;background:var(--accent-soft);border:2px solid rgba(59,130,246,.35);display:flex;align-items:center;justify-content:center;box-shadow:0 0 32px var(--accent-glow)}
.confirm-icon svg{color:var(--blue-400)}
.confirm-card h2{font-size:26px;font-weight:700;color:var(--blue-400);margin-bottom:10px}
.summary{text-align:left;background:rgba(4,8,15,.45);border:1px solid var(--border);border-radius:var(--radius);padding:22px 24px;font-size:14px;margin-bottom:32px}
.summary .row{display:flex;justify-content:space-between;gap:20px;padding:7px 0}
.summary .row span:first-child{color:var(--muted)}
.summary .row span:last-child{font-weight:600;text-align:right}
.summary hr{border:none;border-top:1px solid var(--border);margin:10px 0}
.countdown{font-size:14px;color:var(--muted);font-weight:500}
.countdown-ring{width:72px;height:72px;border-radius:50%;margin:0 auto 12px;border:3px solid var(--border);display:flex;align-items:center;justify-content:center;font-size:28px;font-weight:700;color:var(--blue-400);box-shadow:0 0 24px var(--accent-glow);background:var(--accent-soft)}
</style>
</head>
<body>
<div id="app">
  <header>
    <div class="hdr-left">
      <div class="brand"><div class="brand-mark">ECS</div>Post Admin</div>
      <div class="station">Post Admin · Bay 1</div>
    </div>
    <div class="hdr-right">
      <div class="user-chip" id="user-chip"><span class="dot"></span><span id="user-text"></span></div>
      <div class="status-pill">Online</div>
    </div>
  </header>
  <div id="flow-bar" class="flow-bar hidden">
    <div class="flow-step" data-step="0"><span class="step-dot"></span><span class="label">Sign in</span></div>
    <div class="flow-connector"></div>
    <div class="flow-step" data-step="1"><span class="step-dot"></span><span class="label">Route</span></div>
    <div class="flow-connector"></div>
    <div class="flow-step" data-step="2"><span class="step-dot"></span><span class="label">Checklist</span></div>
    <div class="flow-connector"></div>
    <div class="flow-step" data-step="3"><span class="step-dot"></span><span class="label">Done</span></div>
  </div>
  
  <div id="screen-lock" class="screen active">
    <div class="lock-split">
      <div class="lock-brand">
        <div class="brand-large">Equitable Computer Services</div>
        <h1>POS Terminal Servicing</h1>
        <p class="sub">Workstation for intake, inspection, and release of field terminals. Authorized staff only.</p>
        <div class="lock-time" id="lock-time">--:--</div>
        <div class="lock-date" id="lock-date">—</div>
      </div>
      <div class="lock-prompt">
        <div class="card-prompt">
          <div class="card-icon"><svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.75"><rect x="2" y="5" width="20" height="14" rx="2"/><path d="M2 10h20"/></svg></div>
          <h2>Sign in required</h2>
          <p>Hold card to reader</p>
          <div class="scan-hint"><span class="pulse"></span> Listening for card</div>
          <div style="margin-top: 24px; border-top: 1px solid var(--border); padding-top: 20px;">
            <div style="font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: .06em; color: var(--muted); margin-bottom: 12px; text-align: left;">Local Test Mock Sign In</div>
            <select id="mock-employee-select" style="width: 100%; height: 40px; background: rgba(4,8,15,.6); color: var(--text); border: 1px solid var(--border-hi); border-radius: var(--radius); padding: 0 12px; font-size: 14px; cursor: pointer; margin-bottom: 12px; font-family: var(--font); outline: none;">
              <option value="" style="background:#071525; color:var(--text);">-- Select Staff Member --</option>
              <option value="RFID-ANA-001" style="background:#071525; color:var(--text);">Ana Reyes (Staff)</option>
              <option value="RFID-JUAN-002" style="background:#071525; color:var(--text);">Juan dela Cruz (Staff)</option>
              <option value="RFID-JOSE-003" style="background:#071525; color:var(--text);">Jose Garcia (Staff)</option>
              <option value="RFID-JOY-004" style="background:#071525; color:var(--text);">Joy Retuba (Staff)</option>
              <option value="RFID-JOSHUA-005" style="background:#071525; color:var(--text);">Joshua Zaide (Staff)</option>
              <option value="RFID-GERSON-006" style="background:#071525; color:var(--text);">Gerson Barrientos (Staff)</option>
              <option value="ADMIN-RFID" style="background:#071525; color:var(--text);">Admin User (Superuser)</option>
            </select>
            <button class="btn btn-primary" onclick="simulateRfidTap()" style="width: 100%; min-height: 40px; padding: 8px 16px; font-size: 13px;">Simulate RFID Tap</button>
          </div>
        </div>
      </div>
    </div>
  </div>

  <div id="screen-welcome" class="screen">
    <div class="welcome-card">
      <div class="welcome-left">
        <div class="welcome-avatar-wrap">
            <img id="welcome-photo" src="" alt="Staff Photo" style="width:100%;height:100%;border-radius:50%;object-fit:cover;">
        </div>
        <div class="welcome-label" id="welcome-emp-id">EMP-ID</div>
      </div>
      <div class="welcome-right">
        <div class="welcome-name" id="welcome-name"></div>
        <div class="welcome-role" id="welcome-role"></div>
        <div class="redirect-msg"><div class="spinner"></div> Authenticating and entering workspace...</div>
      </div>
    </div>
  </div>

  <div id="screen-menu" class="screen">
    <div class="menu-head"><p>Welcome back,</p><h2 id="menu-name"></h2></div>
    <div class="task-grid">
      <div class="task-card receive" onclick="openTask('receive')">
        <div class="task-icon"><svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/></svg></div>
        <div class="task-step">Route 01</div><h3>Receive terminal</h3><p>Log incoming units and verify physical condition on intake.</p>
      </div>
      <div class="task-card process" onclick="openTask('process')">
        <div class="task-icon"><svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M12 1v4M12 19v4M4.22 4.22l2.83 2.83M16.95 16.95l2.83 2.83M1 12h4M19 12h4"/></svg></div>
        <div class="task-step">Route 02</div><h3>Process and check</h3><p>Inspect, test, and document technical findings.</p>
      </div>
      <div class="task-card release" onclick="openTask('release')">
        <div class="task-icon"><svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 2L11 13M22 2l-7 20-4-9-9-4 20-7z"/></svg></div>
        <div class="task-step">Route 03</div><h3>Release terminal</h3><p>Confirm completeness before field deployment.</p>
      </div>
    </div>
    <div class="menu-foot"><span class="menu-time" id="menu-time"></span><button class="btn btn-danger-ghost" onclick="resetToLock()">Sign out</button></div>
  </div>

  <div id="screen-task" class="screen">
    <div class="task-bar">
      <div class="task-bar-left"><span class="tag" id="task-tag"></span><div><h2 id="task-title"></h2><div class="sub" id="task-staff"></div></div></div>
      <button class="btn btn-danger-ghost" onclick="resetToLock()">Cancel Task</button>
    </div>
    <div class="task-body">
      
      <!-- Left Panel -->
      <div class="panel">
        <div class="panel-hd">
          <span id="left-panel-title">Detected Items (Scanning Phase)</span>
          <span class="count" id="prog-text">0</span>
        </div>
        
        <!-- View 1: Scanning List -->
        <div class="checklist" id="scanned-list-view">
           <div style="padding:20px;text-align:center;color:var(--muted);font-size:14px;">No items scanned yet.<br><br>Place an item in front of the camera and tap <strong>Scan Current Item</strong>.</div>
        </div>

        <!-- View 2: Verification Checklist -->
        <div class="checklist" id="checklist-view" style="display:none;"></div>

        <!-- Warning Block -->
        <div id="incident-report-block">
          <div class="incident-title">⚠️ Incident Report Required</div>
          <div style="font-size:13px;color:var(--muted);margin-bottom:12px;">Missing required materials detected. Please explain below.</div>
          <textarea id="incident-notes" class="incident" placeholder="Explanation for missing items..."></textarea>
        </div>

        <div class="panel-ft">
          <div class="ft-btns" id="scanning-btns">
            <button class="btn btn-primary" onclick="verifyChecklist()">Finish Scanning & Verify Checklist</button>
          </div>
          <div class="ft-btns" id="verify-btns" style="display:none;">
            <button class="btn btn-ghost" onclick="backToScan()">← Back to Camera</button>
            <button class="btn btn-primary" id="cap-btn" onclick="capture()">Submit & Finalize Task</button>
          </div>
        </div>
      </div>

      <!-- Right Panel -->
      <div class="panel">
        <div class="panel-hd" style="display:flex; justify-content:space-between; align-items:center;">
          <span>Camera</span>
          <div style="display:flex; align-items:center; gap:10px;">
            <select id="camera-source-select" onchange="toggleCameraSource()" style="background:rgba(4,8,15,.6); color:var(--text); border:1px solid var(--border-hi); border-radius:4px; padding:4px 8px; font-size:12px; cursor:pointer; font-family:var(--font); outline:none;">
              <option value="esp32" style="background:#071525; color:var(--text);">ESP32 Camera</option>
              <option value="pc" style="background:#071525; color:var(--text);">PC Webcam</option>
            </select>
            <span class="cam-live"><span class="live-dot" id="cam-dot"></span><span id="cam-label">Connecting</span></span>
          </div>
        </div>
        <div id="scanning-phase-banner" style="background:#071525; border-bottom:1px solid var(--border); padding:8px 16px; font-size:12px; font-weight:600; color:#22d3ee; display:flex; justify-content:space-between; align-items:center;">
          <span id="phase-title">Phase 1: Scan Terminal Serial</span>
          <span id="phase-status-badge" style="background:rgba(34,211,238,0.1); padding:2px 6px; border-radius:4px; font-size:10px;">Required</span>
        </div>
        <div class="cam-wrap" style="position:relative; background:#02050a; display:flex; align-items:center; justify-content:center; min-height:240px; overflow:hidden;">
          <img id="cam" src="__CAM_URL__" alt="" onload="camOk()" onerror="camFail()" crossorigin="anonymous" style="width:100%; height:100%; object-fit:cover; display:block;">
          <video id="pc-cam" autoplay playsinline style="display:none; width:100%; height:100%; object-fit:cover;"></video>
          <img id="scan-preview" style="display:none; width:100%; height:100%; object-fit:cover;">
          <canvas id="cam-overlay" style="position:absolute; top:0; left:0; width:100%; height:100%; pointer-events:none; z-index:5;"></canvas>
          <div class="cam-off" id="cam-off"><p><strong>Camera unavailable</strong></p><p>Check ESP32-CAM power and network.</p></div>
          <div class="overlay" id="overlay"><div class="spinner"></div><p id="overlay-text">Analyzing item…</p></div>
        </div>
        <div id="ai-report-card" style="display:none; padding:16px; background:rgba(7,21,37,0.6); border-top:1px solid var(--border); border-bottom:1px solid var(--border); font-family:var(--font); color:var(--text);">
          <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;">
             <span style="font-size:12px; font-weight:600; text-transform:uppercase; letter-spacing:1px; color:#22d3ee;">AI Visual Report</span>
             <span id="ai-report-status" style="font-size:11px; padding:3px 8px; border-radius:4px; font-weight:bold;">Review</span>
          </div>
          <div style="font-size:15px; font-weight:500; color:#e8f0fa; margin-bottom:8px;" id="ai-report-summary">No summary.</div>
          <div style="margin-bottom:12px;">
             <div style="font-size:12px; color:var(--muted); margin-bottom:4px;">Detected:</div>
             <div id="ai-report-objects" style="display:flex; flex-wrap:wrap; gap:6px;"></div>
          </div>
          <div style="margin-bottom:12px; display:none;" id="ai-report-defects-container">
             <div style="font-size:12px; color:#ef4444; margin-bottom:4px;">Defects / Flags:</div>
             <ul id="ai-report-defects" style="margin:0; padding-left:16px; font-size:13px; color:#fca5a5;"></ul>
          </div>
          <div style="font-size:13px; color:var(--muted); line-height:1.4; border-left:2px solid var(--border-hi); padding-left:8px; font-style:italic;" id="ai-report-notes"></div>
        </div>
        <div class="panel-ft" style="background:rgba(4,8,15,.3);border-top:1px solid var(--border);padding:16px;">
          <button class="btn btn-primary" style="width:100%;display:flex;align-items:center;justify-content:center;gap:8px;" id="scan-item-btn" onclick="handleScanButtonClick()">
            <svg width="20" height="20" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M12 1v4M12 19v4M4.22 4.22l2.83 2.83M16.95 16.95l2.83 2.83M1 12h4M19 12h4"/></svg>
            <span id="scan-btn-text">Scan Terminal Serial Number</span>
          </button>
        </div>
      </div>

    </div>
  </div>

  <div id="screen-remarks" class="screen">
    <div class="card">
      <h2>Final details</h2>
      <p class="lead">Enter the terminal serial and any general notes.</p>
      <div class="field-label">Terminal serial</div>
      <input id="serial" class="serial-field" type="text" maxlength="64" placeholder="e.g. PAX-007-2026" autocomplete="off">
      <div class="field-label">General Remarks</div>
      <textarea id="remarks" class="std-notes" maxlength="400" placeholder="e.g. Minor scratch on rear panel…" oninput="countChars()"></textarea>
      <div class="char"><span id="chars">0</span> / 400</div>
      <div class="actions"><button class="btn btn-ghost" onclick="finish()">Skip notes</button><button class="btn btn-primary" onclick="finish()">Save and finish</button></div>
    </div>
  </div>

  <div id="screen-confirm" class="screen">
    <div class="card confirm-card">
      <div class="confirm-icon"><svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M5 13l4 4L19 7"/></svg></div>
      <h2>Record saved</h2>
      <p class="lead">Checklist logged and synchronized.</p>
      <div class="summary" id="summary"></div>
      <div class="countdown"><div class="countdown-ring" id="cd">5</div>Signing out automatically</div>
    </div>
  </div>
</div>

<script>
const BACKEND_BASE = (window.location.port === "8000") 
  ? window.location.origin 
  : "http://10.193.115.239:8000";
const POLL=500;
let user=null, task=null, cdTimer=null, taskStartedAt=0;
let sessionScannedItems = new Set();
let states=[], sources=[]; 

const LABELS={receive:'Receive terminal',process:'Process and check',release:'Release terminal'};
const LISTS={
  receive:[
    {l:'POS Terminal (PAX A920)',r:1,d:1,m:0},
    {l:'POS Terminal',r:0,d:1,m:0},
    {l:'POS Battery',r:1,d:1,m:0},
    {l:'LAN Cable',r:1,d:1,m:0},
    {l:'POS Power Supply',r:1,d:1,m:0},
    {l:'Adaptor Plug',r:0,d:1,m:0},
    {l:'Antenna',r:0,d:1,m:0},
    {l:'Base',r:0,d:1,m:0},
    {l:'Cradle',r:0,d:1,m:0},
    {l:'POS Cable',r:0,d:1,m:0},
    {l:'Power Cord',r:0,d:1,m:0},
    {l:'Router',r:0,d:1,m:0},
    {l:'POS SIM Card',r:0,d:0,m:1},
    {l:'Screen condition intact',r:1,d:1,m:0},
    {l:'No physical damage',r:1,d:1,m:0},
    {l:'Serial number verified',r:1,d:0,m:1}
  ],
  process:[
    {l:'Software version checked',r:1,d:0,m:1},
    {l:'Network connectivity test',r:1,d:0,m:1},
    {l:'Payment function test',r:1,d:0,m:1},
    {l:'Receipt printer test',r:0,d:1,m:0},
    {l:'Screen responsiveness',r:1,d:1,m:0},
    {l:'Battery health check',r:1,d:0,m:1},
    {l:'Security seal intact',r:1,d:1,m:0},
    {l:'POS SIM card present',r:0,d:0,m:1}
  ],
  release:[
    {l:'POS Terminal (PAX A920)',r:1,d:1,m:0},
    {l:'POS Terminal',r:0,d:1,m:0},
    {l:'POS Battery',r:1,d:1,m:0},
    {l:'LAN Cable',r:1,d:1,m:0},
    {l:'POS Power Supply',r:1,d:1,m:0},
    {l:'Adaptor Plug',r:0,d:1,m:0},
    {l:'Antenna',r:0,d:1,m:0},
    {l:'Base',r:0,d:1,m:0},
    {l:'Cradle',r:0,d:1,m:0},
    {l:'POS Cable',r:0,d:1,m:0},
    {l:'Power Cord',r:0,d:1,m:0},
    {l:'Router',r:0,d:1,m:0},
    {l:'POS SIM Card',r:0,d:0,m:1},
    {l:'Packaging / box',r:0,d:1,m:0},
    {l:'Release form signed',r:1,d:0,m:1},
    {l:'Terminal config confirmed',r:1,d:0,m:1}
  ]
};

function show(id){document.querySelectorAll('.screen').forEach(s=>s.classList.remove('active'));document.getElementById(id).classList.add('active')}
function fmt(){return new Date().toLocaleString('en-PH',{dateStyle:'medium',timeStyle:'short'})}
function setUser(n,r){document.getElementById('user-text').textContent=n+' · '+r;document.getElementById('user-chip').style.display='flex'}
function setFlow(n){
  const bar=document.getElementById('flow-bar');
  const steps=bar.querySelectorAll('.flow-step'), connectors=bar.querySelectorAll('.flow-connector');
  steps.forEach((el,i)=>{el.classList.remove('active','done');if(i<n)el.classList.add('done');if(i===n)el.classList.add('active')});
  connectors.forEach((el,i)=>el.classList.toggle('done',i<n));
}
function tickClock(){
  const now=new Date();
  document.getElementById('lock-time').textContent=now.toLocaleTimeString('en-PH',{hour:'2-digit',minute:'2-digit',second:'2-digit'});
  document.getElementById('lock-date').textContent=now.toLocaleDateString('en-PH',{weekday:'long',year:'numeric',month:'long',day:'numeric'});
}
tickClock();setInterval(tickClock,1000);

function resetToLock(){
  if(cdTimer)clearInterval(cdTimer);
  user=null; task=null;
  stopPcCamera();
  stopLiveDetectionLoop();
  const reportCard = document.getElementById('ai-report-card');
  if (reportCard) reportCard.style.display = 'none';
  if (document.getElementById('camera-source-select')) {
    document.getElementById('camera-source-select').value = 'esp32';
  }
  document.getElementById('user-chip').style.display='none';
  document.getElementById('flow-bar').classList.add('hidden');
  show('screen-lock');
}

function showWelcome(d){
  document.getElementById('welcome-name').textContent=d.name;
  document.getElementById('welcome-role').textContent=d.role;
  document.getElementById('welcome-emp-id').textContent="ID: " + d.uid;
  document.getElementById('welcome-photo').src = 'https://ui-avatars.com/api/?name='+encodeURIComponent(d.name)+'&background=1e40af&color=fff&size=120';
  document.getElementById('flow-bar').classList.remove('hidden');
  setFlow(0);
  show('screen-welcome');
  setTimeout(showMenu, 2500); 
}

setInterval(()=>{
  fetch('/api/status').then(r=>r.json()).then(d=>{
    if(d.scanned){
      user={name:d.name,role:d.role,uid:d.uid};
      setUser(d.name,d.role);
      showWelcome(d);
    }
  }).catch(()=>{});
},POLL);

function showMenu(){
  document.getElementById('menu-name').textContent=user.name;
  document.getElementById('menu-time').textContent=fmt();
  setFlow(1);
  show('screen-menu');
}

function openTask(t){
  task=t; taskStartedAt=Date.now();
  sessionScannedItems.clear();
  
  isSerialScanned = false;
  
  const reportCard = document.getElementById('ai-report-card');
  if (reportCard) reportCard.style.display = 'none';
  
  const items = LISTS[t];
  states = items.map(()=>0);
  sources = items.map(()=>'m');
  
  document.getElementById('task-tag').className='tag '+t;
  document.getElementById('task-tag').textContent=LABELS[t];
  document.getElementById('task-title').textContent=LABELS[t];
  document.getElementById('task-staff').textContent=user.name+' · '+user.role;
  
  document.getElementById('scanned-list-view').style.display='flex';
  document.getElementById('checklist-view').style.display='none';
  document.getElementById('scanning-btns').style.display='flex';
  document.getElementById('verify-btns').style.display='none';
  document.getElementById('incident-report-block').style.display='none';
  document.getElementById('left-panel-title').textContent='Detected Items (Scanning Phase)';
  document.getElementById('scan-item-btn').disabled=false;
  
  renderScannedItems();
  updatePhaseUI();
  setFlow(2);
  show('screen-task');
  startLiveDetectionLoop();
}

let pcStream = null;
let isSerialScanned = false;

function handleScanButtonClick() {
  if (!isSerialScanned) {
    scanTerminalSerial();
  } else {
    scanSingleItem();
  }
}

function updatePhaseUI() {
  const banner = document.getElementById('scanning-phase-banner');
  const title = document.getElementById('phase-title');
  const badge = document.getElementById('phase-status-badge');
  const btnText = document.getElementById('scan-btn-text');
  
  if (!isSerialScanned) {
    if (banner) {
      banner.style.background = '#071525';
      banner.style.color = '#22d3ee';
    }
    if (title) title.innerHTML = '<span>Phase 1: Scan Terminal Serial</span>';
    if (badge) {
      badge.textContent = 'Required';
      badge.style.background = 'rgba(34,211,238,0.1)';
      badge.style.color = '#22d3ee';
    }
    if (btnText) btnText.textContent = 'Scan Terminal Serial Number';
  } else {
    const serialVal = document.getElementById('serial') ? document.getElementById('serial').value : '';
    if (banner) {
      banner.style.background = '#071c18';
      banner.style.color = '#10b981';
    }
    if (title) {
      title.innerHTML = `
        <span>Phase 2: Scan Checklist Items</span>
        <button class="btn btn-ghost" onclick="resetSerialScan()" style="font-size:10px; padding:2px 6px; height:auto; color:#ef4444; border-color:rgba(239,68,68,0.3); margin-left:12px; font-weight:normal; border-radius:3px; cursor:pointer;">Reset Serial</button>
      `;
    }
    if (badge) {
      badge.textContent = `Serial: ${serialVal}`;
      badge.style.background = 'rgba(16,185,129,0.1)';
      badge.style.color = '#10b981';
    }
    if (btnText) btnText.textContent = 'Scan Current Item';
  }
}

function resetSerialScan() {
  isSerialScanned = false;
  const serialField = document.getElementById('serial');
  if (serialField) serialField.value = '';
  updatePhaseUI();
  resumeLiveFeed();
}

async function scanTerminalSerial() {
  const btn = document.getElementById('scan-item-btn');
  if (btn) btn.disabled = true;
  
  const overlay = document.getElementById('overlay');
  if (overlay) {
    overlay.classList.add('on');
    const overlayText = document.getElementById('overlay-text');
    if (overlayText) {
      overlayText.innerHTML = 'Extracting Serial Number/Barcode…';
    }
  }
  
  const sourceSelect = document.getElementById('camera-source-select');
  const source = sourceSelect ? sourceSelect.value : 'esp32';
  
  let blob = null;
  
  if (source === 'pc') {
    const video = document.getElementById('pc-cam');
    if (!video || !video.srcObject) {
      alert("PC Webcam stream is not active.");
      if (btn) btn.disabled = false;
      if (overlay) overlay.classList.remove('on');
      return;
    }
    
    const canvas = document.createElement('canvas');
    canvas.width = video.videoWidth || 640;
    canvas.height = video.videoHeight || 480;
    const ctx = canvas.getContext('2d');
    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);
    
    blob = await new Promise(resolve => canvas.toBlob(resolve, 'image/jpeg', 0.90));
  } else {
    // ESP32 camera
    const img = document.getElementById('cam');
    const originalStreamSrc = img ? img.src : "";
    let captureUrl = "http://10.193.115.233/capture";
    
    if (img && originalStreamSrc && originalStreamSrc.includes('/stream')) {
      captureUrl = originalStreamSrc.replace('/stream', '/capture');
      img.src = "";
    }
    
    await new Promise(resolve => setTimeout(resolve, 250));
    
    try {
      const captureResp = await fetch(captureUrl);
      if (!captureResp.ok) throw new Error("ESP32-CAM capture failed");
      blob = await captureResp.blob();
    } catch (e) {
      console.error("ESP32 Serial capture failed:", e);
    } finally {
      if (img && originalStreamSrc) {
        img.src = originalStreamSrc;
      }
    }
  }
  
  if (!blob) {
    alert("Failed to capture image for serial scanning.");
    if (btn) btn.disabled = false;
    if (overlay) overlay.classList.remove('on');
    return;
  }
  
  try {
    const formData = new FormData();
    formData.append('file', blob, 'serial-scan.jpg');
    
    const res = await fetch(`${BACKEND_BASE}/api/visual/scan-serial`, {
      method: 'POST',
      body: formData
    });
    
    const data = await res.json();
    if (data.serial_number) {
      const serialVal = data.brand && data.brand.toLowerCase() !== 'unknown' 
        ? `${data.brand} - ${data.serial_number}` 
        : data.serial_number;
      document.getElementById('serial').value = serialVal;
      isSerialScanned = true;
      alert(`Terminal Detected!\nBrand/Model: ${data.brand || 'Unknown'}\nSerial/Barcode: ${data.serial_number}\n\nProceeding to product checklist scanning.`);
      updatePhaseUI();
    } else {
      alert("No serial number or barcode detected.\n\nMake sure the barcode/serial label is centered and clearly visible under the camera, then try again.");
    }
  } catch (err) {
    console.error("Serial scan failed:", err);
    alert("Serial Scanning Failed:\n" + err.message);
  } finally {
    if (btn) btn.disabled = false;
    if (overlay) overlay.classList.remove('on');
    resumeLiveFeed();
  }
}

async function simulateRfidTap() {
  const rfidUid = document.getElementById('mock-employee-select').value;
  if (!rfidUid) {
    alert("Please select a staff member to simulate.");
    return;
  }
  
  const backendUrl = BACKEND_BASE;
  
  try {
    const res = await fetch(`${backendUrl}/api/rfid/verify`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ rfid_uid: rfidUid, station_id: "Bay 1" })
    });
    
    if (!res.ok) {
      const data = await res.json();
      throw new Error(data.detail || "Authentication failed");
    }
    
    const data = await res.json();
    const emp = data.employee;
    
    user = { name: emp.name, role: emp.role, uid: emp.rfid_uid };
    setUser(emp.name, emp.role);
    showWelcome(user);
    
  } catch (err) {
    alert("Simulation Failed: " + err.message);
  }
}

async function startPcCamera() {
  if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
    alert("Webcam Access Blocked by Browser Security\n\n" +
          "Your browser blocks webcam access on insecure HTTP IP addresses (like http://10.193.115.239).\n\n" +
          "How to fix:\n" +
          "1. Recommended: Open the workstation page served from the backend at:\n" +
          "   http://localhost:8000/workstation\n" +
          "   (localhost is treated as secure by default)\n\n" +
          "2. Or allow webcam for this specific IP in your browser:\n" +
          "   - Go to: chrome://flags/#unsafely-treat-insecure-origin-as-secure\n" +
          "   - Add '" + window.location.origin + "' to the text area\n" +
          "   - Enable the flag, restart your browser, and reload the page.");
    document.getElementById('camera-source-select').value = 'esp32';
    toggleCameraSource();
    return;
  }
  try {
    pcStream = await navigator.mediaDevices.getUserMedia({ 
      video: { 
        width: 640,  
        height: 480, 
        facingMode: "environment" 
      } 
    });
    const video = document.getElementById('pc-cam');
    video.srcObject = pcStream;
    video.style.display = 'block';
    document.getElementById('cam').style.display = 'none';
    document.getElementById('cam-off').style.display = 'none';
    
    document.getElementById('cam-dot').classList.add('on');
    document.getElementById('cam-label').textContent = 'Webcam Live';
  } catch (err) {
    console.error("Failed to access PC camera:", err);
    alert("Could not access PC webcam: " + err.message + "\n\nMake sure your browser has permission and you are on a secure origin (localhost or HTTPS).");
    document.getElementById('camera-source-select').value = 'esp32';
    toggleCameraSource();
  }
}

function stopPcCamera() {
  if (pcStream) {
    pcStream.getTracks().forEach(track => track.stop());
    pcStream = null;
  }
  const video = document.getElementById('pc-cam');
  if (video) {
    video.srcObject = null;
    video.style.display = 'none';
  }
}

function resumeLiveFeed() {
  document.getElementById('scan-preview').style.display = 'none';
  // Keep the AI report card on the screen as requested
  
  const sourceSelect = document.getElementById('camera-source-select');
  const source = sourceSelect ? sourceSelect.value : 'esp32';
  if (source === 'pc') {
    document.getElementById('pc-cam').style.display = 'block';
    document.getElementById('cam').style.display = 'none';
  } else {
    document.getElementById('pc-cam').style.display = 'none';
    document.getElementById('cam').style.display = 'block';
  }
}

function displayAiReport(data) {
  const card = document.getElementById('ai-report-card');
  if (!card) return;
  
  const parsed = data.parsed_result || {};
  const status = data.status || "review";
  const summary = parsed.summary || data.result_text || "Analysis completed.";
  const objects = parsed.objects || [];
  const defects = parsed.defects || [];
  const notes = parsed.notes || "No extra operational remarks.";
  
  // Set status badge style
  const statusBadge = document.getElementById('ai-report-status');
  if (statusBadge) {
    statusBadge.textContent = status.toUpperCase();
    if (status === 'completed' || status === 'approved') {
      statusBadge.style.background = 'rgba(16,185,129,0.2)';
      statusBadge.style.color = '#10b981';
      statusBadge.style.border = '1px solid rgba(16,185,129,0.4)';
    } else if (status === 'defect') {
      statusBadge.style.background = 'rgba(239,68,68,0.2)';
      statusBadge.style.color = '#ef4444';
      statusBadge.style.border = '1px solid rgba(239,68,68,0.4)';
    } else if (status === 'missing') {
      statusBadge.style.background = 'rgba(245,158,11,0.2)';
      statusBadge.style.color = '#f59e0b';
      statusBadge.style.border = '1px solid rgba(245,158,11,0.4)';
    } else {
      statusBadge.style.background = 'rgba(59,130,246,0.2)';
      statusBadge.style.color = '#3b82f6';
      statusBadge.style.border = '1px solid rgba(59,130,246,0.4)';
    }
  }
  
  // Set summary and notes
  const summaryEl = document.getElementById('ai-report-summary');
  if (summaryEl) summaryEl.textContent = summary;
  const notesEl = document.getElementById('ai-report-notes');
  if (notesEl) notesEl.textContent = `"${notes}"`;
  
  // Render objects as neon tag pills
  const objectsContainer = document.getElementById('ai-report-objects');
  if (objectsContainer) {
    objectsContainer.innerHTML = '';
    if (objects.length > 0) {
      objects.forEach(obj => {
        const pill = document.createElement('span');
        pill.style.cssText = 'font-size:11px; padding:2px 8px; background:rgba(34,211,238,0.1); color:#22d3ee; border:1px solid rgba(34,211,238,0.2); border-radius:10px; font-weight:500; margin:2px 0;';
        pill.textContent = obj;
        objectsContainer.appendChild(pill);
      });
    } else {
      objectsContainer.innerHTML = '<span style="font-size:12px; color:var(--muted);">No objects list returned.</span>';
    }
  }
  
  // Render defects list if present
  const defectsContainer = document.getElementById('ai-report-defects-container');
  const defectsList = document.getElementById('ai-report-defects');
  if (defectsList && defectsContainer) {
    defectsList.innerHTML = '';
    if (defects && defects.length > 0) {
      defectsContainer.style.display = 'block';
      defects.forEach(def => {
        const li = document.createElement('li');
        li.textContent = def;
        defectsList.appendChild(li);
      });
    } else {
      defectsContainer.style.display = 'none';
    }
  }
  
  card.style.display = 'block';
}

let liveDetectionTimer = null;
let isScanningActive = false;

function startLiveDetectionLoop() {
  if (liveDetectionTimer) return;
  liveDetectionTimer = setInterval(async () => {
    if (document.getElementById('screen-task').classList.contains('active') && !isScanningActive) {
      await performLiveDetectionFrame();
    } else {
      clearLiveOverlay();
    }
  }, 600);
}

function stopLiveDetectionLoop() {
  if (liveDetectionTimer) {
    clearInterval(liveDetectionTimer);
    liveDetectionTimer = null;
  }
  clearLiveOverlay();
}

function clearLiveOverlay() {
  const canvas = document.getElementById('cam-overlay');
  if (canvas) {
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
  }
}

async function autoScanAndCapture(blob) {
  isScanningActive = true;
  const btn = document.getElementById('scan-item-btn');
  if (btn) btn.disabled = true;
  
  const overlay = document.getElementById('overlay');
  if (overlay) {
    overlay.classList.add('on');
    const overlayText = document.getElementById('overlay-text');
    if (overlayText) {
      overlayText.innerHTML = '<span style="color:#22d3ee; font-weight:bold;">AUTO-CAPTURED!</span><br>Syncing and storing data…';
    }
  }
  
  const formData = new FormData();
  formData.append('file', blob, 'auto-capture.jpg');
  formData.append('station_id', 'Bay 1');
  if (document.getElementById('serial') && document.getElementById('serial').value) {
    formData.append('serial_number', document.getElementById('serial').value);
  }
  
  try {
    const res = await fetch(`${BACKEND_BASE}/api/visual/analyze`, {
      method: 'POST',
      body: formData
    });
    
    let data;
    try {
      data = await res.json();
    } catch(e) {
      data = { error: "Invalid response from server" };
    }
    
    if (!res.ok) throw new Error((data.error || data.detail || "HTTP " + res.status));
    
    let parsed = data.parsed_result;
    if (typeof parsed === 'string') {
      try { parsed = JSON.parse(parsed); } catch(e) {}
    }
    const detected = parsed ? (parsed.objects || []) : [];
    
    if (detected.length > 0) {
       detected.forEach(item => sessionScannedItems.add(item));
    }
    renderScannedItems();
    
    // Show visual report card on UI
    displayAiReport(data);
    
    // Show static annotated preview with bounding boxes
    if (data.annotated_image) {
      const previewImg = document.getElementById('scan-preview');
      if (previewImg) {
        previewImg.src = data.annotated_image;
        previewImg.style.display = 'block';
      }
      const pcCam = document.getElementById('pc-cam');
      if (pcCam) pcCam.style.display = 'none';
      const cam = document.getElementById('cam');
      if (cam) cam.style.display = 'none';
      clearLiveOverlay();
    }
    
    if (overlay) overlay.classList.remove('on');
    
    // Show checklist verified state momentarily if everything checks out,
    // otherwise resume stream after showing the preview.
    await new Promise(resolve => setTimeout(resolve, 2500));
    
    resumeLiveFeed();
  } catch(err) {
    console.error("Auto scan failed:", err);
    if (overlay) overlay.classList.remove('on');
    alert("Auto-Scan Storing Failed:\n" + err.message);
  } finally {
    isScanningActive = false;
    if (btn) btn.disabled = false;
  }
}

async function performLiveDetectionFrame() {
  const sourceSelect = document.getElementById('camera-source-select');
  const source = sourceSelect ? sourceSelect.value : 'esp32';
  
  let blob = null;
  let elementToMeasure = null;
  
  if (source === 'pc') {
    const video = document.getElementById('pc-cam');
    if (!video || !video.srcObject || video.style.display === 'none') {
      clearLiveOverlay();
      return;
    }
    
    const captureCanvas = document.createElement('canvas');
    captureCanvas.width = video.videoWidth || 640;
    captureCanvas.height = video.videoHeight || 480;
    const ctx = captureCanvas.getContext('2d');
    ctx.drawImage(video, 0, 0, captureCanvas.width, captureCanvas.height);
    
    blob = await new Promise(resolve => captureCanvas.toBlob(resolve, 'image/jpeg', 0.8));
    elementToMeasure = video;
  } else {
    const img = document.getElementById('cam');
    if (!img || img.style.display === 'none' || !img.complete || img.naturalWidth === 0) {
      clearLiveOverlay();
      return;
    }
    
    try {
      const captureCanvas = document.createElement('canvas');
      captureCanvas.width = img.naturalWidth || 640;
      captureCanvas.height = img.naturalHeight || 480;
      const ctx = captureCanvas.getContext('2d');
      ctx.drawImage(img, 0, 0, captureCanvas.width, captureCanvas.height);
      
      blob = await new Promise((resolve, reject) => {
        try {
          captureCanvas.toBlob(resolve, 'image/jpeg', 0.8);
        } catch (e) {
          reject(e);
        }
      });
      elementToMeasure = img;
    } catch (corsErr) {
      clearLiveOverlay();
      return;
    }
  }
  
  if (!blob || !elementToMeasure) return;
  
  try {
    const formData = new FormData();
    formData.append('file', blob, 'live-frame.jpg');
    
    const res = await fetch(`${BACKEND_BASE}/api/visual/detect-live`, {
      method: 'POST',
      body: formData
    });
    
    if (!res.ok) return;
    
    const data = await res.json();
    if (document.getElementById('screen-task').classList.contains('active') && !isScanningActive) {
      drawLiveBoxes(data.boxes, elementToMeasure);
      
      // Auto scan checklist trigger logic
      if (isSerialScanned && data.boxes && data.boxes.length > 0) {
        const uncheckedItems = LISTS[task].filter(it => it.d && !sessionScannedItems.has(it.l));
        const hasUncheckedMatch = data.boxes.some(box => {
          const boxLabel = box.label.toLowerCase();
          return uncheckedItems.some(it => {
            const itemLabel = it.l.toLowerCase();
            return itemLabel.includes(boxLabel) || boxLabel.includes(itemLabel);
          });
        });
        
        if (hasUncheckedMatch) {
          autoScanAndCapture(blob);
        }
      }
    } else {
      clearLiveOverlay();
    }
  } catch (err) {
    console.debug("Live frame detection skipped:", err);
  }
}

function drawLiveBoxes(boxes, element) {
  const canvas = document.getElementById('cam-overlay');
  if (!canvas) return;
  
  const displayWidth = element.clientWidth;
  const displayHeight = element.clientHeight;
  canvas.width = displayWidth;
  canvas.height = displayHeight;
  
  const ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, displayWidth, displayHeight);
  
  if (!boxes || boxes.length === 0) return;
  
  const rawW = boxes[0].image_width;
  const rawH = boxes[0].image_height;
  
  const scaleX = displayWidth / rawW;
  const scaleY = displayHeight / rawH;
  
  boxes.forEach(item => {
    const [bx, by, bw, bh] = item.box;
    const x = bx * scaleX;
    const y = by * scaleY;
    const w = bw * scaleX;
    const h = bh * scaleY;
    
    ctx.strokeStyle = '#22d3ee';
    ctx.lineWidth = 3;
    ctx.shadowBlur = 10;
    ctx.shadowColor = 'rgba(34,211,238,0.6)';
    ctx.strokeRect(x, y, w, h);
    
    ctx.fillStyle = 'rgba(12,26,46,0.85)';
    ctx.shadowBlur = 0;
    const labelText = `${item.label} (${Math.round(item.confidence * 100)}%)`;
    ctx.font = 'bold 11px var(--font)';
    const textWidth = ctx.measureText(labelText).width;
    
    const tabY = (y - 18 > 0) ? y - 18 : y;
    ctx.fillRect(x, tabY, textWidth + 10, 18);
    
    ctx.fillStyle = '#e8f0fa';
    ctx.fillText(labelText, x + 5, tabY + 13);
  });
}

function toggleCameraSource() {
  document.getElementById('scan-preview').style.display = 'none';
  const source = document.getElementById('camera-source-select').value;
  if (source === 'pc') {
    startPcCamera();
  } else {
    stopPcCamera();
    document.getElementById('cam').style.display = 'block';
    const img = document.getElementById('cam');
    if (img.style.display === 'none') {
      img.style.display = 'block';
      img.src = img.src; 
    }
  }
}

async function scanSingleItem(){
  isScanningActive = true;
  clearLiveOverlay();
  const btn = document.getElementById('scan-item-btn');
  btn.disabled = true;
  document.getElementById('overlay').classList.add('on');
  document.getElementById('overlay-text').textContent = 'Analyzing item…';
  
  resumeLiveFeed();
  
  const sourceSelect = document.getElementById('camera-source-select');
  const source = sourceSelect ? sourceSelect.value : 'esp32';
  
  if (source === 'pc') {
    const video = document.getElementById('pc-cam');
    if (!video || !video.srcObject) {
      alert("PC Webcam stream is not active.");
      btn.disabled = false;
      document.getElementById('overlay').classList.remove('on');
      return;
    }
    
    const canvas = document.createElement('canvas');
    canvas.width = video.videoWidth || 640;
    canvas.height = video.videoHeight || 480;
    const ctx = canvas.getContext('2d');
    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);
    
    canvas.toBlob(async (blob) => {
      if (!blob) {
        alert("Failed to capture image from webcam.");
        btn.disabled = false;
        document.getElementById('overlay').classList.remove('on');
        return;
      }
      
      const formData = new FormData();
      formData.append('file', blob, 'pc-capture.jpg');
      formData.append('station_id', 'Bay 1');
      if (document.getElementById('serial') && document.getElementById('serial').value) {
        formData.append('serial_number', document.getElementById('serial').value);
      }
      
      try {
        const res = await fetch(`${BACKEND_BASE}/api/visual/analyze`, {
          method: 'POST',
          body: formData
        });
        
        let data;
        try {
          data = await res.json();
        } catch(e) {
          data = { error: "Invalid response from server" };
        }
        
        if (!res.ok) throw new Error((data.error || data.detail || "HTTP " + res.status));
        
        let parsed = data.parsed_result;
        if (typeof parsed === 'string') {
          try { parsed = JSON.parse(parsed); } catch(e) {}
        }
        const detected = parsed ? (parsed.objects || []) : [];
        
        if (detected.length === 0) {
           alert("No recognizable items found in the image. Please try again.");
        } else {
           detected.forEach(item => sessionScannedItems.add(item));
        }
        
        // Show visual report card on UI
        displayAiReport(data);
        
        // Show scan preview with labeled boxes if returned
        if (data.annotated_image) {
          const previewImg = document.getElementById('scan-preview');
          previewImg.src = data.annotated_image;
          previewImg.style.display = 'block';
          document.getElementById('pc-cam').style.display = 'none';
          document.getElementById('cam').style.display = 'none';
        }
        
        renderScannedItems();
      } catch(err) {
        console.error("PC Scan failed:", err);
        alert("Visual Analysis Failed:\n" + err.message);
      } finally {
        isScanningActive = false;
        btn.disabled = false;
        document.getElementById('overlay').classList.remove('on');
      }
    }, 'image/jpeg', 0.90);
    
  } else {
    // Original ESP32 Camera logic
    const img = document.getElementById('cam');
    const originalStreamSrc = img ? img.src : "";
    let captureUrl = "http://10.193.115.233/capture"; // fallback
    
    if (img && originalStreamSrc && originalStreamSrc.includes('/stream')) {
      captureUrl = originalStreamSrc.replace('/stream', '/capture');
      img.src = "";
    }
    
    await new Promise(resolve => setTimeout(resolve, 250));
    
    const payload = {
      capture_url: captureUrl,
      station_id: "Bay 1"
    };
    try {
      const res = await fetch(`${BACKEND_BASE}/api/visual/analyze-from-camera`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
      
      let data;
      try {
        data = await res.json();
      } catch(e) {
        data = { error: "Invalid response from server" };
      }
      
      if (!res.ok) throw new Error((data.error || data.detail || "HTTP " + res.status));
      
      let parsed = data.parsed_result;
      if (typeof parsed === 'string') {
        try { parsed = JSON.parse(parsed); } catch(e) {}
      }
      const detected = parsed ? (parsed.objects || []) : [];
      
      if (detected.length === 0) {
         alert("No recognizable items found in the image. Please try again.");
      } else {
         detected.forEach(item => sessionScannedItems.add(item));
      }
      
      // Show visual report card on UI
      displayAiReport(data);
      
      // Show scan preview with labeled boxes if returned
      if (data.annotated_image) {
        const previewImg = document.getElementById('scan-preview');
        previewImg.src = data.annotated_image;
        previewImg.style.display = 'block';
        document.getElementById('pc-cam').style.display = 'none';
        document.getElementById('cam').style.display = 'none';
      }
      
      renderScannedItems();
    } catch(err) {
      console.error("Scan failed:", err);
      alert("Connection to Backend/Gemini Failed:\n" + err.message + "\n\nPlease check your BACKEND_URL in the ESP32 code and ensure your Node server is running.");
    } finally {
      if (img && originalStreamSrc) {
        img.src = originalStreamSrc;
      }
      isScanningActive = false;
      btn.disabled = false;
      document.getElementById('overlay').classList.remove('on');
    }
  }
}

function renderScannedItems() {
  const el = document.getElementById('scanned-list-view');
  document.getElementById('prog-text').textContent = sessionScannedItems.size;
  el.innerHTML = '';
  
  if(sessionScannedItems.size === 0) {
    el.innerHTML = '<div style="padding:20px;text-align:center;color:var(--muted);font-size:14px;">No items scanned yet.<br><br>Place an item in front of the camera and tap <strong>Scan Current Item</strong>.</div>';
    return;
  }
  
  sessionScannedItems.forEach(item => {
    el.innerHTML += `
      <div class="check-row auto" style="cursor:default;">
        <div class="cb"></div>
        <div class="check-body">
          <div class="check-label">${item}</div>
          <div class="check-meta"><span class="badge auto">Detected</span></div>
        </div>
      </div>
    `;
  });
}

function verifyChecklist() {
  const items = LISTS[task];
  let missingCount = 0;
  
  items.forEach((it, i) => {
    if (it.m) { states[i]=0; sources[i]='m'; return; } 
    
    let match = false;
    sessionScannedItems.forEach(scannedObj => {
      if (scannedObj.toLowerCase().includes(it.l.toLowerCase()) || 
          it.l.toLowerCase().includes(scannedObj.toLowerCase())) {
          match = true;
      }
    });
    
    states[i] = match ? 1 : 0;
    sources[i] = match ? 'a' : 'm';
    
    if (it.r && !match) missingCount++;
  });

  document.getElementById('scanned-list-view').style.display = 'none';
  document.getElementById('checklist-view').style.display = 'flex';
  document.getElementById('scanning-btns').style.display = 'none';
  document.getElementById('verify-btns').style.display = 'flex';
  document.getElementById('left-panel-title').textContent = 'Verification Checklist';
  document.getElementById('scan-item-btn').disabled = true;

  if (missingCount > 0) {
    document.getElementById('incident-report-block').style.display = 'block';
  } else {
    document.getElementById('incident-report-block').style.display = 'none';
  }
  
  renderChecklist();
}

function backToScan() {
  document.getElementById('scanned-list-view').style.display = 'flex';
  document.getElementById('checklist-view').style.display = 'none';
  document.getElementById('scanning-btns').style.display = 'flex';
  document.getElementById('verify-btns').style.display = 'none';
  document.getElementById('left-panel-title').textContent = 'Detected Items (Scanning Phase)';
  document.getElementById('scan-item-btn').disabled = false;
  document.getElementById('incident-report-block').style.display = 'none';
  renderScannedItems();
}

function renderChecklist() {
  const items = LISTS[task];
  const el = document.getElementById('checklist-view');
  el.innerHTML = '';
  let doneCount = 0;

  items.forEach((it, i) => {
    const on = states[i];
    if (on) doneCount++;

    const auto = sources[i] === 'a';
    const mc = it.m; 
    
    let cls = 'check-row';
    if (on && auto) cls += ' auto';
    else if (on) cls += ' done';
    
    if (it.r && !on && !mc) cls += ' needs-input'; 

    let meta = '';
    if (it.r) meta += '<span class="badge req">Required</span>';
    if (it.d && !it.m) meta += on ? '<span class="badge auto">Detected</span>' : '<span class="badge miss">Missing</span>';
    if (it.m) meta += '<span class="badge manual">Confirm Manual</span>';

    const row = document.createElement('div');
    row.className = cls;
    row.innerHTML = `
      <div class="cb"></div>
      <div class="check-body">
        <div class="check-label">${it.l}</div>
        <div class="check-meta">${meta}</div>
      </div>
    `;
    
    if (it.m || (!on && !auto)) { 
      row.onclick = () => toggle(i);
    }
    el.appendChild(row);
  });
  
  document.getElementById('prog-text').textContent = doneCount + ' / ' + items.length;
}

function toggle(i) {
  states[i] = states[i] ? 0 : 1;
  sources[i] = 'm';
  renderChecklist();
  
  const items = LISTS[task];
  const missingCount = items.filter((it, idx) => it.r && !states[idx]).length;
  if (missingCount === 0) {
    document.getElementById('incident-report-block').style.display = 'none';
  }
}

function capture(){
  const missingCount = LISTS[task].filter((it, idx) => it.r && !states[idx]).length;
  if(missingCount > 0 && document.getElementById('incident-notes').value.trim() === '') {
      alert('Missing materials detected. An incident report explanation is required.');
      document.getElementById('incident-notes').focus();
      return;
  }
  
  document.getElementById('cap-btn').textContent='Saving…';
  fetch('/api/capture',{method:'POST'}).catch(()=>{});
  setTimeout(()=>{
    document.getElementById('serial').value='';
    document.getElementById('remarks').value='';
    countChars();
    setFlow(2);
    show('screen-remarks');
    document.getElementById('cap-btn').textContent='Submit & Finalize Task';
  }, 500);
}

function countChars(){document.getElementById('chars').textContent=document.getElementById('remarks').value.length}

async function finish(){
  const items = LISTS[task];
  const rm = (document.getElementById('remarks').value||'').trim();
  const serial = (document.getElementById('serial').value||'').trim();
  const incidentNotes = document.getElementById('incident-notes').value.trim();
  
  if(!serial){alert('Enter the terminal serial number.');return}
  
  let finalRemarks = rm;
  if (incidentNotes) finalRemarks = "INCIDENT REPORT:\n" + incidentNotes + (rm ? "\n\nStandard Notes:\n" + rm : "");

  const auto = sources.filter(s=>s==='a').length;
  const durationSec = taskStartedAt ? Math.round((Date.now()-taskStartedAt)/1000) : 0;
  
  const payload = {
    terminal_id: serial,
    terminal_brand: 'PAX',
    staff_id: user.uid,
    staff_name: user.name,
    staff_role: user.role,
    station: 'Bay 1',
    task: task,
    remarks: finalRemarks,
    duration_sec: durationSec,
    items_checked: states.filter(Boolean).length,
    items_total: items.length,
    camera_scan_count: auto
  };

  let syncText = 'Saving to cloud…';
  try{
    const r = await fetch(`${BACKEND_BASE}/api/transaction`,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});
    const d = await r.json();
    syncText = d.synced ? 'Synced to supervisor dashboard' : 'Saved on station (cloud offline)';
  }catch(e){ syncText = 'Saved on station (cloud offline)'; }
  
  document.getElementById('summary').innerHTML=`
    <div class="row"><span>Staff</span><span>${user.name}</span></div>
    <div class="row"><span>Role</span><span>${user.role}</span></div><hr>
    <div class="row"><span>Serial</span><span>${serial}</span></div>
    <div class="row"><span>Task</span><span>${LABELS[task]}</span></div>
    <div class="row"><span>Items checked</span><span>${states.filter(Boolean).length} / ${items.length}</span></div>
    <div class="row"><span>Camera scan</span><span>${auto} matched items</span></div>
    <div class="row"><span>Cloud</span><span>${syncText}</span></div>
    ${finalRemarks ? `<hr><div class="row"><span>Remarks</span><span style="max-width:200px;font-size:13px">${finalRemarks}</span></div>` : ''}
  `;
  
  setFlow(3);
  show('screen-confirm');
  let s=5; document.getElementById('cd').textContent=s;
  if(cdTimer)clearInterval(cdTimer);
  cdTimer=setInterval(()=>{ s--; document.getElementById('cd').textContent=s; if(s<=0){clearInterval(cdTimer); resetToLock();} }, 1000);
}

function camOk(){document.getElementById('cam-dot').classList.add('on');document.getElementById('cam-label').textContent='Live'}
function camFail(){document.getElementById('cam-dot').classList.remove('on');document.getElementById('cam-label').textContent='Offline';document.getElementById('cam').style.display='none';document.getElementById('cam-off').style.display='flex'}
</script>
</body>
</html>
)rawliteral";


const char REGISTER_HTML[] PROGMEM = R"ECSREG(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Card registration — ECS</title>
<link href="https://fonts.googleapis.com/css2?family=Source+Sans+3:wght@400;600;700&display=swap" rel="stylesheet">
<style>
body{font-family:'Source Sans 3',system-ui,sans-serif;background:#04080f;color:#e8f0fa;min-height:100vh;display:flex;align-items:center;justify-content:center;margin:0;padding:20px}
body::before{content:'';position:fixed;width:50vw;height:50vw;top:-10%;left:-10%;background:rgba(30,64,175,.15);filter:blur(80px);pointer-events:none}
.card{background:linear-gradient(145deg,rgba(14,28,50,.95),rgba(10,20,38,.92));border:1px solid rgba(96,165,250,.22);border-radius:16px;padding:40px;max-width:440px;width:100%;box-shadow:0 24px 56px rgba(0,0,0,.4);position:relative;z-index:1}
h1{font-size:22px;font-weight:700;margin-bottom:8px}
p{color:#8ba3be;font-size:14px;line-height:1.6;margin-bottom:28px}
.uid{background:rgba(4,8,15,.5);border:1px dashed rgba(96,165,250,.25);border-radius:14px;padding:28px;text-align:center;font-family:ui-monospace,monospace;font-size:26px;font-weight:700;letter-spacing:.14em;color:#38bdf8;min-height:80px;display:flex;align-items:center;justify-content:center;margin-bottom:16px}
.wait{color:#5a7390;font-size:14px;font-weight:500}
.btn{background:linear-gradient(135deg,#1e40af,#3b82f6,#38bdf8);color:#fff;border:none;padding:13px 24px;border-radius:12px;font-size:14px;font-weight:700;cursor:pointer;width:100%;display:none;margin-bottom:20px;box-shadow:0 6px 20px rgba(59,130,246,.35)}
.steps{font-size:13px;color:#8ba3be;line-height:1.9;background:rgba(4,8,15,.4);padding:18px;border-radius:12px;border:1px solid rgba(56,120,200,.12)}
.steps code{background:rgba(59,130,246,.15);padding:2px 7px;border-radius:5px;font-size:12px;color:#38bdf8}
a{display:inline-block;margin-top:18px;font-size:13px;color:#5a7390;text-decoration:none}
a:hover{color:#e8f0fa}
</style>
</head>
<body>
<div class="card">
  <h1>Register a card</h1>
  <p>Tap a card on the reader. Copy the UID into <code>staffDB[]</code> in the sketch, then re-flash.</p>
  <div class="uid" id="uid"><span class="wait">Waiting for card…</span></div>
  <button class="btn" id="copy" onclick="copyUid()">Copy UID</button>
  <div class="steps">
    <strong style="color:#e8f0fa">Steps</strong><br>
    1. Tap card on reader<br>
    2. Copy UID above<br>
    3. Paste into <code>staffDB[]</code> with name and role<br>
    4. Upload sketch again
  </div>
  <a href="/">Back to workstation</a>
</div>
<script>
let last='';
setInterval(()=>{
  fetch('/api/rawuid').then(r=>r.json()).then(d=>{
    if(d.uid&&d.uid!==last){last=d.uid;document.getElementById('uid').textContent=d.uid;document.getElementById('copy').style.display='block'}
  }).catch(()=>{});
},500);
function copyUid(){navigator.clipboard.writeText(last)}
</script>
</body>
</html>
)ECSREG";