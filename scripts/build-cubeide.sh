#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project_dir="${1:-rt_led}"
configuration="${2:-Debug}"
workspace="${STM32CUBEIDE_WORKSPACE:-}"

"$repo_root/scripts/setup-local-deps.sh"

find_cubeide() {
  if [[ -n "${STM32CUBEIDE:-}" ]]; then
    printf '%s\n' "$STM32CUBEIDE"
    return
  fi

  if command -v stm32cubeide >/dev/null 2>&1; then
    command -v stm32cubeide
    return
  fi

  if [[ -x "/Applications/STM32CubeIDE.app/Contents/MacOS/stm32cubeide" ]]; then
    printf '%s\n' "/Applications/STM32CubeIDE.app/Contents/MacOS/stm32cubeide"
    return
  fi

  for candidate in /opt/st/stm32cubeide_*/stm32cubeide /opt/ST/STM32CubeIDE_*/STM32CubeIDE/stm32cubeide; do
    if [[ -x "$candidate" ]]; then
      printf '%s\n' "$candidate"
      return
    fi
  done

  return 1
}

cubeide="$(find_cubeide || true)"
if [[ -z "$cubeide" || ! -x "$cubeide" ]]; then
  echo "error: STM32CubeIDE executable was not found." >&2
  echo "Set STM32CUBEIDE=/path/to/stm32cubeide or install STM32CubeIDE." >&2
  exit 127
fi

project_path="$repo_root/$project_dir"
project_file="$project_path/.project"

if [[ ! -f "$project_file" ]]; then
  echo "error: $project_dir is not an STM32CubeIDE project with a .project file." >&2
  exit 2
fi

project_name="$(sed -n 's:.*<name>\(.*\)</name>.*:\1:p' "$project_file" | head -n 1)"
if [[ -z "$project_name" ]]; then
  echo "error: could not read project name from $project_file." >&2
  exit 2
fi

if [[ -z "$workspace" ]]; then
  workspace="$(mktemp -d "${TMPDIR:-/tmp}/stm32cubeide-workspace.XXXXXX")"
  cleanup_workspace=1
else
  mkdir -p "$workspace"
  cleanup_workspace=0
fi

cleanup() {
  if [[ "${cleanup_workspace:-0}" == "1" ]]; then
    rm -rf "$workspace"
  fi
}
trap cleanup EXIT

"$cubeide" \
  --launcher.suppressErrors \
  -nosplash \
  -no-indexer \
  -data "$workspace" \
  -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
  -import "$project_path" \
  -cleanBuild "$project_name/$configuration"

artifact="$project_path/$configuration/$project_name.elf"
if [[ -f "$artifact" ]] && command -v arm-none-eabi-size >/dev/null 2>&1; then
  arm-none-eabi-size "$artifact"
fi
