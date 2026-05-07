#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

git -C "$repo_root" submodule update --init --recursive

"$repo_root/scripts/setup-local-deps.sh"
"$repo_root/scripts/ci_test_host.sh"
"$repo_root/scripts/ci_static_analysis.sh"
"$repo_root/scripts/build-firmware.sh" rt_led Debug

if [[ "${RUN_CUBEIDE_HEADLESS:-0}" == "1" ]]; then
  "$repo_root/scripts/build-cubeide.sh" rt_led Debug
fi
