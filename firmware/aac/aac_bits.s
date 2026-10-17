// SPDX-License-Identifier: BSD-2-Clause
// AAC bit reader. State at 0x0FF0 (aac_dsp.h): bit position, buffer address.
// The frame is big-endian 16-bit words (aac_bs_swap after the DMA). Word
// indices wrap at 1024, so no read leaves the buffer, whatever the stream says.
.text

// void aac_bs_swap(u16 words): byte-swaps the first words of the buffer
.global aac_bs_swap
aac_bs_swap:
    push    r0
    push    r3
    mov     a0l, r3
    modr    [r3--]
    mov     0x0ff1, r0
    mov     [r0], r0
    bkrep   r3, .Lsw_end
    mov     [r0], a0
    shfi    a0, a1, +8
    and     0xff00, a1              // low byte up
    shfi    a0, a0, -8
    and     0x00ff, a0              // high byte down
    or      a1, a0
    mov     a0l, [r0++]
.Lsw_end:
    pop     r3
    pop     r0
    ret     always

// u16 aac_getbits(u16 n): the next n bits, 1 <= n <= 16
.global aac_getbits
aac_getbits:
    push    r0
    push    sv
    mov     a0, b1                  // n
    mov     0x0ff0, r0
    mov     [r0], a1
    add     a1, a0
    mov     a0l, [r0]               // pos += n
    mov     a1, b0                  // old pos
    shfi    a1, a1, -4
    and     0x03ff, a1
    mov     0x0ff1, r0
    add     [r0], a1
    mov     a1l, r0
    mov     [r0++], a1
    shfi    a1, a1, +16
    or      [r0], a1                // two words around pos
    mov     b0, a0
    and     0x000f, a0
    mov     a0l, sv
    shfc    a1, a1, always          // the 16 bits from pos in bits 31..16
    shfi    a1, a0, -16
    and     0xffff, a0
    mov     b1, a1
    sub     0x10, a1
    mov     a1l, sv
    shfc    a0, a0, always          // >> (16 - n)
    pop     sv
    pop     r0
    ret     always

// u16 aac_huff(u16 table): one symbol. Entries: leaf length << 11 | symbol,
// node 0x8000 | bits << 11 | offset of the subtable; the root takes 8 bits.
.global aac_huff
aac_huff:
    push    r0
    push    r1
    push    r2
    push    sv
    mov     a0l, r2                 // table
    mov     a0l, r1                 // current (sub)table
    mov     0x8, b1                 // bits it takes
.Lhf_next:
    mov     0x0ff0, r0
    mov     [r0], a1
    mov     a1, b0
    shfi    a1, a1, -4
    and     0x03ff, a1
    mov     0x0ff1, r0
    add     [r0], a1
    mov     a1l, r0
    mov     [r0++], a1
    shfi    a1, a1, +16
    or      [r0], a1
    mov     b0, a0
    and     0x000f, a0
    mov     a0l, sv
    shfc    a1, a1, always
    shfi    a1, a0, -16
    and     0xffff, a0
    mov     b1, a1
    sub     0x10, a1
    mov     a1l, sv
    shfc    a0, a0, always          // index
    mov     r1, a1
    add     a1, a0
    mov     a0l, r0
    mov     [r0], a0                // entry, sign-extended: nodes are negative
    br      .Lhf_leaf, ge
    mov     0x0ff0, r0          // node: skip the bits, go to the subtable
    mov     [r0], a1
    add     b1, a1
    mov     a1l, [r0]
    mov     a0, a1
    shfi    a1, a1, -11
    and     0x000f, a1
    mov     a1, b1
    and     0x07ff, a0              // subtable offset, from the table start
    mov     r2, a1
    add     a1, a0
    mov     a0l, r1
    br      .Lhf_next, always
.Lhf_leaf:
    mov     0x0ff0, r0
    mov     [r0], a1
    mov     a0, b0
    shfi    b0, b0, -11
    add     b0, a1
    mov     a1l, [r0]               // pos += length
    and     0x07ff, a0
    pop     sv
    pop     r2
    pop     r1
    pop     r0
    ret     always

// u16 aac_decode_band(const u16 *job): type, table, dst, count
// Spectral data of one band, as decode_band() in syntax.c: values as s32 at
// dst. Returns 0, or 3 (corrupt) for an escape prefix longer than 8 bits.
// The bit position lives in r6 meanwhile. Window (a1): the 16 bits from the
// position in bits 15..0; bits above are spent ones, so shifts never overflow.
// frame [r7+]: 0 buffer, 1 table, 2 type, 3 values left, 4 y sign, 5 z sign,
// 6 status, 7 bits of this lookup, 8 lookup base, 9 escape prefix
.global aac_decode_band
aac_decode_band:
    push    r0
    push    r1
    push    r6
    push    r7
    push    sv
    mov     a0l, r6
    addv    0xfff6, sp
    mov     sp, r7
    nop
    mov     [r6++], a0
    mov     a0l, [r7+0x0002s7]      // type
    mov     [r6++], a0
    mov     a0l, [r7+0x0001s7]      // table
    mov     [r6++], r1              // dst
    mov     [r6], a0
    mov     a0l, [r7+0x0003s7]      // count
    mov     0x0ff1, r0
    mov     [r0], a0
    mov     a0l, [r7+0x0000s7]      // bit buffer
    mov     0x0ff0, r0
    mov     [r0], r6                // position
    clr     a0, always
    mov     a0l, [r7+0x0006s7]
    mov     [r7+0x0002s7], a0
    cmp     0x4, a0
    br      .Ldb_quads, le

.Ldb_pair:
    call    .Ldb_cw, always
    mov     a0, b1                  // symbol
    mov     [r7+0x0002s7], a0
    cmp     0xb, a0
    br      .Ldb_split4, neq
    mov     b1, a0
    shfi    a0, a0, -5
    mov     a0, b0                  // y
    mov     b1, a0
    and     0x001f, a0
    mov     a0, b1                  // z
    br      .Ldb_split_done, always
.Ldb_split4:
    mov     b1, a0
    shfi    a0, a0, -4
    mov     a0, b0
    mov     b1, a0
    and     0x000f, a0
    mov     a0, b1
.Ldb_split_done:
    mov     [r7+0x0002s7], a0
    cmp     0x6, a0
    br      .Ldb_unsigned, gt
    mov     b0, a0                  // books 5, 6: -4 .. 4
    sub     0x4, a0
    mov     a0, b0
    mov     b1, a0
    sub     0x4, a0
    mov     a0, b1
    br      .Ldb_pair_store, always
.Ldb_unsigned:
    clr     a0, always
    mov     a0l, [r7+0x0004s7]
    mov     a0l, [r7+0x0005s7]
    mov     b0, a0
    br      .Ldb_no_sy, eq
    call    .Ldb_bit, always
    mov     a0l, [r7+0x0004s7]
.Ldb_no_sy:
    mov     b1, a0
    br      .Ldb_no_sz, eq
    call    .Ldb_bit, always
    mov     a0l, [r7+0x0005s7]
.Ldb_no_sz:
    mov     [r7+0x0002s7], a0
    cmp     0xb, a0
    br      .Ldb_signs, neq
    mov     b0, a0                  // book 11: escapes after the signs
    call    .Ldb_esc, always
    mov     a0, b0
    mov     b1, a0
    call    .Ldb_esc, always
    mov     a0, b1
    mov     [r7+0x0006s7], a0
    br      .Ldb_done, neq
.Ldb_signs:
    mov     [r7+0x0004s7], a0
    br      .Ldb_y_done, eq
    mov     b0, a0
    neg     a0, always
    mov     a0, b0
.Ldb_y_done:
    mov     [r7+0x0005s7], a0
    br      .Ldb_pair_store, eq
    mov     b1, a0
    neg     a0, always
    mov     a0, b1
.Ldb_pair_store:
    mov     b0, a0
    mov     a0h, [r1++]
    mov     a0l, [r1++]
    mov     b1, a0
    mov     a0h, [r1++]
    mov     a0l, [r1++]
    mov     [r7+0x0003s7], a0
    sub     0x2, a0
    mov     a0l, [r7+0x0003s7]
    br      .Ldb_pair, gt
    br      .Ldb_done, always

.Ldb_quads:
    call    .Ldb_cw, always
    mov     a0, b1                  // symbol: four 2-bit values
    shfi    a0, a0, -6
    and     0x0003, a0
    call    .Ldb_qval, always
    mov     b1, a0
    shfi    a0, a0, -4
    and     0x0003, a0
    call    .Ldb_qval, always
    mov     b1, a0
    shfi    a0, a0, -2
    and     0x0003, a0
    call    .Ldb_qval, always
    mov     b1, a0
    and     0x0003, a0
    call    .Ldb_qval, always
    mov     [r7+0x0003s7], a0
    sub     0x4, a0
    mov     a0l, [r7+0x0003s7]
    br      .Ldb_quads, gt

.Ldb_done:
    mov     0x0ff0, r0
    mov     r6, [r0]
    mov     [r7+0x0006s7], a0
    addv    0x000a, sp
    pop     sv
    pop     r7
    pop     r6
    pop     r1
    pop     r0
    ret     always

// one quad value in a0: books 1, 2 signed (v - 1), books 3, 4 sign bit
.Ldb_qval:
    mov     a0, b0
    mov     [r7+0x0002s7], a0
    cmp     0x2, a0
    br      .Ldb_qv_unsigned, gt
    mov     b0, a0
    sub     0x1, a0
    br      .Ldb_qv_store, always
.Ldb_qv_unsigned:
    mov     b0, a0
    br      .Ldb_qv_store, eq
    call    .Ldb_bit, always
    br      .Ldb_qv_positive, eq
    mov     b0, a0
    neg     a0, always
    br      .Ldb_qv_store, always
.Ldb_qv_positive:
    mov     b0, a0
.Ldb_qv_store:
    mov     a0h, [r1++]
    mov     a0l, [r1++]
    ret     always

// a1 = the 16 bits at r6 in bits 15..0 (a0, r0, sv clobbered)
.Ldb_peek:
    mov     r6, a0
    shfi    a0, a0, -4
    and     0x03ff, a0
    add     [r7+0x0000s7], a0
    mov     a0l, r0
    mov     [r0++], a1
    and     0xffff, a1
    shfi    a1, a1, +16
    or      [r0], a1
    mov     r6, a0
    and     0x000f, a0
    sub     0x0010, a0
    mov     a0l, sv
    shfc    a1, a1, always
    ret     always

// a0 = next bit from the window (flags set), window and position advance
.Ldb_bit:
    mov     a1, a0
    shfi    a0, a0, -15
    shfi    a1, a1, +1
    modr    [r6++]
    and     0x0001, a0
    ret     always

// a0 = next Huffman symbol, a1 = fresh window behind it; b0 clobbered
.Ldb_cw:
    mov     0x8, a0
    mov     a0l, [r7+0x0007s7]
    mov     [r7+0x0001s7], a0
    mov     a0l, [r7+0x0008s7]
    call    .Ldb_peek, always
    mov     a1, a0
    shfi    a0, a0, -8
    and     0x00ff, a0
.Ldb_cw_lookup:
    add     [r7+0x0008s7], a0
    mov     a0l, r0
    mov     [r0], a0                // entry, sign-extended: nodes negative
    br      .Ldb_cw_node, lt
    mov     a0, b0
    shfi    a0, a0, -11             // length
    mov     r6, a1
    add     a0, a1
    mov     a1l, r6
    mov     b0, a0
    and     0x07ff, a0
    mov     a0, b0
    call    .Ldb_peek, always
    mov     b0, a0
    ret     always
.Ldb_cw_node:
    mov     a0, b0
    mov     [r7+0x0007s7], a1       // skip the bits of this level
    mov     r6, a0
    add     a1, a0
    mov     a0l, r6
    mov     b0, a0
    shfi    a0, a0, -11
    and     0x000f, a0
    mov     a0l, [r7+0x0007s7]
    mov     b0, a0
    and     0x07ff, a0
    add     [r7+0x0001s7], a0
    mov     a0l, [r7+0x0008s7]
    call    .Ldb_peek, always
    mov     [r7+0x0007s7], a0
    sub     0x0010, a0
    mov     a0l, sv
    and     0xffff, a1
    shfc    a1, a1, always
    mov     a1, a0
    br      .Ldb_cw_lookup, always

// a0 = escaped value (unchanged unless 16); a failed escape sets the status
.Ldb_esc:
    cmp     0x10, a0
    br      .Ldb_esc_none, neq
    call    .Ldb_peek, always
    clr     a0, always
    mov     a0l, [r7+0x0009s7]
.Ldb_esc_prefix:
    call    .Ldb_bit, always
    br      .Ldb_esc_value, eq
    mov     [r7+0x0009s7], a0
    add     0x1, a0
    mov     a0l, [r7+0x0009s7]
    cmp     0x8, a0
    br      .Ldb_esc_prefix, le
    mov     0x3, a0
    mov     a0l, [r7+0x0006s7]
    clr     a0, always
    ret     always
.Ldb_esc_value:
    call    .Ldb_peek, always
    mov     [r7+0x0009s7], a0
    add     0x4, a0                 // n + 4 bits
    sub     0x0010, a0
    mov     a0l, sv
    and     0xffff, a1
    shfc    a1, a1, always
    mov     a1l, [r7+0x0007s7]      // the bits
    mov     [r7+0x0009s7], a0
    add     0x4, a0
    mov     r6, a1
    add     a0, a1
    mov     a1l, r6
    mov     a0l, sv
    mov     0x1, a0
    shfc    a0, a0, always
    add     [r7+0x0007s7], a0       // (1 << (n + 4)) + bits
.Ldb_esc_none:
    ret     always
