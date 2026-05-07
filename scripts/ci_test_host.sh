#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

"$repo_root/scripts/build-desktop.sh"

build_dir="${BUILD_DIR:-"$repo_root/desktop/build"}"
if [[ -f "$build_dir/CTestTestfile.cmake" ]]; then
  ctest --test-dir "$build_dir" --output-on-failure
fi
