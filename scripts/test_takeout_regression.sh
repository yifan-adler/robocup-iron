#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
runs="${TAKEOUT_REGRESSION_RUNS:-20}"

if ! [[ "$runs" =~ ^[1-9][0-9]*$ ]] || (( runs > 100 )); then
  echo "ERROR: TAKEOUT_REGRESSION_RUNS must be an integer from 1 to 100" >&2
  exit 2
fi

for ((iteration = 1; iteration <= runs; ++iteration)); do
  output="$("$repo_root/scripts/run_case.sh" \
    --stage 2 \
    --mode it \
    --case 01 \
    --client iron \
    --tests-dir tests/regressions/takeout-misleading \
    --timeout 60)"
  printf '%s\n' "$output"

  run_dir="$(printf '%s\n' "$output" | awk '/^Run artifacts:/ { print $3 }' | tail -n 1)"
  if [[ -z "$run_dir" || ! -f "$run_dir/summary.json" ]]; then
    echo "ERROR: regression run $iteration did not produce summary.json" >&2
    exit 1
  fi

  python3 - "$run_dir/summary.json" "$iteration" <<'PY'
import json
import sys

summary_path = sys.argv[1]
iteration = sys.argv[2]
with open(summary_path, "r") as handle:
    summary = json.load(handle)

takeout_count = summary.get("actions", {}).get("takeout", 0)
if takeout_count < 1:
    raise SystemExit(
        "ERROR: regression run %s marked the task complete without TakeOut (%s)"
        % (iteration, summary_path)
    )
PY
done

echo "Takeout regression passed: $runs/$runs runs executed TakeOut"
