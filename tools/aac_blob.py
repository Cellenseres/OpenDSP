#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Extract the bank-1 AAC tables; never load them over bank-0 effects."""
import struct
import sys
from pathlib import Path
b = Path(sys.argv[1]).read_bytes()
off = struct.unpack_from('<I', b, 32)[0]
stride, n, names = struct.unpack_from('<HHH', b, 46)
sections = [struct.unpack_from('<10I', b, off+i*stride) for i in range(n)]
s = sections[names]
strings = b[s[4]:s[4]+s[5]]
for s in sections:
    name = strings[s[0]:strings.index(0, s[0])]
    if name == b'.aac_tables':
        if s[3] != 0x10038000 or s[5] != 22716:
            raise ValueError('AAC table layout mismatch')
        Path(sys.argv[2]).write_bytes(b[s[4]:s[4]+s[5]])
        break
else:
    raise ValueError('missing AAC tables')
