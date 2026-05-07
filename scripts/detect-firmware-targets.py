#!/usr/bin/env python3
"""Detect affected STM32CubeIDE firmware projects for CI."""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path


FIRMWARE_TARGETS = ("rt_led", "rt_led_bootloader")
ALL_TARGETS_MARKERS = (
    ".github/",
    "scripts/",
    "ci/",
)
ROOT_BUILD_SUFFIXES = (".mk", ".yml", ".yaml")
ROOT_BUILD_NAMES = {
    "Makefile",
    "CMakeLists.txt",
    "Dockerfile",
    "compose.yaml",
    "compose.yml",
    "docker-compose.yaml",
    "docker-compose.yml",
}


def run_git_diff(base: str | None, head: str | None) -> list[str]:
    if base and head:
        cmd = ["git", "diff", "--name-only", f"{base}...{head}"]
    elif base:
        cmd = ["git", "diff", "--name-only", base]
    else:
        cmd = ["git", "diff", "--name-only", "HEAD~1", "HEAD"]

    result = subprocess.run(cmd, check=False, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode != 0:
        fallback = subprocess.run(
            ["git", "ls-files"],
            check=True,
            text=True,
            stdout=subprocess.PIPE,
        )
        return [line for line in fallback.stdout.splitlines() if line]

    return [line for line in result.stdout.splitlines() if line]


def normalize(path: str) -> str:
    return path.strip().replace(os.sep, "/")


def root_build_file(path: str) -> bool:
    if "/" in path:
        return False
    name = Path(path).name
    return name in ROOT_BUILD_NAMES or name.endswith(ROOT_BUILD_SUFFIXES)


def affects_all(path: str) -> bool:
    return (
        path.startswith(ALL_TARGETS_MARKERS)
        or path == "documents/ci-cd-prep.md"
        or root_build_file(path)
    )


def detect_targets(changed_files: list[str]) -> list[str]:
    targets: set[str] = set()

    for raw_path in changed_files:
        path = normalize(raw_path)
        if not path:
            continue

        if affects_all(path):
            return list(FIRMWARE_TARGETS)

        if path.startswith("rt_led/"):
            targets.add("rt_led")
        elif path.startswith("rt_led_bootloader/"):
            targets.add("rt_led_bootloader")
        elif path.startswith("shared/") or path.startswith("desktop/") or path.startswith("tests/"):
            targets.add("rt_led")

    return [target for target in FIRMWARE_TARGETS if target in targets]


def write_github_output(matrix: list[str], output_path: str) -> None:
    has_targets = "true" if matrix else "false"
    with open(output_path, "a", encoding="utf-8") as output:
        output.write(f"matrix={json.dumps(matrix)}\n")
        output.write(f"has_targets={has_targets}\n")
        output.write(f"any={has_targets}\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", default=os.environ.get("BASE_SHA"))
    parser.add_argument("--head", default=os.environ.get("HEAD_SHA"))
    parser.add_argument("--all", action="store_true", help="select all firmware targets")
    parser.add_argument("files", nargs="*", help="changed files; defaults to git diff")
    args = parser.parse_args()

    changed_files = args.files or run_git_diff(args.base, args.head)
    matrix = list(FIRMWARE_TARGETS) if args.all else detect_targets(changed_files)
    print(json.dumps(matrix))

    github_output = os.environ.get("GITHUB_OUTPUT")
    if github_output:
        write_github_output(matrix, github_output)

    return 0


if __name__ == "__main__":
    sys.exit(main())
