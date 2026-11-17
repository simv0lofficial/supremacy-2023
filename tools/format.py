"""Format owned C++ sources, or check them with --check, using clang-format 23+."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess


ROOT = Path(__file__).resolve().parents[1]
EXTENSIONS = {".cpp", ".hpp", ".h", ".c"}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Report formatting differences without editing")
    parser.add_argument("--clang-format", default="clang-format", help="Path to the clang-format executable")
    args = parser.parse_args()
    executable = shutil.which(args.clang_format)
    if executable is None:
        parser.error(f"clang-format executable not found: {args.clang_format}")
    executable = str(Path(executable).resolve())
    files = sorted(
        path for directory in (ROOT / "include", ROOT / "src")
        for path in directory.rglob("*") if path.is_file() and path.suffix in EXTENSIONS
    )
    options = ["--dry-run", "--Werror"] if args.check else ["-i"]
    for offset in range(0, len(files), 60):
        result = subprocess.run(
            [executable, "--style=file", *options, *map(str, files[offset:offset + 60])],
            cwd=ROOT,
            check=False,
        )
        if result.returncode:
            return result.returncode
    print(f"{'Checked' if args.check else 'Formatted'} {len(files)} owned C++ files.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
