#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
errors=0

check_command() {
  if command -v "$1" >/dev/null 2>&1; then
    echo "OK: $1 -> $(command -v "$1")"
  else
    echo "ERROR: missing command: $1" >&2
    errors=$((errors + 1))
  fi
}

check_command docker
check_command git

if command -v docker >/dev/null 2>&1; then
  docker version >/dev/null 2>&1 || {
    echo "ERROR: Docker CLI exists but the daemon is unavailable" >&2
    errors=$((errors + 1))
  }
  docker compose version >/dev/null 2>&1 || {
    echo "ERROR: Docker Compose plugin is unavailable" >&2
    errors=$((errors + 1))
  }
fi

archive="$repo_root/archive/legacy-2025/代码/Planner-release-2025(1).zip"
if [[ -f "$archive" ]]; then
  actual="$(sha256sum "$archive" 2>/dev/null | awk '{print $1}' || true)"
  if [[ "$actual" == "fdb9cf54054aaac0ccbfbeabc97a99d0067dc975f0999a90f81fe83a326ac150" ]]; then
    echo "OK: official platform archive SHA256"
  else
    echo "ERROR: official platform archive is missing or changed" >&2
    errors=$((errors + 1))
  fi
else
  echo "ERROR: official platform archive not found" >&2
  errors=$((errors + 1))
fi

if [[ $errors -ne 0 ]]; then
  echo "Doctor found $errors problem(s)" >&2
  exit 1
fi
echo "Host prerequisites look ready"
