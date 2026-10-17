#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Pack a Teak ELF into a DSP1 image.

Program memory sits at VMA 0 (word address = VMA / 2), data memory at
VMA 0x10000000 (word address = (VMA - 0x10000000) / 2).
"""
import argparse
import hashlib
import struct
import sys

DATA_BASE = 0x10000000
HEADER_SIZE = 0x300


def elf_segments(path):
    b = open(path, "rb").read()
    if b[:4] != b"\x7fELF" or b[4] != 1 or b[5] != 1:
        sys.exit("expected a 32-bit little-endian ELF")
    (phoff,) = struct.unpack_from("<I", b, 0x1C)
    phentsize, phnum = struct.unpack_from("<HH", b, 0x2A)
    segments = []
    for i in range(phnum):
        p_type, p_offset, p_vaddr, _, p_filesz, p_memsz, _, _ = \
            struct.unpack_from("<8I", b, phoff + i * phentsize)
        if p_type != 1 or p_filesz == 0:     # .ybss: its owners clear it
            continue
        data = b[p_offset:p_offset + p_filesz] + bytes(p_memsz - p_filesz)
        if len(data) & 1:
            data += b"\0"
        if p_vaddr >= DATA_BASE:
            if p_vaddr >= DATA_BASE + 0x20000:
                continue              # AAC bank1 tables uploaded explicitly
            segments.append((2, (p_vaddr - DATA_BASE) // 2, data))
        else:
            segments.append((0, p_vaddr // 2, data))
    return segments


def pack(segments):
    if not 1 <= len(segments) <= 10:
        sys.exit("DSP1 holds 1 to 10 segments, got %d" % len(segments))
    body = bytearray()
    records = bytearray()
    for kind, word_address, data in segments:
        limit = 0x20000 if kind == 0 else 0x10000
        if word_address >= limit:
            sys.exit("segment address 0x%x out of range" % word_address)
        records += struct.pack("<III3xB", HEADER_SIZE + len(body), word_address, len(data), kind)
        records += hashlib.sha256(data).digest()
        body += data
        while len(body) % 4:
            body += b"\0"
    records += bytes(0x30 * (10 - len(segments)))
    header = bytearray(0x100)           # signature, unused
    header += b"DSP1"
    # flags bit 0: boot handshake
    header += struct.pack("<IH3xBBBII8x", HEADER_SIZE + len(body), 0xFFFF, 0, len(segments), 1, 0, 0)
    header += records
    assert len(header) == HEADER_SIZE
    return bytes(header + body)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("elf")
    ap.add_argument("-o", "--output", required=True)
    args = ap.parse_args()
    image = pack(elf_segments(args.elf))
    open(args.output, "wb").write(image)
    print("%s: %d bytes" % (args.output, len(image)))


if __name__ == "__main__":
    main()
