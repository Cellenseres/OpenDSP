// SPDX-License-Identifier: BSD-2-Clause
// Up to 32 pairs; Count1 stays in C.
// Job: bits, pos/limit/bad, dst32, book, end, linbits, count, scratch.
// R/Y/SV preserved; IRQs enabled, no multiplier state.
.text
.global mp3_huff_pairs
mp3_huff_pairs:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push sv
    push mod0
    rst 0x0080, mod0
    mov a0l, r7
    nop
    mov [r7+0x0000s7], a0
    mov a0l, r0
    mov [r7+0x0001s7], a0
    mov a0l, r1
    mov [r7+0x0002s7], a0
    mov a0l, r6
    mov [r7+0x0003s7], a0
    mov a0l, r5
    mov [r7+0x0006s7], a0
    mov a0l, r4
    cmpv 0x0, r4
    br .Lhuff_done, eq
.Lhuff_pair:
    mov 0x5, r2
    mov 0x0, sv
    call .Lhuff_peek, always
    mov r5, a1
    add a1, a0
.Lhuff_lookup:
    mov [r7+0x0004s7], a1
    cmp a1, a0
    br .Lhuff_bad, ge
    mov a0l, r3
    mov [r3], a1
    mov a1l, r3
    cmp 0x0u8, a1
    br .Lhuff_leaf, ge
    cmpv 0x8, sv
    br .Lhuff_bad, ge
    mov sv, a0
    add 0x1, a0
    mov a0l, sv
    call .Lhuff_drop, always
    br .Lhuff_done, eq
    mov r3, a0
    shfi a0, a0, -3
    mov a0l, [r7+0x0007s7]
    mov r3, a0
    and 0x7, a0
    mov a0l, r2
    call .Lhuff_peek, always
    mov r5, a1
    add a1, a0
    mov [r7+0x0007s7], a1
    sub a1, a0
    br .Lhuff_lookup, always
.Lhuff_leaf:
    mov r3, a0
    shfi a0, a0, -8
    mov a0l, r2
    call .Lhuff_drop, always
    br .Lhuff_done, eq
    call .Lhuff_value, always
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov a0h, [r6++]
    mov a0l, [r6++]
    mov r3, a0
    shfi a0, a0, -4
    mov a0l, r3
    call .Lhuff_value, always
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov a0h, [r6++]
    mov a0l, [r6++]
    modr [r4--]
    mov r1, r2
    addv 0x2, r2
    mov [r2], a0
    cmp 0x0u8, a0
    br .Lhuff_done, neq
    cmpv 0x0, r4
    br .Lhuff_pair, neq
    br .Lhuff_done, always
.Lhuff_bad:
    mov r1, r2
    addv 0x2, r2
    mov 0x1, a0
    mov a0l, [r2]
.Lhuff_done:
    mov [r7+0x0006s7], a0
    mov r4, a1
    sub a1, a0
    pop mod0
    pop sv
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always

.Lhuff_peek:
    // Huffman prefixes are at most seven bits. Use the known register
    // layout for complete prefixes, retaining the zero-fill reader at EOF.
    push r0
    push sv
    mov [r1++], a0
    mov [r1--], a1
    sub a0, a1
    cmp r2, a1
    br .Lhuff_peek_tail, lt
    mov a0, b0
    shfi a0, a0, -4
    mov r0, a1
    add a1, a0
    mov a0l, r0
    mov [r0++], a1
    shfi a1, a1, +16
    or [r0], a1
    mov b0, a0
    and 0xf, a0
    mov a0l, sv
    shfc a1, a1, always
    shfi a1, a0, -16
    and 0xffff, a0
    mov r2, a1
    sub 0x10, a1
    mov a1l, sv
    shfc a0, a0, always
    pop sv
    pop r0
    ret always
.Lhuff_peek_tail:
    pop sv
    pop r0
    mov r0, a0
    mov r1, a1
    mov r2, b0
    call mp3_peek_bits, always
    ret always

// Drop R2 bits. Return Z=1 on truncation, Z=0 otherwise; R0..R7/SV intact.
.Lhuff_drop:
    mov [r1++], a0
    mov [r1], a1
    sub a0, a1
    cmp r2, a1
    br .Lhuff_drop_bad, lt
    mov r2, a1
    add a1, a0
    modr [r1--]
    mov a0l, [r1]
    mov 0x1, a0
    cmp 0x0u8, a0
    ret always
.Lhuff_drop_bad:
    mov [r1++], a1
    mov 0x1, a0
    mov a0l, [r1]
    addv 0xfffe, r1
    mov a1l, [r1]
    clr a0, always
    ret always

.Lhuff_value:
    mov r3, a0
    and 0xf, a0
    mov a0l, r2
    cmp 0x0fu8, a0
    br .Lhuff_sign, neq
    mov [r7+0x0005s7], a0
    cmp 0x0u8, a0
    br .Lhuff_sign, eq
    mov a0, b0
    mov r0, a0
    mov r1, a1
    call mp3_get_bits, always
    add 0xf, a0
    mov a0l, r2
.Lhuff_sign:
    cmpv 0x0, r2
    br .Lhuff_zero, eq
    // One sign bit never crosses a word. R0 is the only live pointer
    // borrowed here; SV is dead after the leaf and saved by the caller.
    // Keep the same exhausted-reader behavior as mp3_get_bits(1).
    mov [r1++], a1
    mov [r1--], a0
    cmp a1, a0
    br .Lhuff_sign_empty, le
    mov a1, b0
    add 0x1, a1
    mov a1l, [r1]
    push r0
    mov b0, a0
    shfi a0, a0, -4
    mov r0, a1
    add a1, a0
    mov a0l, r0
    mov [r0], a0
    pop r0
    mov b0, a1
    and 0xf, a1
    mov a1l, sv
    shfc a0, a0, always
    shfi a0, a0, -15
    and 0x1, a0
    cmp 0x0u8, a0
    br .Lhuff_positive, eq
    mov r2, a0
    neg a0, always
    ret always
.Lhuff_positive:
    mov r2, a0
    ret always
.Lhuff_sign_empty:
    addv 0x2, r1
    mov 0x1, a0
    mov a0l, [r1]
    addv 0xfffe, r1
    br .Lhuff_positive, always
.Lhuff_zero:
    clr a0, always
    ret always
