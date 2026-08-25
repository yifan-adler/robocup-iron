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

if [[ -r /etc/os-release ]]; then
  # shellcheck disable=SC1091
  source /etc/os-release
  if [[ "${ID:-}" == "ubuntu" && "${VERSION_ID:-}" == "18.04" ]]; then
    echo "OK: operating system -> ${PRETTY_NAME:-Ubuntu 18.04}"
  else
    echo "ERROR: Ubuntu 18.04 is required; found ${PRETTY_NAME:-unknown}" >&2
    errors=$((errors + 1))
  fi
else
  echo "ERROR: /etc/os-release is unavailable" >&2
  errors=$((errors + 1))
fi

if [[ "$(uname -m)" == "x86_64" ]]; then
  echo "OK: architecture -> x86_64"
else
  echo "ERROR: x86_64 is required; found $(uname -m)" >&2
  errors=$((errors + 1))
fi

if uname -r | grep -Eqi '(microsoft-standard|wsl2)'; then
  echo "OK: WSL2 kernel -> $(uname -r)"
else
  echo "ERROR: WSL2 is required; kernel is $(uname -r)" >&2
  errors=$((errors + 1))
fi

for command_name in bash dos2unix git gcc g++ make cmake patch python3 sha256sum timeout unzip dpkg-query; do
  check_command "$command_name"
done

if dpkg-query -W -f='${Version}\n' libboost-dev >/dev/null 2>&1; then
  echo "OK: libboost-dev -> $(dpkg-query -W -f='${Version}' libboost-dev)"
else
  echo "ERROR: missing package: libboost-dev" >&2
  errors=$((errors + 1))
fi

for script in bootstrap.sh build.sh doctor.sh onboard.sh run_case.sh run_suite.sh setup-ubuntu18.sh; do
  if [[ -x "$repo_root/scripts/$script" ]]; then
    echo "OK: executable script -> scripts/$script"
  else
    echo "ERROR: script is not executable: scripts/$script" >&2
    errors=$((errors + 1))
  fi
  if grep -q $'\r' "$repo_root/scripts/$script"; then
    echo "ERROR: CRLF detected in scripts/$script" >&2
    errors=$((errors + 1))
  fi
done

case "$repo_root" in
  /mnt/*)
    echo "WARNING: repository is under /mnt; use /home/<user>/robocup-iron for regular builds" >&2
    ;;
esac

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
echo "WSL2 Ubuntu 18.04 prerequisites look ready"
