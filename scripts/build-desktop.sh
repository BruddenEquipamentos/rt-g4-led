#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${BUILD_DIR:-"$repo_root/desktop/build"}"

cmake -S "$repo_root/desktop" -B "$build_dir"

if [[ -n "${CI_JOBS:-}" ]]; then
  cmake --build "$build_dir" --parallel "$CI_JOBS"
else
  cmake --build "$build_dir"
fi
