#!/usr/bin/env python3
"""Generate the read-only payload source for Gnuboy Save Importer."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


SUPPORTED_SIZES = {512, 2_048, 8_192, 32_768, 65_536, 131_072}
TARGET_RE = re.compile(r"[A-Za-z0-9_-]{1,8}\.sav", re.IGNORECASE)


def c_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--target", required=True)
    parser.add_argument("--drive", choices=("drv0", "crd0"), default="drv0")
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    if not TARGET_RE.fullmatch(args.target):
        parser.error("--target must be an 8.3-style .sav name, e.g. game.sav")

    data = args.input.read_bytes()
    original_size = len(data)
    if len(data) not in SUPPORTED_SIZES and len(data) - 48 in SUPPORTED_SIZES:
        data = data[:-48]
        print(f"stripped a 48-byte emulator footer: {original_size} -> {len(data)}")
    if len(data) not in SUPPORTED_SIZES:
        parser.error(
            f"unsupported save size {len(data)}; expected one of "
            + ", ".join(str(size) for size in sorted(SUPPORTED_SIZES))
        )

    stem = args.target[:-4]
    target_path = f"\\\\{args.drive}\\ROMS\\{args.target}"
    backup_path = f"\\\\{args.drive}\\ROMS\\{stem}.bak"
    lines = [
        "/* Generated from a user-supplied save. Do not commit this file. */",
        "const unsigned char save_payload[] = {",
    ]
    for offset in range(0, len(data), 12):
        chunk = data[offset : offset + 12]
        lines.append("    " + ", ".join(f"0x{byte:02x}" for byte in chunk) + ",")
    lines.extend(
        [
            "};",
            f"const unsigned int save_payload_len = {len(data)}U;",
            f'const char save_target_path[] = "{c_string(target_path)}";',
            f'const char save_backup_path[] = "{c_string(backup_path)}";',
            f'const char save_target_name[] = "{c_string(args.target)}";',
            "",
        ]
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines), encoding="ascii")
    print(f"embedded {len(data)} bytes for {target_path}")


if __name__ == "__main__":
    main()
