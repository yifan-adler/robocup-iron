#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $0 --stage 1|2 --mode it|nt --case ID [--client official|iron] [--tests-dir PATH] [--timeout SECONDS]" >&2
}

stage=""
mode=""
case_id=""
client="iron"
tests_dir=""
timeout_seconds="60"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --stage) stage="${2:-}"; shift 2 ;;
    --mode) mode="${2:-}"; shift 2 ;;
    --case) case_id="${2:-}"; shift 2 ;;
    --client) client="${2:-}"; shift 2 ;;
    --tests-dir) tests_dir="${2:-}"; shift 2 ;;
    --timeout) timeout_seconds="${2:-}"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done

if [[ "$stage" != "1" && "$stage" != "2" ]] \
  || [[ "$mode" != "it" && "$mode" != "nt" ]] \
  || [[ -z "$case_id" ]] \
  || [[ "$client" != "official" && "$client" != "iron" ]] \
  || ! [[ "$timeout_seconds" =~ ^[1-9][0-9]*$ ]]; then
  usage
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode_config="$repo_root/config/modes/stage${stage}-${mode}.env"
# shellcheck disable=SC1090
source "$mode_config"

platform="$repo_root/.work/platform-$client"
if [[ ! -x "$platform/bin/example" || ! -x "$platform/bin/cserver" ]]; then
  echo "ERROR: $client is not built; run scripts/build.sh --client $client first" >&2
  exit 1
fi

if [[ -z "$tests_dir" ]]; then
  upper_mode="$(printf '%s' "$mode" | tr '[:lower:]' '[:upper:]')"
  tests_dir="$platform/tests/problems/$upper_mode/stage$stage"
elif [[ "$tests_dir" != /* ]]; then
  tests_dir="$repo_root/$tests_dir"
fi

if [[ ! -f "$tests_dir/$case_id.xml" ]]; then
  echo "ERROR: test case not found: $tests_dir/$case_id.xml" >&2
  exit 1
fi

run_id="$(date -u +%Y%m%dT%H%M%SZ)-s${stage}-${mode}-${case_id}-${client}-$$"
run_dir="$repo_root/artifacts/runs/$run_id"
mkdir -p "$run_dir/platform-log"

cp -p "$platform/res/iclingo" "$platform/bin/iclingo"
cp -p "$platform"/res/*.lp "$platform/bin/"
chmod +x "$platform/bin/cserver" "$platform/bin/example" "$platform/bin/iclingo" || true

server_pid=""
cleanup() {
  if [[ -n "$server_pid" ]] && kill -0 "$server_pid" 2>/dev/null; then
    kill "$server_pid" 2>/dev/null || true
    wait "$server_pid" 2>/dev/null || true
  fi
}
trap cleanup EXIT INT TERM

start_ns="$(date +%s%N)"
set +e
(
  cd "$platform/bin"
  exec ./cserver \
    -td "$tests_dir" \
    -eval "$platform/lib/libasp" \
    -log "$run_dir/platform-log" \
    -mode "$SERVER_MODE" \
    -test "$case_id"
) > "$run_dir/server.log" 2>&1 &
server_pid=$!

sleep 1
(
  cd "$platform/bin"
  timeout --signal=TERM "$timeout_seconds" \
    ./example \
      -nlp "$IRON_NLP" \
      -err "$IRON_ERROR_CORRECTION" \
      -ask_2 "$IRON_ASK_TWICE" \
      -stage "$IRON_STAGE" \
      -path "$platform/example/words.txt"
) > "$run_dir/client.log" 2>&1
client_exit=$?

server_stopped_by_runner=false
if kill -0 "$server_pid" 2>/dev/null; then
  server_stopped_by_runner=true
  kill "$server_pid" 2>/dev/null || true
fi
wait "$server_pid"
server_exit=$?
server_pid=""
set -e

end_ns="$(date +%s%N)"
duration_ms=$(( (end_ns - start_ns) / 1000000 ))

summary_args=(
  --run-dir "$run_dir"
  --stage "$stage"
  --mode "$mode"
  --case "$case_id"
  --client "$client"
  --duration-ms "$duration_ms"
  --server-exit "$server_exit"
  --client-exit "$client_exit"
)
if [[ "$server_stopped_by_runner" == true ]]; then
  summary_args+=(--server-stopped-by-runner)
fi
python3 "$repo_root/tools/summarize_run.py" "${summary_args[@]}"

echo "Run artifacts: $run_dir"
summary_has_score="$(python3 -c 'import json,sys; print("yes" if json.load(open(sys.argv[1], encoding="utf-8"))["raw_score"] is not None else "no")' "$run_dir/summary.json")"
if [[ $client_exit -ne 0 ]] \
  || [[ "$server_stopped_by_runner" == false && $server_exit -ne 0 ]] \
  || ! grep -q '^# Result:' "$run_dir/server.log" \
  || [[ "$summary_has_score" != "yes" ]]; then
  exit 1
fi
