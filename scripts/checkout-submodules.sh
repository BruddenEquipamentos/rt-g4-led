#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ ! -f "$repo_root/.gitmodules" ]]; then
  exit 0
fi

if [[ -n "${CI:-}" && -z "${CI_SUBMODULE_TOKEN:-}" ]]; then
  cat >&2 <<'MSG'
error: CI_SUBMODULE_TOKEN is not configured.

This repository uses a private Git submodule:
  shared/ble/ble_protocol -> https://github.com/BruddenEquipamentos/ble_protocol.git

Create a fine-grained GitHub token with read access to both repositories, then add it to
this repository as an Actions secret named CI_SUBMODULE_TOKEN.
MSG
  exit 2
fi

if [[ -n "${CI_SUBMODULE_TOKEN:-}" ]]; then
  git -C "$repo_root" config --local --unset-all http.https://github.com/.extraheader 2>/dev/null || true
  git config --global \
    url."https://x-access-token:${CI_SUBMODULE_TOKEN}@github.com/".insteadOf \
    "https://github.com/"
fi

git -C "$repo_root" submodule sync --recursive
git -C "$repo_root" submodule update --init --recursive
