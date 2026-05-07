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

git -C "$repo_root" submodule sync --recursive

if [[ -n "${CI_SUBMODULE_TOKEN:-}" ]]; then
  token_url="https://x-access-token:${CI_SUBMODULE_TOKEN}@github.com/"
  git -C "$repo_root" config --local --unset-all http.https://github.com/.extraheader 2>/dev/null || true

  if ! git -c url."$token_url".insteadOf=https://github.com/ \
    ls-remote https://github.com/BruddenEquipamentos/ble_protocol.git HEAD >/dev/null; then
    cat >&2 <<'MSG'
error: CI_SUBMODULE_TOKEN cannot read BruddenEquipamentos/ble_protocol.

Update the secret with a fine-grained GitHub token that has:
  - Resource owner: BruddenEquipamentos
  - Repository access: BruddenEquipamentos/ble_protocol
  - Repository access: BruddenEquipamentos/rt-g4-led
  - Repository permissions: Contents read-only

If the organization requires SSO or fine-grained token approval, approve/authorize
the token for BruddenEquipamentos before rerunning CI.
MSG
    exit 2
  fi

  git -C "$repo_root" \
    -c url."$token_url".insteadOf=https://github.com/ \
    submodule update --init --recursive
else
  git -C "$repo_root" submodule update --init --recursive
fi
