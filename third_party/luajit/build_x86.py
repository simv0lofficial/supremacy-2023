"""Build the vendored pinned, client-gated x86/FR1 LuaJIT static archive."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess


SOURCE = Path(__file__).resolve().parent
COMMIT = "63c8f1a9b479ba5518cf6a170ff7729178bc3cfd"
ARCHIVE_SHA256 = "9d1f1e5e4c7b91d7a047fb22a6bb0c1b074f6d29e011c7940c5b4000e7391bc1"
# The original x86 VM unwinds Lua errors directly (0x4342A648). Letting
# Windows dispatch them first exposes normal errors to process-wide VEH hooks.
RUNTIME_FLAGS = ("-DLUAJIT_ENABLE_LUA52COMPAT", "-DLUAJIT_UNWIND_INTERNAL")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--toolchain", required=True, type=Path,
                        help="Directory containing the llvm-mingw i686 compiler and mingw32-make")
    parser.add_argument("--build-dir", required=True, type=Path)
    args = parser.parse_args()
    tools, build = args.toolchain.resolve(), args.build_dir.resolve()
    if build == SOURCE or SOURCE.is_relative_to(build) or build.is_relative_to(SOURCE):
        raise ValueError("LuaJIT generated files require a separate build directory")
    manifest = json.loads((SOURCE / "pinned_sources.json").read_text(encoding="utf-8"))
    if manifest["commit"] != COMMIT or manifest["archive_sha256"] != ARCHIVE_SHA256:
        raise ValueError("The vendored LuaJIT pin differs from the validated client runtime")
    for name, expected in manifest["files"].items():
        source = (SOURCE / name).resolve()
        target = (build / name).resolve()
        if not source.is_relative_to(SOURCE) or not target.is_relative_to(build):
            raise ValueError("Pinned LuaJIT input escapes its source or build directory")
        data = source.read_bytes()
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError(f"Pinned LuaJIT input changed: {name}")
        target.parent.mkdir(parents=True, exist_ok=True)
        if not target.is_file() or target.read_bytes() != data:
            target.write_bytes(data)
    compiler = tools / "i686-w64-mingw32-clang.exe"
    make = tools / "mingw32-make.exe"
    archiver = tools / "llvm-ar.exe"
    for tool in (compiler, make, archiver):
        if not tool.is_file():
            raise FileNotFoundError(tool)
    env = os.environ.copy()
    env.update(PATH=str(tools) + os.pathsep + env["PATH"], TERM="", MSYSTEM="", OS="Windows_NT")
    flags = " ".join(RUNTIME_FLAGS)
    flags_stamp = build / "skeet_runtime_flags.txt"
    command = [str(make), "-j2", "BUILDMODE=static", "CC=i686-w64-mingw32-clang.exe",
               "HOST_CC=i686-w64-mingw32-clang.exe", "TARGET_AR=llvm-ar.exe rcus",
               "GIT_RELVER=type ..\\.relver >luajit_relver.txt", "GIT_DEP=",
               "HOST_SYS=Windows", "TARGET_SYS=Windows", f"XCFLAGS={flags}",
               "libluajit.a"]
    # LuaJIT's Makefile does not track changes to XCFLAGS in existing objects.
    if not flags_stamp.is_file() or flags_stamp.read_text(encoding="ascii") != flags:
        command.append("-B")
    subprocess.run(command, cwd=build / "src", env=env, check=True)
    layout_object = build / "skeet_layout.o"
    subprocess.run([str(compiler), "-std=c11", "-O2", "-m32", "-msse2",
                    *RUNTIME_FLAGS, "-Wall", "-Wextra", "-Werror",
                    "-I", str(build / "src"), "-c", str(SOURCE / "skeet_layout.c"),
                    "-o", str(layout_object)], env=env, check=True)
    archive = build / "src/libluajit.a"
    subprocess.run([str(archiver), "rcs", str(archive), str(layout_object)], env=env, check=True)
    shutil.copy2(SOURCE / "COPYRIGHT", build / "COPYRIGHT")
    flags_stamp.write_text(flags, encoding="ascii")
    print(f"Pinned LuaJIT x86/FR1/Lua52 static archive: {archive}", flush=True)


if __name__ == "__main__":
    main()
