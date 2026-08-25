#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $0 [--repair-sources]" >&2
}

repair_sources=false
while [[ $# -gt 0 ]]; do
  case "$1" in
    --repair-sources) repair_sources=true; shift ;;
    *) usage; exit 2 ;;
  esac
done

if [[ ! -r /etc/os-release ]]; then
  echo "ERROR: /etc/os-release is unavailable" >&2
  exit 1
fi
# shellcheck disable=SC1091
source /etc/os-release
if [[ "${ID:-}" != "ubuntu" || "${VERSION_ID:-}" != "18.04" ]]; then
  echo "ERROR: Ubuntu 18.04 is required; found ${PRETTY_NAME:-unknown}" >&2
  exit 1
fi
if [[ "$(uname -m)" != "x86_64" ]]; then
  echo "ERROR: x86_64 is required; found $(uname -m)" >&2
  exit 1
fi
if ! uname -r | grep -Eqi '(microsoft-standard|wsl2)'; then
  echo "ERROR: WSL2 is required; kernel is $(uname -r)" >&2
  exit 1
fi

if [[ $EUID -eq 0 ]]; then
  sudo_cmd=()
elif command -v sudo >/dev/null 2>&1; then
  sudo_cmd=(sudo)
else
  echo "ERROR: run as root or install sudo" >&2
  exit 1
fi

if [[ "$repair_sources" == true ]]; then
  backup="/etc/apt/sources.list.robocup-backup-$(date -u +%Y%m%dT%H%M%SZ)"
  "${sudo_cmd[@]}" cp -a /etc/apt/sources.list "$backup"
  printf '%s\n' \
    'deb http://archive.ubuntu.com/ubuntu bionic main restricted universe multiverse' \
    'deb http://archive.ubuntu.com/ubuntu bionic-updates main restricted universe multiverse' \
    'deb http://archive.ubuntu.com/ubuntu bionic-security main restricted universe multiverse' \
    'deb http://archive.ubuntu.com/ubuntu bionic-backports main restricted universe multiverse' \
    | "${sudo_cmd[@]}" tee /etc/apt/sources.list >/dev/null
  "${sudo_cmd[@]}" rm -rf /var/lib/apt/lists
  "${sudo_cmd[@]}" mkdir -p /var/lib/apt/lists/partial
  "${sudo_cmd[@]}" apt-get clean
  echo "APT sources repaired; previous file saved at $backup"
fi

if ! "${sudo_cmd[@]}" apt-get update; then
  if [[ "$repair_sources" == false ]]; then
    echo "ERROR: apt-get update failed. Review the error, then rerun with --repair-sources only if the Bionic sources are missing or invalid." >&2
  fi
  exit 1
fi

packages=(
  build-essential
  ca-certificates
  cmake
  coreutils
  dos2unix
  git
  libboost-all-dev
  patch
  procps
  python3
  unzip
)
"${sudo_cmd[@]}" env DEBIAN_FRONTEND=noninteractive apt-get install -y "${packages[@]}"

echo "Ubuntu development dependencies are ready:"
gcc --version | sed -n '1p'
g++ --version | sed -n '1p'
cmake --version | sed -n '1p'
python3 --version
dpkg-query -W -f='boost=${Version}\n' libboost-dev
unzip -v | sed -n '1p'
