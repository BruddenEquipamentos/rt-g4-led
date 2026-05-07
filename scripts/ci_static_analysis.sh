#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ $# -gt 0 ]]; then
  "$repo_root/scripts/run_cppcheck.sh" "$@"
else
  "$repo_root/scripts/run_cppcheck.sh" --all
fi
