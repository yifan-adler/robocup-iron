#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $0 --manifest PATH" >&2
}

manifest=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --manifest) manifest="${2:-}"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done

if [[ -z "$manifest" ]]; then
  usage
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ "$manifest" != /* ]]; then
  manifest="$repo_root/$manifest"
fi
if [[ ! -f "$manifest" ]]; then
  echo "ERROR: manifest not found: $manifest" >&2
  exit 1
fi

failures=0
while IFS=, read -r stage mode case_id client tests_dir timeout_seconds; do
  stage="${stage//$'\r'/}"
  [[ -z "$stage" || "$stage" == "stage" || "$stage" == \#* ]] && continue
  args=(--stage "$stage" --mode "$mode" --case "$case_id" --client "${client:-iron}" --timeout "${timeout_seconds:-60}")
  if [[ -n "$tests_dir" ]]; then
    args+=(--tests-dir "$tests_dir")
  fi
  if ! "$repo_root/scripts/run_case.sh" "${args[@]}"; then
    failures=$((failures + 1))
  fi
done < "$manifest"

if [[ $failures -ne 0 ]]; then
  echo "Suite completed with $failures failed case(s)" >&2
  exit 1
fi
echo "Suite completed successfully"
