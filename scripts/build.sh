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

source_platform="$repo_root/.work/planner-2025/Planner-release-2025/Planner-release-ubuntu18"
platform="$repo_root/.work/platform-$client"
build_dir="$repo_root/.work/build-$client"

if [[ ! -d "$platform" ]]; then
  cp -a "$source_platform" "$platform"
fi

if [[ "$client" == "iron" ]]; then
  for file in CMakeLists.txt debuglog.hpp main.cpp parser.cpp parser.hpp rdfw.cpp rdfw.hpp words.txt; do
    cp "$repo_root/src/iron/$file" "$platform/example/$file"
  done
fi

mkdir -p "$build_dir" "$repo_root/.work/environment"
(
  cd "$build_dir"
  cmake "$platform"
  cmake --build . -- -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"
)

{
  echo "client=$client"
  echo "platform_archive_sha256=fdb9cf54054aaac0ccbfbeabc97a99d0067dc975f0999a90f81fe83a326ac150"
  uname -a
  gcc --version | head -n 1
  g++ --version | head -n 1
  cmake --version | head -n 1
  dpkg-query -W -f='boost=${Version}\n' libboost-dev 2>/dev/null || true
} > "$repo_root/.work/environment/$client.txt"

if [[ ! -x "$platform/bin/example" || ! -x "$platform/bin/cserver" ]]; then
  echo "ERROR: build finished without bin/example or bin/cserver" >&2
  exit 1
fi

echo "Built $client client at $platform/bin/example"
