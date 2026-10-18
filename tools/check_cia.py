#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Check the installed title version of a CIA."""
import struct
import sys
from pathlib import Path


def title_version(data):
    header, _, _, cert, ticket = struct.unpack_from('<IHHII', data)
    align = lambda n: (n + 63) & ~63
    tmd = align(align(align(header) + cert) + ticket)
    signature = struct.unpack_from('>I', data, tmd)[0]
    signature_bytes = {
        0x10000: 0x240, 0x10001: 0x140, 0x10002: 0x80,
        0x10003: 0x240, 0x10004: 0x140, 0x10005: 0x80,
    }[signature]
    return struct.unpack_from('>H', data, tmd + signature_bytes + 0x9c)[0]


def check(path, version):
    major, minor, micro = map(int, version.split('.'))
    if not (0 <= major <= 63 and 0 <= minor <= 63 and 0 <= micro <= 15):
        raise ValueError('CIA versions allow major/minor 0..63 and micro 0..15')
    expected = (major << 10) | (minor << 4) | micro
    actual = title_version(Path(path).read_bytes())
    if actual != expected:
        raise ValueError(f'CIA version is {actual}, expected {expected} ({version}); '
                         'use makerom 0.18.4 or a version with the minor-version fix')
    print(f'{path}: title version {version} ({actual})')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit('usage: check_cia.py FILE MAJOR.MINOR.MICRO')
    try:
        check(*sys.argv[1:])
    except (OSError, ValueError, KeyError, struct.error) as error:
        sys.exit(str(error))
