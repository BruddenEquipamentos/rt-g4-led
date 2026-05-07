#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

app_drivers="$repo_root/rt_led/Drivers"
bootloader_drivers="$repo_root/rt_led_bootloader/Drivers"

if [[ ! -d "$app_drivers" ]]; then
  echo "error: rt_led/Drivers was not found." >&2
  echo "Restore or generate the STM32Cube HAL/CMSIS Drivers tree before building firmware." >&2
  exit 2
fi

if [[ -e "$bootloader_drivers" && ! -L "$bootloader_drivers" ]]; then
  echo "error: rt_led_bootloader/Drivers exists and is not a symlink." >&2
  echo "Move it aside or keep it intentionally if the bootloader needs a separate vendor tree." >&2
  exit 2
fi

if [[ ! -e "$bootloader_drivers" ]]; then
  ln -s ../rt_led/Drivers "$bootloader_drivers"
fi
