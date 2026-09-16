#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck disable=SC1091
source "$repo_root/config/platform.env"

archive_path="$repo_root/$PLATFORM_ARCHIVE_RELATIVE"
expected_sha="$PLATFORM_ARCHIVE_SHA256"
target="$repo_root/$PLATFORM_EXTRACT_DIR_RELATIVE"
stamp="$target/.archive.sha256"

if [[ ! -f "$archive_path" ]]; then
  echo "ERROR: official platform archive not found: $archive_path" >&2
  exit 1
fi

actual_sha="$(sha256sum "$archive_path" | awk '{print $1}')"
if [[ "$actual_sha" != "$expected_sha" ]]; then
  echo "ERROR: platform archive SHA256 mismatch" >&2
  echo "expected: $expected_sha" >&2
  echo "actual:   $actual_sha" >&2
  exit 1
fi

if [[ -f "$stamp" ]] && [[ "$(tr -d '\r\n' < "$stamp")" == "$expected_sha" ]]; then
  echo "Platform is already bootstrapped at $target"
  exit 0
fi

if [[ -e "$target" ]]; then
  echo "ERROR: $target exists without the expected stamp; move it aside and retry" >&2
  exit 1
fi

mkdir -p "$target"
unzip -q "$archive_path" -d "$target"
printf '%s\n' "$expected_sha" > "$stamp"

platform_root="$target/$PLATFORM_ROOT_RELATIVE"
for required in CMakeLists.txt bin/cserver lib/libasp.so res/iclingo include/cserver/plug.hpp; do
  if [[ ! -e "$platform_root/$required" ]]; then
    echo "ERROR: extracted platform is missing $required" >&2
    exit 1
  fi
done

chmod +x "$platform_root/bin/cserver" "$platform_root/res/iclingo" || true
echo "Platform $PLATFORM_RELEASE bootstrapped and verified at $platform_root"
