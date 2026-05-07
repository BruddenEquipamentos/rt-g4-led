#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

app_drivers="$repo_root/rt_led/Drivers"

if [[ ! -d "$app_drivers" ]]; then
  echo "error: rt_led/Drivers was not found." >&2
  echo "Restore or generate the STM32Cube HAL/CMSIS Drivers tree before building firmware." >&2
  exit 2
fi
