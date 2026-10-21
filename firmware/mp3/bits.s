// SPDX-License-Identifier: BSD-2-Clause
// Packed main-data conversion and bounded two-word prefix fetch. No product
// register, no interrupt masking. position/limit are adjacent u16 fields.
.text
// Shift an odd-address DMA byte stream into packed little-endian pairs.
// The parser ignores the unused high byte of an odd final pair. At most 721
// output words for a legal MPEG-1 frame; IRQs stay enabled, no products.
.global mp3_align_bytes
mp3_align_bytes:
    push r0
    push r1
    push r3
    mov a0l, r0
    mov r0, r1
    mov a1l, r3
    cmpv 0x0, r3
    br .Lmp3_align_done, eq
    modr [r3--]
    bkrep r3, .Lmp3_align_end
    mov [r0++], a0
    shfi a0, a0, -8
    and 0xff, a0
    mov [r0], a1
    shfi a1, a1, +8
    or a1, a0
    mov a0l, [r1++]
.Lmp3_align_end:
.Lmp3_align_done:
    pop r3
    pop r1
    pop r0
    ret always

.global mp3_swap_words
mp3_swap_words:
    push r0
    push r3
    mov a0l, r0
    mov a1l, r3
    cmpv 0x0, r3
    br .Lmp3_swap_pad, eq
    modr [r3--]
    bkrep r3, .Lmp3_swap_end
    mov [r0], a0
    shfi a0, a1, +8
    and 0xff00, a1
    shfi a0, a0, -8
    and 0xff, a0
    or a1, a0
    mov a0l, [r0++]
.Lmp3_swap_end:
.Lmp3_swap_pad:
    clr a0, always
    mov a0l, [r0++]
    mov a0l, [r0]
    pop r3
    pop r0
    ret always

.global mp3_peek_bits
mp3_peek_bits:
    push r0
    push r1
    push r2
    push r3
    push sv
    mov a0l, r0
    mov a1l, r1
    mov b0l, r3
    clr a0, always
    cmpv 0x0, r3
    br .Lmp3_peek_done, eq
    mov [r1++], a1
    mov a1, b1
    mov [r1], a0
    sub a1, a0
    cmp 0x0u8, a0
    br .Lmp3_peek_zero, le
    mov r3, a1
    mov 0x0, r2
    cmp a1, a0
    br .Lmp3_peek_available, ge
    sub a0, a1
    mov a1l, r2                 // zero-fill low missing bits
    mov a0l, r3
.Lmp3_peek_available:
    mov b1, a1
    shfi a1, a1, -4
    mov r0, a0
    add a1, a0
    mov a0l, r0
    mov [r0++], a1
    shfi a1, a1, +16
    or [r0], a1
    mov b1, a0
    and 0xf, a0
    mov a0l, sv
    shfc a1, a1, always
    shfi a1, a0, -16
    and 0xffff, a0
    mov r3, a1
    sub 0x10, a1
    mov a1l, sv
    shfc a0, a0, always
    mov r2, sv
    shfc a0, a0, always
    br .Lmp3_peek_done, always
.Lmp3_peek_zero:
    clr a0, always
.Lmp3_peek_done:
    pop sv
    pop r3
    pop r2
    pop r1
    pop r0
    ret always
// Disjoint packed-byte copy. job: destination, byte offset, source, byte
// offset, byte count. All four source/destination parities and zero lengths
// are legal. Partial destination words retain their other byte. No reads
// beyond the words containing requested source bytes; IRQs stay enabled.
.global mp3_packed_copy
mp3_packed_copy:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r7
    mov a0l, r7
    nop
    mov [r7+0x0004s7], a0
    mov a0l, r3
    cmpv 0x0, r3
    br .Lpacked_done, eq
    mov [r7+0x0001s7], a0
    mov a0, a1
    and 0x1, a1
    mov a1l, r5
    shfi a0, a0, -1
    mov [r7+0x0000s7], a1
    add a1, a0
    mov a0l, r0
    mov [r7+0x0003s7], a0
    mov a0, a1
    and 0x1, a1
    mov a1l, r4
    shfi a0, a0, -1
    mov [r7+0x0002s7], a1
    add a1, a0
    mov a0l, r1
    cmpv 0x0, r5
    br .Lpacked_pairs, eq
    mov [r1], a0
    cmpv 0x0, r4
    br .Lpacked_first_low, eq
    shfi a0, a0, -8
    modr [r1++]
.Lpacked_first_low:
    and 0xff, a0
    shfi a0, a0, +8
    mov [r0], a1
    and 0xff, a1
    or a1, a0
    mov a0l, [r0++]
    mov r4, a0
    xor 0x1, a0
    mov a0l, r4
    modr [r3--]
.Lpacked_pairs:
    mov r3, a0
    shfi a0, a0, -1
    mov a0l, r2
    cmpv 0x0, r2
    br .Lpacked_tail, eq
    modr [r2--]
    cmpv 0x0, r4
    br .Lpacked_odd, neq
    bkrep r2, .Lpacked_even_end
    mov [r1++], a0
    mov a0l, [r0++]
.Lpacked_even_end:
    br .Lpacked_tail, always
.Lpacked_odd:
    bkrep r2, .Lpacked_odd_end
    mov [r1++], a0
    shfi a0, a0, -8
    and 0xff, a0
    mov [r1], a1
    shfi a1, a1, +8
    or a1, a0
    mov a0l, [r0++]
.Lpacked_odd_end:
.Lpacked_tail:
    tstb r3, 0
    br .Lpacked_done, neq
    mov [r1], a0
    cmpv 0x0, r4
    br .Lpacked_tail_low, eq
    shfi a0, a0, -8
.Lpacked_tail_low:
    and 0xff, a0
    mov [r0], a1
    and 0xff00, a1
    or a1, a0
    mov a0l, [r0]
.Lpacked_done:
    pop r7
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always
// Checked consuming read; position/limit/bad are adjacent words. Sign bits
// (the common one-bit case) need just one source word. General reads retain
// peek's zero-fill semantics, then clamp the consumed position on truncation.
.global mp3_get_bits
mp3_get_bits:
    push r0
    push r1
    push r3
    push r4
    push sv
    mov a0l, r0
    mov a1l, r1
    mov b0l, r3
    cmpv 0x1, r3
    br .Lget_general, neq
    mov [r1++], a1
    mov [r1--], a0
    cmp a1, a0
    br .Lget_empty, le
    mov a1, b1
    add 0x1, a1
    mov a1l, [r1]
    mov b1, a0
    shfi a0, a0, -4
    mov r0, a1
    add a1, a0
    mov a0l, r0
    mov [r0], a0
    mov b1, a1
    and 0xf, a1
    mov a1l, sv
    shfc a0, a0, always
    shfi a0, a0, -15
    and 0x1, a0
    br .Lget_done, always
.Lget_empty:
    addv 0x2, r1
    mov 0x1, a0
    mov a0l, [r1]
    clr a0, always
    br .Lget_done, always
.Lget_general:
    mov r0, a0
    mov r1, a1
    mov r3, b0
    call mp3_peek_bits, always
    mov a0l, r4
    mov [r1++], a1
    mov [r1], a0
    sub a1, a0
    mov r3, b0
    cmp b0, a0
    br .Lget_available, ge
    mov [r1++], a1
    mov 0x1, a0
    mov a0l, [r1]
    addv 0xfffe, r1
    mov a1l, [r1]
    br .Lget_value, always
.Lget_available:
    add b0, a1
    modr [r1--]
    mov a1l, [r1]
.Lget_value:
    mov r4, a0
.Lget_done:
    pop sv
    pop r4
    pop r3
    pop r1
    pop r0
    ret always
