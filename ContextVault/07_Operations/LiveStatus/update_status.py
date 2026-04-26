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
