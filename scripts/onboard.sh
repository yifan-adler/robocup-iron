#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "[1/5] Verify and extract official platform"
"$repo_root/scripts/bootstrap.sh"

echo "[2/5] Build official example"
"$repo_root/scripts/build.sh" --client official

echo "[3/5] Run official Stage1 IT smoke"
"$repo_root/scripts/run_case.sh" --stage 1 --mode it --case 01 --client official

echo "[4/5] Build Iron candidate"
"$repo_root/scripts/build.sh" --client iron

echo "[5/5] Run Iron four-mode smoke suite"
"$repo_root/scripts/run_suite.sh" --manifest tests/manifests/smoke.csv

echo "Onboarding passed. Record the latest five artifacts/runs summary.json files in docs/onboarding/."
