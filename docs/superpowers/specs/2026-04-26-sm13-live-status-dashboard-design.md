# SM-13 Local Live Status Dashboard Design

Date: 2026-04-26

## Goal

Build a small localhost-only, read-only dashboard so the user can quickly see whether the ContextVault Agent Company loop is working, validating, or waiting for the next heartbeat.

## Scope

First version is intentionally simple:

- Serve a local HTML dashboard bound to `127.0.0.1`.
- Refresh every 5 seconds.
- Display a concise status summary from `ContextVault/07_Operations/LiveStatus/status.json`.
- Keep ContextVault markdown files as the durable evidence source.
- Do not expose write controls, command triggers, or network access beyond localhost.

Out of scope for v1:

- WebSocket streaming.
- LAN or internet sharing.
- Authentication.
- Buttons that run agents, builds, tests, git, or editor actions.
- Parsing every ContextVault markdown file into a full analytics UI.

## Architecture

Files live under `ContextVault/07_Operations/LiveStatus/`:

- `index.html` renders the dashboard and polls `status.json` every 5 seconds.
- `status.json` stores the current operational snapshot.
- `server.py` serves the directory through Python's standard library on `127.0.0.1`.
- `update_status.py` updates `status.json` safely from command-line arguments.

This keeps the system dependency-light and easy to inspect. Python is already available in the development environment more often than a Node stack, and the repo has no existing `package.json`.

## Status JSON Contract

`status.json` contains:

```json
{
  "state": "working | validating | idle | blocked | failed",
  "packet": "SM13-...",
  "phase": "short human-readable phase",
  "summary": "one-line current status",
  "lastUpdated": "ISO-8601 local timestamp",
  "nextHeartbeat": "human-readable next loop time or empty",
  "validation": {
    "build": "pending | passed | failed | not-run",
    "focused": "pending | passed | failed | not-run",
    "regression": "pending | passed | failed | not-run"
  },
  "events": [
    { "time": "HH:MM", "level": "info | pass | warn | fail", "message": "..." }
  ]
}
```

The dashboard treats missing fields as unknown and keeps rendering. This prevents a partial status update from breaking visibility.

## Dashboard Layout

The page has four simple sections:

1. Big status banner: current state, packet, phase, last update age.
2. Validation row: build, focused automation, full regression.
3. Current summary and next heartbeat.
4. Recent event log, newest first.

The browser refreshes only data, not the full page, so it stays readable during long-running work.

## Update Workflow

During Agent Company work, Claude updates `status.json` at key moments:

- packet selected;
- scouts launched or completed;
- build started/passed/failed;
- focused automation started/passed/failed;
- full regression started/passed/failed;
- ContextVault evidence written;
- next loop heartbeat scheduled.

These updates are operational visibility only. The authoritative completion record remains in `WarStudioStatus.md`, `ConcurrencyBoard.md`, and `99_EventBus.md`.

## Validation

Validation steps:

1. Confirm `status.json` is valid JSON.
2. Start `server.py` bound to `127.0.0.1`.
3. Fetch `/status.json` locally.
4. Fetch `/index.html` locally.
5. Run `update_status.py` once and confirm the JSON changes.

## Safety

The server only serves static files and JSON from the LiveStatus directory. It binds to `127.0.0.1` by default. There are no POST endpoints, shell execution endpoints, or dashboard buttons.
