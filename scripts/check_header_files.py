#!/usr/bin/env python3

import re
import sys
from pathlib import Path

PROJECT_NAME = "NEBEL"
SRC_DIR = Path("src")
HEADER_EXTS = {".hpp"}


def get_expected_guard(rel_path: Path) -> str:
    sanitized = re.sub(r"[^a-zA-Z0-9]", "_", rel_path.as_posix()).upper()
    return f"{PROJECT_NAME}_{sanitized}"


def is_valid_header(filepath: Path, src_root: Path) -> bool:
    expected_guard = get_expected_guard(filepath.relative_to(src_root))

    try:
        lines = filepath.read_text(encoding="utf-8").splitlines()
    except UnicodeDecodeError:
        return False

    non_empty = [line.strip() for line in lines if line.strip()]

    return (
        len(non_empty) >= 4
        and non_empty[0] == "#pragma once"
        and non_empty[1] == f"#ifndef {expected_guard}"
        and non_empty[2] == f"#define {expected_guard}"
        and non_empty[-1] == f"#endif // {expected_guard}"
    )


def main() -> int:
    if not SRC_DIR.is_dir():
        print(f"Error: '{SRC_DIR}' directory not found.", file=sys.stderr)
        return 1

    invalid_files = [
        path.as_posix()
        for path in sorted(SRC_DIR.rglob("*"))
        if path.is_file()
        and path.suffix in HEADER_EXTS
        and not is_valid_header(path, SRC_DIR)
    ]

    if invalid_files:
        print("Found header files with invalid header guards:")
        print("\n".join(invalid_files))
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
