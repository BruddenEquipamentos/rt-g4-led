#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project="${1:-rt_led}"
configuration="${2:-Debug}"
source_dir="$repo_root/$project/$configuration"
package_dir="$repo_root/build/firmware/$project"

if [[ ! -d "$source_dir" ]]; then
  echo "error: firmware output directory not found: $project/$configuration" >&2
  exit 2
fi

mkdir -p "$package_dir"

shopt -s nullglob
artifacts=(
  "$source_dir"/*.bin
  "$source_dir"/*.hex
  "$source_dir"/*.elf
  "$source_dir"/*.map
  "$source_dir"/*.list
)
shopt -u nullglob

if [[ ${#artifacts[@]} -eq 0 ]]; then
  echo "error: no firmware artifacts found in $project/$configuration" >&2
  exit 2
fi

for artifact in "${artifacts[@]}"; do
  cp "$artifact" "$package_dir/"
done

{
  printf 'project=%s\n' "$project"
  printf 'configuration=%s\n' "$configuration"
  printf 'generated_utc=%s\n' "$(date -u +'%Y-%m-%dT%H:%M:%SZ')"
  printf 'source_dir=%s\n' "$project/$configuration"
  printf '\nfiles:\n'
  for artifact in "${artifacts[@]}"; do
    printf '  %s\n' "$(basename "$artifact")"
  done
} > "$package_dir/manifest.txt"

(
  cd "$package_dir"
  shasum -a 256 ./* > SHA256SUMS
)

printf '%s\n' "$package_dir"
