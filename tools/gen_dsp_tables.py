#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Convert AAC tables to bank-1 DSP words, with 32-bit values high word first."""
import os
import re
import sys

SKIP = {'aac_sample_rates', 'aac_fft_twiddle', 'aac_bitrev512'}


def parse(path):
    text = open(path).read()
    arrays = {}
    for m in re.finditer(r'const (u?int\d+_t) (aac_\w+)\[(\d+)\] = \{(.*?)\};', text, re.S):
        ctype, name, count, body = m.groups()
        values = [int(v) for v in re.findall(r'-?\d+', re.sub(r'//[^\n]*', '', body))]
        assert len(values) == int(count), name
        arrays[name] = (ctype, values)
    pointers = {}
    for m in re.finditer(r'const uint16_t \*const (aac_\w+)\[(\d+)\] = \{(.*?)\};', text, re.S):
        pointers[m.group(1)] = re.findall(r'aac_\w+', m.group(3))
    return arrays, pointers


def emit_array(out, name, words, signed):
    attr = ' __attribute__((section(".aac_tables")))'
    ctype = 's16' if signed else 'u16'
    out.append('const %s d%s[%d]%s = {' % (ctype, name, len(words), attr))
    for i in range(0, len(words), 16):
        out.append('    ' + ', '.join(str(v) for v in words[i:i + 16]) + ',')
    out.append('};')
    out.append('')


def main():
    arrays, pointers = parse(sys.argv[1])
    out_dir = sys.argv[2]
    c = ['// SPDX-License-Identifier: BSD-2-Clause',
         '// From gen_dsp_tables.py: library/source/aac/tables.c in DSP form.',
         '#include "aac_tables_dsp.h"', '']
    h = ['// SPDX-License-Identifier: BSD-2-Clause',
         '// From gen_dsp_tables.py.', '#pragma once', '#include "hw.h"', '']
    for name, (ctype, values) in arrays.items():
        if name in SKIP:
            continue
        if ctype == 'int32_t':          # high word first
            words = []
            for v in values:
                v &= 0xFFFFFFFF
                words += [v >> 16, v & 0xFFFF]
            emit_array(c, name, words, False)
            h.append('extern const u16 d%s[%d];   // s32 pairs, high first' % (name, len(words)))
            continue
        signed = ctype.startswith('int')
        emit_array(c, name, values, signed)
        h.append('extern const %s d%s[%d];' % ('s16' if signed else 'u16', name, len(values)))
    # FFT twiddles per stage: size s at word s - 4, s/2 (cos, sin) pairs
    fft = arrays['aac_fft_twiddle'][1]
    stage = []
    size = 4
    while size <= 512:
        step = 512 // size
        for j in range(size // 2):
            stage += [fft[2 * j * step], fft[2 * j * step + 1]]
        size *= 2
    emit_array(c, '_aac_fft_stage_twiddle', stage, True)
    h.append('extern const s16 d_aac_fft_stage_twiddle[%d];' % len(stage))
    bitrev = arrays['aac_bitrev512'][1]
    emit_array(c, '_aac_head_bitrev', [4 * bitrev[2 * i] for i in range(256)], False)
    h.append('extern const u16 d_aac_head_bitrev[256];')
    # pointer tables become index tables into the band offset arrays
    for name, targets in pointers.items():
        h.append('// %s: %s' % (name, ', '.join(targets)))
    open(os.path.join(out_dir, 'aac_tables_dsp.c'), 'w', newline='\n').write('\n'.join(c))
    open(os.path.join(out_dir, 'aac_tables_dsp.h'), 'w', newline='\n').write('\n'.join(h) + '\n')
    total = sum(len(v) * (2 if t == 'int32_t' else 1) for n, (t, v) in arrays.items() if n not in SKIP)
    print('%d words + %d stage twiddles + 256 bitrev' % (total, len(stage)))


if __name__ == '__main__':
    main()
