#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project="${1:-rt_led}"
configuration="${2:-Debug}"
project_dir="$repo_root/$project"
build_dir="$project_dir/$configuration"

"$repo_root/scripts/setup-local-deps.sh"

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
  echo "error: arm-none-eabi-gcc was not found on PATH." >&2
  echo "Install the STM32 ARM GCC toolchain before running the firmware build." >&2
  exit 127
fi

if [[ ! -f "$build_dir/makefile" ]]; then
  echo "error: $project/$configuration/makefile was not found." >&2
  echo "This repo currently needs STM32CubeIDE-generated makefiles for firmware CI." >&2
  echo "Generate the $configuration build for $project, or replace this script with a standalone firmware build." >&2
  exit 2
fi

if [[ ! -d "$project_dir/Drivers" ]]; then
  echo "error: $project/Drivers was not found." >&2
  echo "Vendor/pin STM32 dependencies or add a setup step that restores them before CI runs." >&2
  exit 2
fi

make -C "$build_dir" all

artifact="$build_dir/$project.elf"
if [[ -f "$artifact" ]]; then
  arm-none-eabi-size "$artifact"
fi
