# SM-13 Live Status Dashboard Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a localhost-only, read-only dashboard that shows current SM-13 Agent Company status from a simple JSON file and refreshes every 5 seconds.

**Architecture:** Create a small self-contained LiveStatus directory under ContextVault. `status.json` is the visible operational snapshot, `index.html` polls it, `server.py` serves static files on `127.0.0.1`, and `update_status.py` safely updates the snapshot from CLI arguments.

**Tech Stack:** Python 3 standard library, browser HTML/CSS/JavaScript, JSON, ContextVault markdown evidence remains authoritative.

---

## File Structure

- Create `ContextVault/07_Operations/LiveStatus/status.json`
  - Stores the current visible dashboard snapshot.
- Create `ContextVault/07_Operations/LiveStatus/index.html`
  - Renders the dashboard and fetches `status.json` every 5 seconds.
- Create `ContextVault/07_Operations/LiveStatus/server.py`
  - Serves only the LiveStatus directory on `127.0.0.1`.
- Create `ContextVault/07_Operations/LiveStatus/update_status.py`
  - Merges CLI updates into `status.json`, stamps `lastUpdated`, and prepends a recent event.

---

### Task 1: Seed Status JSON

**Files:**
- Create: `ContextVault/07_Operations/LiveStatus/status.json`

- [ ] **Step 1: Create the initial status snapshot**

Write this exact JSON:

```json
{
  "state": "idle",
  "packet": "SM13-20260426-LIVESTATUS",
  "phase": "Dashboard bootstrap",
  "summary": "Local live status dashboard is being prepared.",
  "lastUpdated": "2026-04-26T00:00:00",
  "nextHeartbeat": "Scheduled by Claude loop",
  "validation": {
    "build": "not-run",
    "focused": "not-run",
    "regression": "not-run"
  },
  "events": [
    {
      "time": "00:00",
      "level": "info",
      "message": "Live status dashboard status file created."
    }
  ]
}
```

- [ ] **Step 2: Validate JSON**

Run:

```bash
python -m json.tool "ContextVault/07_Operations/LiveStatus/status.json"
```

Expected: command exits 0 and prints formatted JSON.

---

### Task 2: Build the Read-Only Dashboard Page

**Files:**
- Create: `ContextVault/07_Operations/LiveStatus/index.html`

- [ ] **Step 1: Create the dashboard HTML**

Write this exact file:

```html
<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SM-13 Live Status</title>
  <style>
    :root {
      color-scheme: dark;
      --bg: #08111f;
      --panel: #101b2d;
      --panel-2: #162338;
      --text: #edf4ff;
      --muted: #91a3bb;
      --line: #27364d;
      --info: #62a8ff;
      --pass: #5ee0a0;
      --warn: #ffd166;
      --fail: #ff6b6b;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Segoe UI", system-ui, sans-serif;
      background: radial-gradient(circle at top left, #132540, var(--bg) 45%);
      color: var(--text);
    }
    main {
      width: min(1100px, calc(100vw - 32px));
      margin: 24px auto;
    }
    header {
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      gap: 16px;
      margin-bottom: 18px;
    }
    h1 { margin: 0; font-size: 28px; }
    .subtitle { margin-top: 6px; color: var(--muted); }
    .pill {
      border: 1px solid var(--line);
      background: rgba(16, 27, 45, 0.82);
      border-radius: 999px;
      padding: 8px 12px;
      color: var(--muted);
      white-space: nowrap;
    }
    .banner {
      border: 1px solid var(--line);
      border-radius: 20px;
      padding: 22px;
      background: linear-gradient(135deg, rgba(98,168,255,.18), rgba(16,27,45,.94));
      box-shadow: 0 20px 60px rgba(0,0,0,.26);
      margin-bottom: 16px;
    }
    .state {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      font-size: 14px;
      text-transform: uppercase;
      letter-spacing: .12em;
      color: var(--muted);
    }
    .dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--info);
      box-shadow: 0 0 18px var(--info);
    }
    .dot.pass { background: var(--pass); box-shadow: 0 0 18px var(--pass); }
    .dot.warn { background: var(--warn); box-shadow: 0 0 18px var(--warn); }
    .dot.fail { background: var(--fail); box-shadow: 0 0 18px var(--fail); }
    .packet { margin: 14px 0 8px; font-size: 30px; font-weight: 700; }
    .phase { color: var(--muted); font-size: 18px; }
    .grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 14px;
      margin: 16px 0;
    }
    .card {
      border: 1px solid var(--line);
      border-radius: 16px;
      background: rgba(16, 27, 45, 0.86);
      padding: 16px;
    }
    .label { color: var(--muted); font-size: 13px; margin-bottom: 8px; }
    .value { font-size: 18px; font-weight: 650; }
    .events { display: grid; gap: 10px; }
    .event {
      border: 1px solid var(--line);
      border-radius: 14px;
      padding: 12px 14px;
      background: rgba(22, 35, 56, 0.72);
      display: grid;
      grid-template-columns: 70px 70px 1fr;
      gap: 10px;
      align-items: center;
    }
    .level { font-weight: 700; }
    .level.pass { color: var(--pass); }
    .level.warn { color: var(--warn); }
    .level.fail { color: var(--fail); }
    .level.info { color: var(--info); }
    @media (max-width: 760px) {
      header { display: block; }
      .pill { display: inline-block; margin-top: 12px; }
      .grid { grid-template-columns: 1fr; }
      .event { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <main>
    <header>
      <div>
        <h1>SM-13 Agent Company Live Status</h1>
        <div class="subtitle">localhost-only · read-only · refreshes every 5 seconds</div>
      </div>
      <div class="pill" id="last-updated">Last update: loading</div>
    </header>

    <section class="banner">
      <div class="state"><span id="state-dot" class="dot"></span><span id="state">loading</span></div>
      <div class="packet" id="packet">Loading status...</div>
      <div class="phase" id="phase">Waiting for status.json</div>
    </section>

    <section class="grid">
      <div class="card"><div class="label">Build</div><div class="value" id="build">unknown</div></div>
      <div class="card"><div class="label">Focused automation</div><div class="value" id="focused">unknown</div></div>
      <div class="card"><div class="label">Full regression</div><div class="value" id="regression">unknown</div></div>
    </section>

    <section class="grid">
      <div class="card" style="grid-column: span 2;"><div class="label">Current summary</div><div class="value" id="summary">loading</div></div>
      <div class="card"><div class="label">Next heartbeat</div><div class="value" id="next-heartbeat">unknown</div></div>
    </section>

    <section class="card">
      <div class="label">Recent events</div>
      <div class="events" id="events"></div>
    </section>
  </main>

  <script>
    const stateClass = (state) => {
      if (["idle"].includes(state)) return "pass";
      if (["blocked"].includes(state)) return "warn";
      if (["failed"].includes(state)) return "fail";
      return "info";
    };

    const setText = (id, value) => {
      document.getElementById(id).textContent = value || "unknown";
    };

    async function refreshStatus() {
      try {
        const response = await fetch(`status.json?ts=${Date.now()}`, { cache: "no-store" });
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        const data = await response.json();
        const state = data.state || "unknown";
        const dot = document.getElementById("state-dot");
        dot.className = `dot ${stateClass(state)}`;
        setText("state", state);
        setText("packet", data.packet);
        setText("phase", data.phase);
        setText("summary", data.summary);
        setText("next-heartbeat", data.nextHeartbeat);
        setText("last-updated", `Last update: ${data.lastUpdated || "unknown"}`);
        setText("build", data.validation?.build);
        setText("focused", data.validation?.focused);
        setText("regression", data.validation?.regression);

        const events = document.getElementById("events");
        events.innerHTML = "";
        (data.events || []).slice(0, 5).forEach((event) => {
          const row = document.createElement("div");
          row.className = "event";
          row.innerHTML = `<span>${event.time || "--:--"}</span><span class="level ${event.level || "info"}">${event.level || "info"}</span><span>${event.message || ""}</span>`;
          events.appendChild(row);
        });
      } catch (error) {
        setText("state", "failed");
        setText("packet", "Unable to read status.json");
        setText("phase", error.message);
        document.getElementById("state-dot").className = "dot fail";
      }
    }

    refreshStatus();
    setInterval(refreshStatus, 5000);
  </script>
</body>
</html>
```

- [ ] **Step 2: Confirm the file exists**

Run:

```bash
python - <<'PY'
from pathlib import Path
p = Path('ContextVault/07_Operations/LiveStatus/index.html')
print(p.exists(), p.stat().st_size)
PY
```

Expected: prints `True` and a positive byte count.

---

### Task 3: Add Localhost Static Server

**Files:**
- Create: `ContextVault/07_Operations/LiveStatus/server.py`

- [ ] **Step 1: Create the server script**

Write this exact file:

```python
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

HOST = "127.0.0.1"
PORT = 8765
ROOT = Path(__file__).resolve().parent


def main() -> None:
    handler = partial(SimpleHTTPRequestHandler, directory=str(ROOT))
    server = ThreadingHTTPServer((HOST, PORT), handler)
    print(f"SM-13 Live Status dashboard: http://{HOST}:{PORT}/index.html")
    print(f"Serving read-only files from: {ROOT}")
    server.serve_forever()


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Syntax-check the server**

Run:

```bash
python -m py_compile "ContextVault/07_Operations/LiveStatus/server.py"
```

Expected: exit 0 with no output.

---

### Task 4: Add Safe Status Updater

**Files:**
- Create: `ContextVault/07_Operations/LiveStatus/update_status.py`

- [ ] **Step 1: Create the updater script**

Write this exact file:

```python
import argparse
import json
import os
import tempfile
from datetime import datetime
from pathlib import Path

STATUS_PATH = Path(__file__).resolve().parent / "status.json"
VALID_STATES = {"working", "validating", "idle", "blocked", "failed"}
VALID_LEVELS = {"info", "pass", "warn", "fail"}
VALID_RESULTS = {"pending", "passed", "failed", "not-run"}


def load_status() -> dict:
    if not STATUS_PATH.exists():
        return {
            "state": "idle",
            "packet": "SM13",
            "phase": "Status bootstrap",
            "summary": "Status file created by update_status.py.",
            "lastUpdated": "",
            "nextHeartbeat": "",
            "validation": {"build": "not-run", "focused": "not-run", "regression": "not-run"},
            "events": [],
        }
    with STATUS_PATH.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def atomic_write(data: dict) -> None:
    STATUS_PATH.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_path = tempfile.mkstemp(prefix="status.", suffix=".json", dir=STATUS_PATH.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as handle:
            json.dump(data, handle, ensure_ascii=False, indent=2)
            handle.write("\n")
        os.replace(temp_path, STATUS_PATH)
    finally:
        if os.path.exists(temp_path):
            os.remove(temp_path)


def main() -> None:
    parser = argparse.ArgumentParser(description="Update SM-13 live status dashboard JSON.")
    parser.add_argument("--state", choices=sorted(VALID_STATES))
    parser.add_argument("--packet")
    parser.add_argument("--phase")
    parser.add_argument("--summary")
    parser.add_argument("--next-heartbeat")
    parser.add_argument("--build", choices=sorted(VALID_RESULTS))
    parser.add_argument("--focused", choices=sorted(VALID_RESULTS))
    parser.add_argument("--regression", choices=sorted(VALID_RESULTS))
    parser.add_argument("--event")
    parser.add_argument("--level", choices=sorted(VALID_LEVELS), default="info")
    args = parser.parse_args()

    status = load_status()
    status.setdefault("validation", {})
    status.setdefault("events", [])

    for key in ("state", "packet", "phase", "summary"):
        value = getattr(args, key)
        if value is not None:
            status[key] = value

    if args.next_heartbeat is not None:
        status["nextHeartbeat"] = args.next_heartbeat

    if args.build is not None:
        status["validation"]["build"] = args.build
    if args.focused is not None:
        status["validation"]["focused"] = args.focused
    if args.regression is not None:
        status["validation"]["regression"] = args.regression

    now = datetime.now().replace(microsecond=0)
    status["lastUpdated"] = now.isoformat()

    if args.event:
        status["events"] = [
            {"time": now.strftime("%H:%M"), "level": args.level, "message": args.event},
            *status["events"],
        ][:8]

    atomic_write(status)


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Syntax-check the updater**

Run:

```bash
python -m py_compile "ContextVault/07_Operations/LiveStatus/update_status.py"
```

Expected: exit 0 with no output.

- [ ] **Step 3: Update status once**

Run:

```bash
python "ContextVault/07_Operations/LiveStatus/update_status.py" --state working --packet SM13-20260426-LIVESTATUS --phase "Implementation" --summary "Local dashboard files are being implemented." --build not-run --focused not-run --regression not-run --event "Dashboard implementation started." --level info
```

Expected: exit 0.

- [ ] **Step 4: Validate updated JSON**

Run:

```bash
python -m json.tool "ContextVault/07_Operations/LiveStatus/status.json"
```

Expected: formatted JSON includes `SM13-20260426-LIVESTATUS` and `Dashboard implementation started.`

---

### Task 5: Validate Local Server

**Files:**
- Modify: none unless validation reveals a defect.

- [ ] **Step 1: Start the server in background**

Run:

```bash
python "ContextVault/07_Operations/LiveStatus/server.py"
```

Expected: prints `http://127.0.0.1:8765/index.html` and keeps running.

- [ ] **Step 2: Fetch status JSON locally**

Run in another shell/tool call while the server is running:

```bash
python - <<'PY'
from urllib.request import urlopen
print(urlopen('http://127.0.0.1:8765/status.json', timeout=5).status)
PY
```

Expected: `200`.

- [ ] **Step 3: Fetch dashboard HTML locally**

Run:

```bash
python - <<'PY'
from urllib.request import urlopen
text = urlopen('http://127.0.0.1:8765/index.html', timeout=5).read().decode('utf-8')
print('SM-13 Agent Company Live Status' in text)
PY
```

Expected: `True`.

- [ ] **Step 4: Update status to idle after validation**

Run:

```bash
python "ContextVault/07_Operations/LiveStatus/update_status.py" --state idle --phase "Ready" --summary "Local live status dashboard is ready at http://127.0.0.1:8765/index.html" --event "Dashboard validation passed." --level pass
```

Expected: exit 0 and dashboard updates within 5 seconds.

---

### Task 6: Record ContextVault Evidence

**Files:**
- Modify: `ContextVault/07_Operations/WarStudioStatus.md`
- Modify: `ContextVault/07_Operations/ConcurrencyBoard.md`
- Modify: `ContextVault/99_EventBus.md`

- [ ] **Step 1: Update WarStudioStatus active packet list**

Add one packet after the latest active packet:

```markdown
42. Local live status dashboard — localhost-only read-only dashboard created and validated 2026-04-26
```

- [ ] **Step 2: Update ConcurrencyBoard active board**

Add a row near other passed packets:

```markdown
| SM13-20260426-LIVESTATUS | Operations / Visibility | FORGE + CURATOR | passed | `ContextVault/07_Operations/LiveStatus/**` | JSON validation exit 0; server status fetch exit 0; dashboard fetch exit 0 | localhost-only read-only dashboard refreshes status.json every 5 seconds |
```

- [ ] **Step 3: Append EventBus validation event**

Add before the closing code fence:

```yaml
- timestamp: 2026-04-26T12:10:00
  event: Event.Validation.SM13.LiveStatusDashboard.Passed
  from: REAPER
  payload:
    subject: Local read-only status dashboard is available for Agent Company visibility
    dashboard: http://127.0.0.1:8765/index.html
    scope:
      - status.json operational snapshot
      - index.html five-second polling dashboard
      - server.py localhost-only static server
      - update_status.py atomic status updater
    validation:
      - status JSON parsed successfully
      - server returned status.json over localhost
      - server returned dashboard HTML over localhost
    safety:
      - localhost binding only
      - no POST endpoints
      - no command buttons
      - no LAN exposure
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, CURATOR]
```

---

## Self-Review

Spec coverage:

- Localhost-only server: Task 3 and Task 5.
- Five-second refresh: Task 2 JavaScript `setInterval(refreshStatus, 5000)`.
- `status.json` snapshot: Task 1 and Task 4.
- No controls or write endpoints: Task 2 contains no buttons; Task 3 only uses `SimpleHTTPRequestHandler`.
- Validation: Task 1, Task 3, Task 4, Task 5.
- ContextVault evidence: Task 6.

Placeholder scan: no TBD/TODO placeholders remain.

Type consistency: JSON keys used by `index.html` match keys written by `status.json` and `update_status.py`.
