#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>ECS Post Admin</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
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
.lock-clock-label{font-size:10px;font-weight:700;text-transform:uppercase;letter-spacing:.1em;color:var(--muted-2);margin-bottom:8px}
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
.welcome-avatar-wrap{width:104px;height:104px;border-radius:50%;padding:3px;background:var(--grad);margin:0 auto 16px}
.welcome-avatar{width:100%;height:100%;border-radius:50%;background:var(--navy-2);font-size:30px;font-weight:700;color:var(--blue-400);display:flex;align-items:center;justify-content:center}
.welcome-label{font-size:10px;font-weight:700;text-transform:uppercase;letter-spacing:.12em;color:var(--blue-400)}
.welcome-right{flex:1;min-width:0}
.welcome-name{font-size:34px;font-weight:700;letter-spacing:-.03em;margin-bottom:6px;line-height:1.15}
.welcome-role{font-size:15px;color:var(--muted);margin-bottom:28px;font-weight:500}
.welcome-time{font-size:13px;color:var(--muted-2);margin-top:16px}
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
.task-card:hover,.task-card:active{border-color:var(--border-hi);box-shadow:0 16px 40px rgba(0,0,0,.3);transform:translateY(-2px)}
.task-icon{width:48px;height:48px;border-radius:12px;margin-bottom:18px;display:flex;align-items:center;justify-content:center}
.task-card.receive .task-icon{background:rgba(30,64,175,.2);color:#60a5fa}
.task-card.process .task-icon{background:rgba(37,99,235,.18);color:#38bdf8}
.task-card.release .task-icon{background:rgba(34,211,238,.12);color:#22d3ee}
.task-step{font-size:11px;font-weight:700;text-transform:uppercase;letter-spacing:.1em;color:var(--blue-400);margin-bottom:8px}
.task-card h3{font-size:17px;font-weight:700;margin-bottom:8px}
.task-card p{font-size:13px;color:var(--muted);line-height:1.6}
.task-arrow{position:absolute;bottom:24px;right:24px;width:32px;height:32px;border-radius:50%;background:rgba(59,130,246,.1);border:1px solid var(--border);display:flex;align-items:center;justify-content:center;color:var(--blue-400)}
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
.check-row:hover{border-color:var(--border-hi);background:rgba(14,28,50,.6)}
.check-row.done{border-color:rgba(59,130,246,.28);background:rgba(30,64,175,.1)}
.check-row.auto{border-color:rgba(34,211,238,.3);background:var(--cyan-soft)}
.check-row.needs-input{border-color:rgba(245,158,11,.28);background:var(--warn-soft)}
.cb{width:24px;height:24px;border:2px solid var(--border-hi);border-radius:7px;flex-shrink:0;display:flex;align-items:center;justify-content:center;margin-top:1px}
.check-row.done .cb,.check-row.auto .cb{background:var(--grad);border-color:transparent}
.check-row.done .cb::after,.check-row.auto .cb::after{content:'';width:5px;height:9px;border:solid #fff;border-width:0 2px 2px 0;transform:rotate(45deg);margin-top:-2px}
.check-body{flex:1;min-width:0}
.check-label{font-size:14px;font-weight:600;line-height:1.45}
.check-row.done .check-label{color:var(--muted);text-decoration:line-through}
.check-meta{display:flex;gap:6px;margin-top:6px;flex-wrap:wrap}
.badge{font-size:9px;font-weight:700;padding:3px 8px;border-radius:5px;text-transform:uppercase;letter-spacing:.05em}
.badge.req{background:var(--danger-soft);color:#fca5a5}
.badge.auto{background:rgba(34,211,238,.15);color:#67e8f9}
.badge.manual{background:rgba(59,130,246,.15);color:#93c5fd}
.badge.miss{background:var(--danger-soft);color:#fca5a5}
.mc-row{display:flex;gap:8px;margin-top:10px}
.mc{flex:1;min-height:var(--touch);padding:10px;border-radius:8px;border:1px solid var(--border);background:rgba(4,8,15,.4);font-size:13px;font-weight:600;cursor:pointer;color:var(--muted);touch-action:manipulation}
.mc.yes.active{background:var(--accent-soft);border-color:var(--blue-600);color:var(--blue-400)}
.mc.no.active{background:var(--danger-soft);border-color:var(--danger);color:#fca5a5}
.scan-note{margin:12px 12px 0;padding:12px 16px;background:var(--cyan-soft);border:1px solid rgba(34,211,238,.22);border-radius:var(--radius);font-size:13px;color:#7dd3fc;display:none;font-weight:500}
.scan-note.show{display:block}
.panel-ft{padding:16px 18px;border-top:1px solid var(--border);flex-shrink:0;background:rgba(4,8,15,.3)}
.prog{height:6px;background:rgba(255,255,255,.06);border-radius:3px;margin-bottom:14px;overflow:hidden}
.prog>div{height:100%;border-radius:3px;transition:width .35s var(--ease);background:var(--grad-h)}
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
.cam-caption{padding:0 18px 14px;font-size:12px;color:var(--muted-2);line-height:1.5;flex-shrink:0}
.overlay{position:absolute;inset:0;background:rgba(4,8,15,.85);backdrop-filter:blur(4px);display:none;align-items:center;justify-content:center;flex-direction:column;gap:14px}
.overlay.on{display:flex}
.spinner{width:44px;height:44px;border-radius:50%;border:2px solid var(--border-hi);border-top-color:var(--blue-400);animation:spin .8s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}
#screen-remarks,#screen-confirm{align-items:center;justify-content:center;padding:32px}
.card{background:linear-gradient(145deg,var(--surface-2),var(--surface));border:1px solid var(--border-hi);border-radius:var(--radius-lg);padding:40px 44px;box-shadow:0 24px 56px rgba(0,0,0,.35);width:100%;max-width:540px}
.card h2{font-size:24px;font-weight:700;margin-bottom:8px}
.card .lead{font-size:15px;color:var(--muted);margin-bottom:28px;line-height:1.6}
.field-label{font-size:11px;font-weight:700;color:var(--muted);margin-bottom:10px;text-transform:uppercase;letter-spacing:.07em}
textarea{width:100%;height:140px;padding:16px 18px;border:1px solid var(--border-hi);border-radius:var(--radius);font-family:var(--font);font-size:15px;resize:none;outline:none;background:rgba(4,8,15,.5);color:var(--text)}
.serial-field{width:100%;padding:16px 18px;border:1px solid var(--border-hi);border-radius:var(--radius);font-family:var(--font);font-size:15px;outline:none;background:rgba(4,8,15,.5);color:var(--text);margin-bottom:18px}
.serial-field:focus,textarea:focus{border-color:rgba(59,130,246,.5);box-shadow:0 0 0 3px var(--accent-soft)}
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
@media(max-width:900px){.lock-split{grid-template-columns:1fr}.lock-brand{display:none}.welcome-card{flex-direction:column;text-align:center}.task-grid{grid-template-columns:1fr}.task-body{grid-template-columns:1fr}.flow-step span.label{display:none}}
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
        <div class="lock-clock-label">Local time</div>
        <div class="lock-time" id="lock-time">--:--</div>
        <div class="lock-date" id="lock-date">—</div>
      </div>
      <div class="lock-prompt">
        <div class="card-prompt">
          <div class="card-icon"><svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.75"><rect x="2" y="5" width="20" height="14" rx="2"/><path d="M2 10h20"/></svg></div>
          <h2>Sign in required</h2>
          <p>Hold card to reader</p>
          <div class="scan-hint"><span class="pulse"></span> Listening for card</div>
        </div>
      </div>
    </div>
  </div>
  <div id="screen-welcome" class="screen">
    <div class="welcome-card">
      <div class="welcome-left">
        <div class="welcome-avatar-wrap"><div class="welcome-avatar" id="welcome-avatar">—</div></div>
        <div class="welcome-label">Identity verified</div>
      </div>
      <div class="welcome-right">
        <div class="welcome-name" id="welcome-name"></div>
        <div class="welcome-role" id="welcome-role"></div>
        <button class="btn btn-primary" style="width:100%;max-width:280px" onclick="showMenu()">Enter workspace</button>
        <div class="welcome-time" id="welcome-time"></div>
      </div>
    </div>
  </div>
  <div id="screen-menu" class="screen">
    <div class="menu-head"><p>Welcome back,</p><h2 id="menu-name"></h2></div>
    <div class="task-grid">
      <div class="task-card receive" onclick="openTask('receive')">
        <div class="task-icon"><svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/></svg></div>
        <div class="task-step">Route 01</div><h3>Receive terminal</h3><p>Log incoming units and verify physical condition on intake.</p>
        <div class="task-arrow"><svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M5 12h14M13 6l6 6-6 6"/></svg></div>
      </div>
      <div class="task-card process" onclick="openTask('process')">
        <div class="task-icon"><svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M12 1v4M12 19v4M4.22 4.22l2.83 2.83M16.95 16.95l2.83 2.83M1 12h4M19 12h4"/></svg></div>
        <div class="task-step">Route 02</div><h3>Process and check</h3><p>Inspect, test, and document technical findings.</p>
        <div class="task-arrow"><svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M5 12h14M13 6l6 6-6 6"/></svg></div>
      </div>
      <div class="task-card release" onclick="openTask('release')">
        <div class="task-icon"><svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 2L11 13M22 2l-7 20-4-9-9-4 20-7z"/></svg></div>
        <div class="task-step">Route 03</div><h3>Release terminal</h3><p>Confirm completeness before field deployment.</p>
        <div class="task-arrow"><svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M5 12h14M13 6l6 6-6 6"/></svg></div>
      </div>
    </div>
    <div class="menu-foot"><span class="menu-time" id="menu-time"></span><button class="btn btn-danger-ghost" onclick="resetToLock()">Sign out</button></div>
  </div>
  <div id="screen-task" class="screen">
    <div class="task-bar">
      <div class="task-bar-left"><span class="tag" id="task-tag"></span><div><h2 id="task-title"></h2><div class="sub" id="task-staff"></div></div></div>
      <button class="btn btn-danger-ghost" onclick="resetToLock()">Cancel</button>
    </div>
    <div class="task-body">
      <div class="panel">
        <div class="panel-hd">Checklist <span class="count" id="prog-text">0 / 0</span></div>
        <div class="scan-note" id="scan-note"><span id="scan-count">0</span> items found on camera. Review before continuing.</div>
        <div class="checklist" id="checklist"></div>
        <div class="panel-ft">
          <div class="prog"><div id="prog-bar" style="width:0%"></div></div>
          <div class="ft-btns">
            <button class="btn btn-ghost" id="scan-btn" onclick="runScan()">Scan workspace</button>
            <button class="btn btn-primary" id="cap-btn" disabled onclick="capture()">Complete checklist first</button>
          </div>
        </div>
      </div>
      <div class="panel">
        <div class="panel-hd">Camera <span class="cam-live"><span class="live-dot" id="cam-dot"></span><span id="cam-label">Connecting</span></span></div>
        <div class="cam-wrap">
          <img id="cam" src="__CAM_URL__" alt="" onload="camOk()" onerror="camFail()">
          <div class="cam-off" id="cam-off"><p><strong>Camera unavailable</strong></p><p>Check ESP32-CAM power and network.</p></div>
          <div class="overlay" id="overlay"><div class="spinner"></div><p>Scanning frame…</p></div>
        </div>
        <div class="cam-caption">Position the terminal and accessories within the frame before scanning.</div>
      </div>
    </div>
  </div>
  <div id="screen-remarks" class="screen">
    <div class="card">
      <h2>Unit details</h2>
      <p class="lead">Enter the terminal serial, then optional notes.</p>
      <div class="field-label">Terminal serial</div>
      <input id="serial" class="serial-field" type="text" maxlength="64" placeholder="e.g. PAX-007-2026" autocomplete="off">
      <div class="field-label">Notes</div>
      <textarea id="remarks" maxlength="400" placeholder="e.g. Minor scratch on rear panel…" oninput="countChars()"></textarea>
      <div class="char"><span id="chars">0</span> / 400</div>
      <div class="actions"><button class="btn btn-ghost" onclick="finish()">Skip notes</button><button class="btn btn-primary" onclick="finish()">Save and finish</button></div>
    </div>
  </div>
  <div id="screen-confirm" class="screen">
    <div class="card confirm-card">
      <div class="confirm-icon"><svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M5 13l4 4L19 7"/></svg></div>
      <h2>Record saved</h2>
      <p class="lead">Photo captured and checklist logged.</p>
      <div class="summary" id="summary"></div>
      <div class="countdown"><div class="countdown-ring" id="cd">5</div>Signing out automatically</div>
    </div>
  </div>
</div>
<script>
const POLL=500;let user=null,task=null,states=[],sources=[],manual={},scanned=false,cdTimer=null,taskStartedAt=0;
const LABELS={receive:'Receive terminal',process:'Process and check',release:'Release terminal'};
const LISTS={receive:[{l:'POS Terminal (PAX A920)',r:1,d:1,m:0},{l:'POS Battery',r:1,d:1,m:0},{l:'LAN Cable',r:1,d:1,m:0},{l:'POS Power Supply',r:1,d:1,m:0},{l:'Adaptor Plug',r:0,d:1,m:0},{l:'POS SIM Card',r:0,d:0,m:1},{l:'Screen condition intact',r:1,d:1,m:0},{l:'No physical damage',r:1,d:1,m:0},{l:'Serial number verified',r:1,d:0,m:0}],process:[{l:'Software version checked',r:1,d:0,m:0},{l:'Network connectivity test',r:1,d:0,m:0},{l:'Payment function test',r:1,d:0,m:0},{l:'Receipt printer test',r:0,d:1,m:0},{l:'Screen responsiveness',r:1,d:1,m:0},{l:'Battery health check',r:1,d:0,m:0},{l:'Security seal intact',r:1,d:1,m:0},{l:'POS SIM card present',r:0,d:0,m:1}],release:[{l:'POS Terminal (PAX A920)',r:1,d:1,m:0},{l:'POS Battery',r:1,d:1,m:0},{l:'LAN Cable',r:1,d:1,m:0},{l:'POS Power Supply',r:1,d:1,m:0},{l:'Adaptor Plug',r:0,d:1,m:0},{l:'POS SIM Card',r:0,d:0,m:1},{l:'Packaging / box',r:0,d:1,m:0},{l:'Release form signed',r:1,d:0,m:0},{l:'Terminal config confirmed',r:1,d:0,m:0}]};
function show(id){document.querySelectorAll('.screen').forEach(s=>s.classList.remove('active'));document.getElementById(id).classList.add('active')}
function fmt(){return new Date().toLocaleString('en-PH',{dateStyle:'medium',timeStyle:'short'})}
function setUser(n,r){document.getElementById('user-text').textContent=n+' · '+r;document.getElementById('user-chip').style.display='flex'}
function setFlow(n){const bar=document.getElementById('flow-bar');const steps=bar.querySelectorAll('.flow-step');const connectors=bar.querySelectorAll('.flow-connector');steps.forEach((el,i)=>{el.classList.remove('active','done');if(i<n)el.classList.add('done');if(i===n)el.classList.add('active')});connectors.forEach((el,i)=>el.classList.toggle('done',i<n))}
function tickClock(){const now=new Date();document.getElementById('lock-time').textContent=now.toLocaleTimeString('en-PH',{hour:'2-digit',minute:'2-digit',second:'2-digit'});document.getElementById('lock-date').textContent=now.toLocaleDateString('en-PH',{weekday:'long',year:'numeric',month:'long',day:'numeric'})}
tickClock();setInterval(tickClock,1000);
function resetToLock(){if(cdTimer)clearInterval(cdTimer);user=null;task=null;scanned=false;manual={};document.getElementById('user-chip').style.display='none';document.getElementById('flow-bar').classList.add('hidden');show('screen-lock')}
function initials(n){return n.split(' ').filter(Boolean).map(w=>w[0]).join('').slice(0,2).toUpperCase()}
function showWelcome(d){document.getElementById('welcome-name').textContent=d.name;document.getElementById('welcome-role').textContent=d.role;document.getElementById('welcome-avatar').textContent=initials(d.name);document.getElementById('welcome-time').textContent=fmt();document.getElementById('flow-bar').classList.remove('hidden');setFlow(0);show('screen-welcome')}
setInterval(()=>{fetch('/api/status').then(r=>r.json()).then(d=>{if(d.scanned){user={name:d.name,role:d.role,uid:d.uid};setUser(d.name,d.role);showWelcome(d)}}).catch(()=>{})},POLL);
function showMenu(){document.getElementById('menu-name').textContent=user.name;document.getElementById('menu-time').textContent=fmt();setFlow(1);show('screen-menu')}
function openTask(t){task=t;scanned=false;manual={};taskStartedAt=Date.now();const items=LISTS[t];states=items.map(()=>0);sources=items.map(()=>'m');const tag=document.getElementById('task-tag');tag.className='tag '+t;tag.textContent=LABELS[t];document.getElementById('task-title').textContent=LABELS[t];document.getElementById('task-staff').textContent=user.name+' · '+user.role;document.getElementById('scan-note').classList.remove('show');document.getElementById('scan-btn').disabled=false;document.getElementById('scan-btn').textContent='Scan workspace';setFlow(2);render();show('screen-task')}
function render(){const items=LISTS[task],el=document.getElementById('checklist');el.innerHTML='';items.forEach((it,i)=>{const on=states[i],auto=sources[i]==='a',mc=it.m&&scanned;let cls='check-row';if(on&&auto)cls+=' auto';else if(on)cls+=' done';if(mc&&!on&&manual[i]===undefined)cls+=' needs-input';const row=document.createElement('div');row.className=cls;let meta='';if(it.r)meta+='<span class="badge req">Required</span>';if(scanned&&it.d&&!it.m)meta+=on?'<span class="badge auto">Detected</span>':'<span class="badge miss">Not found</span>';if(scanned&&it.m)meta+='<span class="badge manual">Confirm</span>';let mcHtml='';if(mc){const y=manual[i]===1?' active':'',n=manual[i]===0?' active':'';mcHtml=`<div class="mc-row"><button class="mc yes${y}" onclick="setMc(${i},1,event)">Present</button><button class="mc no${n}" onclick="setMc(${i},0,event)">Missing</button></div>`}row.innerHTML=`<div class="cb"></div><div class="check-body"><div class="check-label">${it.l}</div><div class="check-meta">${meta}</div>${mcHtml}</div>`;if(!it.m)row.onclick=()=>toggle(i);el.appendChild(row)});updateProg()}
function toggle(i){states[i]=states[i]?0:1;sources[i]='m';render()}
function setMc(i,v,e){e.stopPropagation();manual[i]=v;states[i]=v;sources[i]='m';render()}
function updateProg(){const items=LISTS[task],total=items.length,done=states.filter(Boolean).length;const pending=items.filter((it,i)=>it.r&&(it.m?manual[i]===undefined:!states[i])).length;document.getElementById('prog-text').textContent=done+' / '+total;document.getElementById('prog-bar').style.width=Math.round(done/total*100)+'%';const btn=document.getElementById('cap-btn');if(!pending){btn.disabled=false;btn.textContent='Capture and continue'}else{btn.disabled=true;btn.textContent=pending+' required item'+(pending>1?'s':'')+' remaining'}}
function runScan(){document.getElementById('scan-btn').disabled=true;document.getElementById('scan-btn').textContent='Scanning…';document.getElementById('overlay').classList.add('on');setTimeout(()=>{document.getElementById('overlay').classList.remove('on');let n=0;scanned=true;LISTS[task].forEach((it,i)=>{if(it.m||!it.d)return;const f=Math.random()>.15;states[i]=f?1:0;sources[i]='a';if(f)n++});document.getElementById('scan-count').textContent=n;document.getElementById('scan-note').classList.add('show');document.getElementById('scan-btn').textContent='Scan complete';render()},2000)}
function capture(){if(document.getElementById('cap-btn').disabled)return;document.getElementById('cap-btn').textContent='Capturing…';fetch('/api/capture',{method:'POST'}).catch(()=>{});setTimeout(()=>{document.getElementById('remarks').value='';document.getElementById('serial').value='';countChars();setFlow(2);show('screen-remarks')},1000)}
function countChars(){document.getElementById('chars').textContent=document.getElementById('remarks').value.length}
function buildChecklist(){return LISTS[task].map((it,i)=>({label:it.l,checked:!!states[i],required:!!it.r,source:sources[i]||'m'}))}
async function finish(){const items=LISTS[task],rm=(document.getElementById('remarks').value||'').trim(),serial=(document.getElementById('serial').value||'').trim();if(!serial){alert('Enter the terminal serial number.');return}const auto=sources.filter(s=>s==='a').length,durationSec=taskStartedAt?Math.round((Date.now()-taskStartedAt)/1000):0;const endedAt=new Date().toISOString();const payload={terminal_id:serial,terminal_model:'PAX A920',terminal_brand:'PAX',staff_id:user.uid,staff_name:user.name,staff_role:user.role,station:'Bay 1',task:task,remarks:rm,duration_sec:durationSec,items_checked:states.filter(Boolean).length,items_total:items.length,camera_scan_count:auto,checklist:buildChecklist(),started_at:new Date(taskStartedAt).toISOString(),ended_at:endedAt};let syncText='Saving to cloud…';try{const r=await fetch('/api/transaction',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});const d=await r.json();syncText=d.synced?'Synced to supervisor dashboard':'Saved on station (cloud offline)'}catch(e){syncText='Saved on station (cloud offline)'}document.getElementById('summary').innerHTML=`<div class="row"><span>Staff</span><span>${user.name}</span></div><div class="row"><span>Role</span><span>${user.role}</span></div><hr><div class="row"><span>Serial</span><span>${serial}</span></div><div class="row"><span>Task</span><span>${LABELS[task]}</span></div><div class="row"><span>Items checked</span><span>${states.filter(Boolean).length} / ${items.length}</span></div><div class="row"><span>Duration</span><span>${durationSec}s</span></div><div class="row"><span>Camera scan</span><span>${auto} items</span></div><div class="row"><span>Cloud</span><span>${syncText}</span></div>${rm?`<hr><div class="row"><span>Remarks</span><span style="max-width:200px;font-size:13px">${rm}</span></div>`:''}<hr><div class="row"><span>Time</span><span>${fmt()}</span></div>`;setFlow(3);show('screen-confirm');let s=5;document.getElementById('cd').textContent=s;if(cdTimer)clearInterval(cdTimer);cdTimer=setInterval(()=>{s--;document.getElementById('cd').textContent=s;if(s<=0){clearInterval(cdTimer);resetToLock()}},1000)}
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
