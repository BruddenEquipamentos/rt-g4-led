#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode="all"
target=""

usage() {
  cat <<'USAGE'
Usage:
  scripts/run_cppcheck.sh --all
  scripts/run_cppcheck.sh --target <rt_led|rt_led_bootloader>

Environment:
  CPPCHECK_ERROR_EXITCODE  Error exit code for cppcheck issues. Defaults to 0.
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --all)
      mode="all"
      shift
      ;;
    --target)
      mode="target"
      target="${2:-}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "error: unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if ! command -v cppcheck >/dev/null 2>&1; then
  echo "error: cppcheck was not found on PATH." >&2
  exit 127
fi

case "$mode:$target" in
  all:)
    paths=("$repo_root/rt_led/Core" "$repo_root/rt_led_bootloader/Core" "$repo_root/shared" "$repo_root/desktop")
    ;;
  target:rt_led)
    paths=("$repo_root/rt_led/Core" "$repo_root/shared")
    ;;
  target:rt_led_bootloader)
    paths=("$repo_root/rt_led_bootloader/Core")
    ;;
  *)
    echo "error: unsupported target '$target'." >&2
    exit 2
    ;;
esac

cppcheck \
  --enable=warning,style,performance,portability \
  --inline-suppr \
  --quiet \
  --error-exitcode="${CPPCHECK_ERROR_EXITCODE:-0}" \
  --suppress=missingIncludeSystem \
  -i"$repo_root/desktop/include" \
  -i"$repo_root/rt_led/Core/Src/eeprom_emul.c" \
  -i"$repo_root/rt_led/Drivers" \
  -i"$repo_root/rt_led/Middlewares" \
  -i"$repo_root/rt_led/Debug" \
  -i"$repo_root/rt_led/Release" \
  -i"$repo_root/rt_led_bootloader/Middlewares" \
  -i"$repo_root/rt_led_bootloader/Debug" \
  -i"$repo_root/rt_led_bootloader/Release" \
  "${paths[@]}"
