#!/usr/bin/env python3
"""Check firmware flash/RAM usage against linker-script memory regions."""

from __future__ import annotations

import argparse
import os
import re
import struct
import sys
from dataclasses import dataclass
from pathlib import Path


PT_LOAD = 1
SHF_ALLOC = 0x2


@dataclass(frozen=True)
class MemoryRegion:
    name: str
    origin: int
    length: int

    @property
    def end(self) -> int:
        return self.origin + self.length


@dataclass(frozen=True)
class ElfUsage:
    flash_end: int
    ram_end: int
    rwx_load_segments: int


def parse_int_with_unit(value: str) -> int:
    match = re.fullmatch(r"\s*(0x[0-9a-fA-F]+|\d+)\s*([KkMm]?)\s*", value)
    if not match:
        raise ValueError(f"unsupported memory size: {value!r}")

    number = int(match.group(1), 0)
    unit = match.group(2).lower()
    if unit == "k":
        return number * 1024
    if unit == "m":
        return number * 1024 * 1024
    return number


def strip_c_comments(text: str) -> str:
    return re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)


def parse_linker_memory(linker_script: Path) -> dict[str, MemoryRegion]:
    text = strip_c_comments(linker_script.read_text(encoding="utf-8"))
    regions: dict[str, MemoryRegion] = {}

    pattern = re.compile(
        r"^\s*(?P<name>\w+)\s*(?:\([^)]*\))?\s*:\s*"
        r"ORIGIN\s*=\s*(?P<origin>[^,\s]+)\s*,\s*"
        r"LENGTH\s*=\s*(?P<length>[^\s,]+)",
        re.MULTILINE,
    )

    for match in pattern.finditer(text):
        name = match.group("name").upper()
        regions[name] = MemoryRegion(
            name=name,
            origin=parse_int_with_unit(match.group("origin")),
            length=parse_int_with_unit(match.group("length")),
        )

    missing = {"FLASH", "RAM"} - set(regions)
    if missing:
        missing_text = ", ".join(sorted(missing))
        raise ValueError(f"{linker_script} is missing MEMORY region(s): {missing_text}")

    return regions


def overlaps_region(start: int, size: int, region: MemoryRegion) -> bool:
    if size == 0:
        return False
    return start < region.end and start + size > region.origin


def region_span_end(start: int, size: int, region: MemoryRegion) -> int:
    if start < region.origin:
        return region.origin + size - (region.origin - start)
    return start + size


def parse_elf_usage(elf_path: Path, flash: MemoryRegion, ram: MemoryRegion) -> ElfUsage:
    data = elf_path.read_bytes()
    if data[:4] != b"\x7fELF":
        raise ValueError(f"{elf_path} is not an ELF file")
    if data[5] != 1:
        raise ValueError(f"{elf_path} is not little-endian ELF")

    elf_class = data[4]
    if elf_class == 1:
        endian = "<"
        header = struct.unpack_from(endian + "HHIIIIIHHHHHH", data, 16)
        e_phoff, e_shoff = header[4], header[5]
        e_phentsize, e_phnum = header[8], header[9]
        e_shentsize, e_shnum = header[10], header[11]
        ph_fmt = endian + "IIIIIIII"
        sh_fmt = endian + "IIIIIIIIII"
    elif elf_class == 2:
        endian = "<"
        header = struct.unpack_from(endian + "HHIQQQIHHHHHH", data, 16)
        e_phoff, e_shoff = header[4], header[5]
        e_phentsize, e_phnum = header[8], header[9]
        e_shentsize, e_shnum = header[10], header[11]
        ph_fmt = endian + "IIQQQQQQ"
        sh_fmt = endian + "IIQQQQIIQQ"
    else:
        raise ValueError(f"{elf_path} has unsupported ELF class {elf_class}")

    flash_end = flash.origin
    rwx_load_segments = 0

    for index in range(e_phnum):
        offset = e_phoff + index * e_phentsize
        ph = struct.unpack_from(ph_fmt, data, offset)
        if elf_class == 1:
            p_type, _p_offset, _p_vaddr, p_paddr, p_filesz, _p_memsz, p_flags, _p_align = ph
        else:
            p_type, p_flags, _p_offset, _p_vaddr, p_paddr, p_filesz, _p_memsz, _p_align = ph

        if p_type != PT_LOAD:
            continue
        if (p_flags & 0x7) == 0x7:
            rwx_load_segments += 1
        if overlaps_region(p_paddr, p_filesz, flash):
            flash_end = max(flash_end, region_span_end(p_paddr, p_filesz, flash))

    ram_end = ram.origin
    for index in range(e_shnum):
        offset = e_shoff + index * e_shentsize
        section = struct.unpack_from(sh_fmt, data, offset)
        _sh_name, _sh_type, sh_flags, sh_addr = section[:4]
        sh_size = section[5]
        if not (sh_flags & SHF_ALLOC):
            continue
        if overlaps_region(sh_addr, sh_size, ram):
            ram_end = max(ram_end, region_span_end(sh_addr, sh_size, ram))

    return ElfUsage(flash_end=flash_end, ram_end=ram_end, rwx_load_segments=rwx_load_segments)


def usage_percent(used: int, total: int) -> float:
    return (used / total) * 100.0 if total else 0.0


def format_bytes(value: int) -> str:
    return f"{value} B"


def env_limit(project: str, kind: str) -> int | None:
    project_key = re.sub(r"[^A-Za-z0-9]+", "_", project).upper()
    names = (
        f"{project_key}_MAX_{kind.upper()}_BYTES",
        f"FIRMWARE_MAX_{kind.upper()}_BYTES",
    )
    for name in names:
        raw_value = os.environ.get(name)
        if raw_value:
            return parse_int_with_unit(raw_value)
    return None


def append_step_summary(
    project: str,
    configuration: str,
    flash: MemoryRegion,
    ram: MemoryRegion,
    flash_used: int,
    ram_used: int,
) -> None:
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if not summary_path:
        return

    rows = [
        ("FLASH", flash_used, flash.length, flash.length - flash_used),
        ("RAM", ram_used, ram.length, ram.length - ram_used),
    ]
    with open(summary_path, "a", encoding="utf-8") as summary:
        summary.write(f"\n### Firmware size: `{project}` `{configuration}`\n\n")
        summary.write("| Region | Used | Limit | Remaining | Usage |\n")
        summary.write("| --- | ---: | ---: | ---: | ---: |\n")
        for name, used, limit, remaining in rows:
            summary.write(
                f"| {name} | {used} B | {limit} B | {remaining} B | {usage_percent(used, limit):.1f}% |\n"
            )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("project", help="firmware project directory, e.g. rt_led")
    parser.add_argument("configuration", nargs="?", default="Debug", help="build configuration")
    parser.add_argument("--elf", type=Path, help="path to firmware ELF")
    parser.add_argument("--linker", type=Path, help="path to linker script")
    parser.add_argument("--max-flash", type=parse_int_with_unit, help="override maximum FLASH bytes")
    parser.add_argument("--max-ram", type=parse_int_with_unit, help="override maximum RAM bytes")
    parser.add_argument("--warn-percent", type=float, default=float(os.environ.get("FIRMWARE_SIZE_WARN_PERCENT", "95")))
    args = parser.parse_args()

    project_dir = Path(args.project)
    elf_path = args.elf or project_dir / args.configuration / f"{project_dir.name}.elf"
    linker_script = args.linker or project_dir / "STM32G070CBTX_FLASH.ld"

    regions = parse_linker_memory(linker_script)
    flash = regions["FLASH"]
    ram = regions["RAM"]
    usage = parse_elf_usage(elf_path, flash, ram)

    flash_used = usage.flash_end - flash.origin
    ram_used = usage.ram_end - ram.origin
    flash_limit = args.max_flash or env_limit(args.project, "flash") or flash.length
    ram_limit = args.max_ram or env_limit(args.project, "ram") or ram.length

    print(f"Firmware size: {args.project} {args.configuration}")
    print(
        f"  FLASH: {format_bytes(flash_used)} / {format_bytes(flash.length)} "
        f"({usage_percent(flash_used, flash.length):.1f}%, {format_bytes(flash.length - flash_used)} free)"
    )
    print(
        f"  RAM:   {format_bytes(ram_used)} / {format_bytes(ram.length)} "
        f"({usage_percent(ram_used, ram.length):.1f}%, {format_bytes(ram.length - ram_used)} free)"
    )
    sys.stdout.flush()

    failed = False
    if flash_used > flash_limit:
        print(f"error: FLASH usage exceeds limit: {flash_used} > {flash_limit}", file=sys.stderr)
        failed = True
    if ram_used > ram_limit:
        print(f"error: RAM usage exceeds limit: {ram_used} > {ram_limit}", file=sys.stderr)
        failed = True
    if usage.rwx_load_segments:
        print(
            f"warning: ELF has {usage.rwx_load_segments} RWX LOAD segment(s); review linker segment permissions",
            file=sys.stderr,
        )
    if usage_percent(flash_used, flash.length) >= args.warn_percent:
        print(f"warning: FLASH usage is at or above {args.warn_percent:.1f}%", file=sys.stderr)
    if usage_percent(ram_used, ram.length) >= args.warn_percent:
        print(f"warning: RAM usage is at or above {args.warn_percent:.1f}%", file=sys.stderr)

    append_step_summary(args.project, args.configuration, flash, ram, flash_used, ram_used)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
