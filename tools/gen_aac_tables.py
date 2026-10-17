#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Write the AAC-LC decoder tables (library/source/aac/tables.c and tables.h)."""
import math
import os
import sys
from fractions import Fraction

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import aac_codebooks as cb  # noqa: E402

SAMPLE_RATES = [96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000]

# ISO/IEC 14496-3 tables 4.129-4.140 (scalefactor band offsets)
SWB_LONG = {
    96: [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 96, 108, 120, 132,
         144, 156, 172, 188, 212, 240, 276, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024],
    64: [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 100, 112, 124, 140,
         156, 172, 192, 216, 240, 268, 304, 344, 384, 424, 464, 504, 544, 584, 624, 664, 704, 744, 784,
         824, 864, 904, 944, 984, 1024],
    48: [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 160,
         176, 196, 216, 240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704,
         736, 768, 800, 832, 864, 896, 928, 1024],
    32: [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 160,
         176, 196, 216, 240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704,
         736, 768, 800, 832, 864, 896, 928, 960, 992, 1024],
    24: [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 52, 60, 68, 76, 84, 92, 100, 108, 116, 124, 136,
         148, 160, 172, 188, 204, 220, 240, 260, 284, 308, 336, 364, 396, 432, 468, 508, 552, 600, 652,
         704, 768, 832, 896, 960, 1024],
    16: [0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 100, 112, 124, 136, 148, 160, 172, 184, 196, 212,
         228, 244, 260, 280, 300, 320, 344, 368, 396, 424, 456, 492, 532, 572, 616, 664, 716, 772, 832,
         896, 960, 1024],
    8: [0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 172, 188, 204, 220, 236, 252, 268,
        288, 308, 328, 348, 372, 396, 420, 448, 476, 508, 544, 580, 620, 664, 712, 764, 820, 880, 944, 1024],
}
SWB_SHORT = {
    96: [0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128],
    48: [0, 4, 8, 12, 16, 20, 28, 36, 44, 56, 68, 80, 96, 112, 128],
    24: [0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 64, 76, 92, 108, 128],
    16: [0, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 60, 72, 88, 108, 128],
    8: [0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 60, 72, 88, 108, 128],
}
LONG_FOR_INDEX = [96, 96, 64, 48, 48, 32, 24, 24, 16, 16, 16, 8]
SHORT_FOR_INDEX = [96, 96, 96, 48, 48, 48, 24, 24, 16, 16, 16, 8]
NUM_SWB_LONG = {96: 41, 64: 47, 48: 49, 32: 51, 24: 47, 16: 43, 8: 40}
NUM_SWB_SHORT = {96: 12, 48: 14, 24: 15, 16: 15, 8: 15}

# ISO/IEC 14496-3 table 4.157 (LC)
TNS_MAX_BANDS_LONG = [31, 31, 34, 40, 42, 51, 46, 46, 42, 42, 42, 39]
TNS_MAX_BANDS_SHORT = [9, 9, 10, 14, 14, 14, 14, 14, 14, 14, 14, 14]

HUFF_ROOT_BITS = 8
HUFF_SUB_BITS = 4


def q15(x):
    return max(-32768, min(32767, int(round(x * 32768))))


def check_codebook(name, codes, lens):
    assert len(codes) == len(lens), name
    assert sum(Fraction(1, 1 << n) for n in lens) == 1, name + ": not a complete prefix code"
    words = sorted(format(c, "0%db" % n) for c, n in zip(codes, lens))
    for a, b in zip(words, words[1:]):
        assert not b.startswith(a), name + ": prefix collision"


def pack_symbol(book, index):
    if book == "sf":
        return index
    _, _, dim, unsigned, lav = cb.SPECTRAL[book]
    mod = lav + 1 if unsigned else 2 * lav + 1
    if dim == 4:
        w, x, y, z = index // 27, index // 9 % 3, index // 3 % 3, index % 3
        return (w << 6) | (x << 4) | (y << 2) | z
    y, z = index // mod, index % mod
    return (y << 5) | z if book == 11 else (y << 4) | z


def build_decoder(codes, lens, syms):
    """Multi-level lookup table, decoded by huff_decode()."""
    entries = []

    def make(items, nbits):
        base = len(entries)
        entries.extend([None] * (1 << nbits))
        longer = {}
        for code, n, sym in items:
            if n <= nbits:
                shift = nbits - n
                for j in range(1 << shift):
                    entries[base + (code << shift) + j] = (n << 11) | sym
            else:
                longer.setdefault(code >> (n - nbits), []).append((code & ((1 << (n - nbits)) - 1), n - nbits, sym))
        for top, sub in sorted(longer.items()):
            sub_bits = min(HUFF_SUB_BITS, max(n for _, n, _ in sub))
            offset = make(sub, sub_bits)
            assert offset < 2048 and sub_bits < 16
            entries[base + top] = 0x8000 | (sub_bits << 11) | offset
        return base

    root = min(HUFF_ROOT_BITS, max(lens))
    make(list(zip(codes, lens, syms)), root)
    assert None not in entries and all(s < 2048 for s in syms)
    return root, entries


def bessel_i0(x):
    total, term, k = 1.0, 1.0, 1
    while term > 1e-17 * total:
        term *= (x / (2 * k)) ** 2
        total += term
        k += 1
    return total


def kbd_window(n, alpha):
    kernel = [bessel_i0(math.pi * alpha * math.sqrt(max(0.0, 1.0 - ((i - n / 4) / (n / 4)) ** 2)))
              for i in range(n // 2 + 1)]
    total = sum(kernel)
    out, acc = [], 0.0
    for i in range(n // 2):
        acc += kernel[i]
        out.append(math.sqrt(acc / total))
    return out


def sine_window(n):
    return [math.sin(math.pi / n * (i + 0.5)) for i in range(n // 2)]


def twiddles(count, n, offset):
    out = []
    for k in range(count):
        a = 2 * math.pi * (k + offset) / n
        out += [q15(math.cos(a)), q15(-math.sin(a))]
    return out


class Writer:
    def __init__(self):
        self.c, self.h = [], []

    def array(self, ctype, name, values, per_line=12, comment=None):
        if comment:
            self.c.append("// " + comment)
        self.c.append("const %s %s[%d] = {" % (ctype, name, len(values)))
        for i in range(0, len(values), per_line):
            self.c.append("    " + ", ".join(str(v) for v in values[i:i + per_line]) + ",")
        self.c.append("};")
        self.c.append("")
        self.h.append("extern const %s %s[%d];" % (ctype, name, len(values)))


def main():
    out_dir = sys.argv[1]
    w = Writer()

    for size in SWB_LONG:
        assert len(SWB_LONG[size]) == NUM_SWB_LONG[size] + 1 and SWB_LONG[size][-1] == 1024
        assert all(a < b for a, b in zip(SWB_LONG[size], SWB_LONG[size][1:]))
    for size in SWB_SHORT:
        assert len(SWB_SHORT[size]) == NUM_SWB_SHORT[size] + 1 and SWB_SHORT[size][-1] == 128
        assert all(a < b for a, b in zip(SWB_SHORT[size], SWB_SHORT[size][1:]))

    w.array("uint32_t", "aac_sample_rates", SAMPLE_RATES)
    for size, offs in SWB_LONG.items():
        w.array("uint16_t", "aac_swb_long_%d" % size, offs, 16)
    for size, offs in SWB_SHORT.items():
        w.array("uint16_t", "aac_swb_short_%d" % size, offs, 16)
    w.c.append("const uint16_t *const aac_swb_long[12] = {")
    w.c.append("    " + ", ".join("aac_swb_long_%d" % s for s in LONG_FOR_INDEX) + ",")
    w.c.append("};")
    w.c.append("const uint16_t *const aac_swb_short[12] = {")
    w.c.append("    " + ", ".join("aac_swb_short_%d" % s for s in SHORT_FOR_INDEX) + ",")
    w.c.append("};")
    w.c.append("")
    w.h.append("extern const uint16_t *const aac_swb_long[12];")
    w.h.append("extern const uint16_t *const aac_swb_short[12];")
    w.array("uint8_t", "aac_num_swb_long", [NUM_SWB_LONG[s] for s in LONG_FOR_INDEX])
    w.array("uint8_t", "aac_num_swb_short", [NUM_SWB_SHORT[s] for s in SHORT_FOR_INDEX])
    w.array("uint8_t", "aac_tns_max_bands_long", TNS_MAX_BANDS_LONG)
    w.array("uint8_t", "aac_tns_max_bands_short", TNS_MAX_BANDS_SHORT)

    books = [("sf", cb.SF_CODES, cb.SF_LENS)] + [(b, cb.SPECTRAL[b][0], cb.SPECTRAL[b][1]) for b in range(1, 12)]
    roots, total_entries = [], 0
    for book, codes, lens in books:
        check_codebook(str(book), codes, lens)
        syms = [pack_symbol(book, i) for i in range(len(codes))]
        root, entries = build_decoder(codes, lens, syms)
        roots.append(root)
        total_entries += len(entries)
        w.array("uint16_t", "aac_huff_%s" % book, entries, 12)
    w.c.append("const uint16_t *const aac_huff_tables[12] = {")
    w.c.append("    " + ", ".join("aac_huff_%s" % b for b, _, _ in books) + ",")
    w.c.append("};")
    w.c.append("")
    w.h.append("extern const uint16_t *const aac_huff_tables[12];")
    w.array("uint8_t", "aac_huff_root_bits", roots)

    w.array("int16_t", "aac_sine_long", [q15(v) for v in sine_window(2048)], 16)
    w.array("int16_t", "aac_sine_short", [q15(v) for v in sine_window(256)], 16)
    w.array("int16_t", "aac_kbd_long", [q15(v) for v in kbd_window(2048, 4)], 16)
    w.array("int16_t", "aac_kbd_short", [q15(v) for v in kbd_window(256, 6)], 16)

    w.array("int16_t", "aac_fft_twiddle", twiddles(256, 512, 0), 16, "exp(-2pi i k/512), cos/sin pairs, Q15")
    w.array("int16_t", "aac_imdct_twiddle_long", twiddles(512, 2048, 0.125), 16, "exp(-2pi i (k+1/8)/2048), Q15")
    w.array("int16_t", "aac_imdct_twiddle_short", twiddles(64, 256, 0.125), 16, "exp(-2pi i (k+1/8)/256), Q15")
    w.array("uint16_t", "aac_bitrev512", [int(format(i, "09b")[::-1], 2) for i in range(512)], 16)

    w.array("int32_t", "aac_pow43", [int(round(i ** (4.0 / 3.0) * (1 << 17))) for i in range(1025)], 8,
            "i^(4/3), Q17")
    w.array("int16_t", "aac_pow2_quarter", [int(round(2 ** (i / 4.0) * 16384)) for i in range(4)], 4,
            "2^(i/4), Q14")
    w.array("int16_t", "aac_pow2_neg_quarter", [int(round(2 ** (-i / 4.0) * 16384)) for i in range(4)], 4,
            "2^(-i/4), Q14")
    w.array("uint16_t", "aac_inv_sqrt", [int(round((i / 64.0) ** -0.5 * 16384)) for i in range(16, 65)], 16,
            "(i/64)^(-1/2) for i = 16..64, Q14")
    for res in (3, 4):
        half = 1 << (res - 1)
        iq = (half - 0.5) / (math.pi / 2)
        iq_neg = (half + 0.5) / (math.pi / 2)
        vals = [q15(math.sin(v / (iq if v >= 0 else iq_neg))) for v in range(-half, half)]
        w.array("int16_t", "aac_tns_coef_%dbit" % res, vals, 8, "index = coef + %d, Q15" % half)

    head = ["// SPDX-License-Identifier: BSD-2-Clause", "// From tools/gen_aac_tables.py"]
    with open(os.path.join(out_dir, "tables.c"), "w", newline="\n") as f:
        f.write("\n".join(head + ['#include "tables.h"', ""] + w.c))
    with open(os.path.join(out_dir, "tables.h"), "w", newline="\n") as f:
        f.write("\n".join(head + ["#pragma once", "#include <stdint.h>", ""] + w.h + [""]))
    print("huffman entries:", total_entries)


if __name__ == "__main__":
    main()
