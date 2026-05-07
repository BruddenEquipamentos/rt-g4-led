#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage:
  scripts/build-cubeide-docker.sh [project] [configuration] [stm32cubeide-version]

Defaults:
  project                 rt_led
  configuration           Debug
  stm32cubeide-version    $STM32CUBEIDE_VERSION or 2.1.1

Environment:
  STM32CUBEIDE_IMAGE      Override the Docker image. Defaults to xanderhendriks/stm32cubeide:<version>.
  STM32CUBEIDE_WORKSPACE  Workspace path inside the container. Defaults to /tmp/stm-workspace.
USAGE
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project="${1:-rt_led}"
configuration="${2:-Debug}"
version="${3:-${STM32CUBEIDE_VERSION:-2.1.1}}"
project_path="$repo_root/$project"
project_file="$project_path/.project"

if [[ ! -f "$project_file" ]]; then
  echo "error: $project is not an STM32CubeIDE project with a .project file." >&2
  usage >&2
  exit 2
fi

project_name="$(sed -n 's:.*<name>\(.*\)</name>.*:\1:p' "$project_file" | head -n 1)"
if [[ -z "$project_name" ]]; then
  echo "error: could not read project name from $project_file." >&2
  exit 2
fi

if ! command -v docker >/dev/null 2>&1; then
  echo "error: docker was not found on PATH." >&2
  exit 127
fi

if ! docker info >/dev/null 2>&1; then
  echo "error: Docker is not running or this user cannot access the Docker daemon." >&2
  exit 127
fi

"$repo_root/scripts/setup-local-deps.sh"

image="${STM32CUBEIDE_IMAGE:-xanderhendriks/stm32cubeide:$version}"
container_workspace="/github/workspace"
cube_workspace="${STM32CUBEIDE_WORKSPACE:-/tmp/stm-workspace}"
project_target="$project_name/$configuration"

docker run --rm \
  --workdir "$container_workspace" \
  -v "$repo_root:$container_workspace" \
  "$image" \
  bash -lc '
    set -euo pipefail
    project_path="$1"
    project_target="$2"
    cube_workspace="$3"

    rm -rf "$cube_workspace"
    mkdir -p "$cube_workspace"

    stm32cubeide \
      --launcher.suppressErrors \
      -nosplash \
      -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
      -no-indexer \
      -data "$cube_workspace" \
      -importAll "$project_path"

    headless-build.sh \
      -no-indexer \
      -data "$cube_workspace" \
      -build "$project_target"
  ' bash "$project" "$project_target" "$cube_workspace"
