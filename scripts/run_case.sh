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

test_list="$tests_dir/test.list"
if [[ ! -f "$test_list" ]]; then
  echo "ERROR: test list not found: $test_list" >&2
  exit 1
fi
server_test_index="$(awk -v target="$case_id.xml" '
  { sub(/\r$/, "") }
  $0 == target { print NR; exit }
' "$test_list")"
if [[ -z "$server_test_index" ]]; then
  echo "ERROR: test case is not listed in $test_list: $case_id.xml" >&2
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
    for _ in {1..20}; do
      kill -0 "$server_pid" 2>/dev/null || break
      sleep 0.1
    done
    if kill -0 "$server_pid" 2>/dev/null; then
      kill -KILL "$server_pid" 2>/dev/null || true
    fi
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
      -test "$server_test_index"
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

# The official cserver deliberately loops back to accept another team after a
# completed run. Once the client receives <quit/> and exits, stop that listener
# instead of waiting for the outer case timeout and reporting a false failure.
if kill -0 "$server_pid" 2>/dev/null; then
  kill "$server_pid" 2>/dev/null || true
  for _ in {1..20}; do
    kill -0 "$server_pid" 2>/dev/null || break
    sleep 0.1
  done
  if kill -0 "$server_pid" 2>/dev/null; then
    kill -KILL "$server_pid" 2>/dev/null || true
  fi
  wait "$server_pid" 2>/dev/null || true
  server_exit=0
else
  wait "$server_pid"
  server_exit=$?
fi
server_pid=""
set -e

end_ns="$(date +%s%N)"
duration_ms=$(( (end_ns - start_ns) / 1000000 ))

python3 "$repo_root/tools/summarize_run.py" \
  --run-dir "$run_dir" \
  --stage "$stage" \
  --mode "$mode" \
  --case "$case_id" \
  --client "$client" \
  --duration-ms "$duration_ms" \
  --server-exit "$server_exit" \
  --client-exit "$client_exit"

echo "Run artifacts: $run_dir"
if [[ $server_exit -ne 0 || $client_exit -ne 0 ]]; then
  exit 1
fi
