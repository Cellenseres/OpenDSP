#!/usr/bin/env python3
"""Verify the complete vendored codec snapshots without network access."""
import hashlib
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
DEPENDENCIES = ("libogg", "tremor", "opus", "opusfile")


def snapshot():
    files = {}
    for name in DEPENDENCIES:
        for path in sorted((ROOT / name).rglob("*")):
            if path.is_file():
                files[path.relative_to(ROOT).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    return files


def main():
    actual = snapshot()
    manifest = ROOT / "SHA256SUMS"
    if sys.argv[1:] == ["--update"]:
        manifest.write_text("".join(f"{digest}  {name}\n" for name, digest in sorted(actual.items())),
                            encoding="utf-8", newline="\n")
        print(f"Wrote {len(actual)} upstream file hashes")
        return 0
    if sys.argv[1:]:
        raise SystemExit("usage: verify.py [--update]")
    expected = dict((line[66:], line[:64]) for line in manifest.read_text(encoding="utf-8").splitlines())
    different = sorted(name for name in expected.keys() | actual.keys()
                       if expected.get(name) != actual.get(name))
    if different:
        print("Vendor snapshot differs:", *different, sep="\n  ")
        return 1
    print(f"Verified {len(actual)} upstream files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
