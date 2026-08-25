#!/usr/bin/env python3
"""Create a machine-readable summary from one cserver/client run."""

import argparse
import json
import re
from collections import Counter
from pathlib import Path


SCORE_RE = re.compile(r"(?:#\s*score|score\s+is)\s*:\s*(-?\d+)", re.IGNORECASE)
PLATFORM_ACTION_RE = re.compile(r"^\s*\[([A-Za-z_]+)(?:\s[^|]*)?\|", re.MULTILINE)
LEGACY_ACTION_RE = re.compile(
    r"Executing\s+the\s+action\s*:\s*([A-Za-z_]+)", re.IGNORECASE
)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--run-dir", required=True, type=Path)
    parser.add_argument("--stage", required=True, type=int)
    parser.add_argument("--mode", required=True, choices=("it", "nt"))
    parser.add_argument("--case", required=True)
    parser.add_argument("--client", required=True)
    parser.add_argument("--duration-ms", required=True, type=int)
    parser.add_argument("--server-exit", required=True, type=int)
    parser.add_argument("--client-exit", required=True, type=int)
    parser.add_argument("--server-stopped-by-runner", action="store_true")
    return parser.parse_args()


def read_logs(run_dir):
    chunks = []
    for path in sorted(run_dir.rglob("*")):
        if path.is_file() and path.name != "summary.json":
            try:
                chunks.append(path.read_text(encoding="utf-8", errors="replace"))
            except OSError:
                pass
    return "\n".join(chunks)


def build_summary(text, stage, mode, case_id, client, duration_ms,
                  server_exit, client_exit, server_stopped_by_runner=False,
                  action_text=None):
    scores = [int(value) for value in SCORE_RE.findall(text)]
    raw_score = scores[-1] if scores else None
    official_score = min(raw_score, 1000) if raw_score is not None else None
    action_source = text if action_text is None else action_text
    action_names = PLATFORM_ACTION_RE.findall(action_source)
    if not action_names:
        action_names = LEGACY_ACTION_RE.findall(action_source)
    actions = Counter(name.lower() for name in action_names)

    return {
        "stage": stage,
        "mode": mode,
        "case": case_id,
        "client": client,
        "duration_ms": duration_ms,
        "server_exit": server_exit,
        "client_exit": client_exit,
        "server_stopped_by_runner": server_stopped_by_runner,
        "timed_out": client_exit == 124 or (
            server_exit == 124 and not server_stopped_by_runner
        ),
        "raw_score": raw_score,
        "official_score": official_score,
        "action_count": sum(actions.values()),
        "actions": dict(sorted(actions.items())),
    }


def main():
    args = parse_args()
    text = read_logs(args.run_dir)
    platform_text = read_logs(args.run_dir / "platform-log")
    summary = build_summary(
        text=text,
        stage=args.stage,
        mode=args.mode,
        case_id=args.case,
        client=args.client,
        duration_ms=args.duration_ms,
        server_exit=args.server_exit,
        client_exit=args.client_exit,
        server_stopped_by_runner=args.server_stopped_by_runner,
        action_text=platform_text or text,
    )
    output = args.run_dir / "summary.json"
    output.write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, ensure_ascii=False))


if __name__ == "__main__":
    main()
