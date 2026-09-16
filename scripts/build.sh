#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $0 --client official|iron" >&2
}

client=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --client)
      client="${2:-}"
      shift 2
      ;;
    *)
      usage
      exit 2
      ;;
  esac
done

if [[ "$client" != "official" && "$client" != "iron" ]]; then
  usage
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$repo_root/scripts/bootstrap.sh"
# shellcheck disable=SC1091
source "$repo_root/config/platform.env"

source_platform="$repo_root/$PLATFORM_EXTRACT_DIR_RELATIVE/$PLATFORM_ROOT_RELATIVE"
platform="$repo_root/.work/platform-$PLATFORM_CACHE_TAG-$client"
build_dir="$repo_root/.work/build-$PLATFORM_CACHE_TAG-$client"
platform_stamp="$platform/.archive.sha256"

if [[ ! -d "$platform" ]]; then
  cp -a "$source_platform" "$platform"
  printf '%s\n' "$PLATFORM_ARCHIVE_SHA256" > "$platform_stamp"
elif [[ ! -f "$platform_stamp" ]] \
  || [[ "$(tr -d '\r\n' < "$platform_stamp")" != "$PLATFORM_ARCHIVE_SHA256" ]]; then
  echo "ERROR: $platform is not a verified $PLATFORM_RELEASE working copy; move it aside and retry" >&2
  exit 1
fi

if [[ "$client" == "iron" ]]; then
  for file in CMakeLists.txt debuglog.hpp main.cpp parser.cpp parser.hpp rdfw.cpp rdfw.hpp words.txt; do
    cp "$repo_root/src/iron/$file" "$platform/example/$file"
  done

  sdk_patch="$repo_root/infra/patches/iron-plug-timeout.patch"
  if grep -q $'\r' "$platform/src/plug.cpp"; then
    sed -i 's/\r$//' "$platform/src/plug.cpp"
  fi
  if patch --dry-run --silent --forward -p1 -d "$platform" < "$sdk_patch" >/dev/null 2>&1; then
    patch --silent --forward -p1 -d "$platform" < "$sdk_patch"
  elif ! patch --dry-run --silent --reverse -p1 -d "$platform" < "$sdk_patch" >/dev/null 2>&1; then
    echo "ERROR: Iron SDK timeout patch does not apply cleanly" >&2
    exit 1
  fi
fi

mkdir -p "$build_dir" "$repo_root/.work/environment"
(
  cd "$build_dir"
  cmake "$platform"
  cmake --build . -- -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"
)

{
  echo "client=$client"
  echo "platform_release=$PLATFORM_RELEASE"
  echo "platform_archive_sha256=$PLATFORM_ARCHIVE_SHA256"
  if [[ -r /etc/os-release ]]; then
    grep -E '^(PRETTY_NAME|VERSION_ID|VERSION_CODENAME)=' /etc/os-release
  fi
  uname -a
  gcc --version | sed -n '1p'
  g++ --version | sed -n '1p'
  cmake --version | sed -n '1p'
  python3 --version
  unzip -v | sed -n '1p'
  dpkg-query -W -f='${binary:Package}=${Version}\n' \
    build-essential cmake coreutils dos2unix git libboost-dev patch procps python3 unzip \
    2>/dev/null | sort || true
} > "$repo_root/.work/environment/$client-$PLATFORM_CACHE_TAG.txt"

if [[ ! -x "$platform/bin/example" || ! -x "$platform/bin/cserver" ]]; then
  echo "ERROR: build finished without bin/example or bin/cserver" >&2
  exit 1
fi

echo "Built $client client at $platform/bin/example"
