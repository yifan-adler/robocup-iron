#!/usr/bin/env python3
"""Create a machine-readable summary from one cserver/client run."""

import argparse
import json
import re
from collections import Counter
from pathlib import Path


SCORE_RE = re.compile(r"\bscore(?:\s+is)?\s*:\s*(-?\d+)", re.IGNORECASE)
ACTION_RE = re.compile(r"Executing\s+the\s+action\s*:\s*([A-Za-z_]+)", re.IGNORECASE)
SERVER_ACTION_RE = re.compile(r"^\s*\[([A-Za-z_][A-Za-z0-9_]*)\b", re.MULTILINE)


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


def main():
    args = parse_args()
    text = read_logs(args.run_dir)
    scores = [int(value) for value in SCORE_RE.findall(text)]
    raw_score = scores[-1] if scores else None
    official_score = min(raw_score, 1000) if raw_score is not None else None
    try:
        server_text = (args.run_dir / "server.log").read_text(encoding="utf-8", errors="replace")
    except OSError:
        server_text = ""
    action_names = SERVER_ACTION_RE.findall(server_text) or ACTION_RE.findall(text)
    actions = Counter(name.lower() for name in action_names)

    summary = {
        "stage": args.stage,
        "mode": args.mode,
        "case": args.case,
        "client": args.client,
        "duration_ms": args.duration_ms,
        "server_exit": args.server_exit,
        "client_exit": args.client_exit,
        "timed_out": args.server_exit == 124 or args.client_exit == 124,
        "raw_score": raw_score,
        "official_score": official_score,
        "action_count": sum(actions.values()),
        "actions": dict(sorted(actions.items())),
    }
    output = args.run_dir / "summary.json"
    output.write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, ensure_ascii=False))


if __name__ == "__main__":
    main()
