// SPDX-License-Identifier: BSD-2-Clause
// CELT pitch postfilter, bit-exact with libopus fixed point.
// Taps: x[i-T] alone, x[i-T+-1] and x[i-T+-2] summed.
// Plus pointer walks down from x[i-T+2].lo, minus up from x[i-T-2].hi.

.macro comb_enter
    push mod0
    push cfgi
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov 0x0007, cfgi
.endm

.macro comb_leave
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgi
    pop mod0
.endm

// Clamp a0 to +-536870911.
.macro comb_saturate
    mov 0x1fff, a1h
    or 0xffff, a1
    cmp a1, a0
    copy a0, gt
    mov 0xe000, a1h
    or 0x0001u8, a1
    cmp a1, a0
    copy a0, lt
.endm

// a0l = {y, x, plus, minus, gains, count}
// gains = {g12, g11, g10}
.section .text.opus_comb_const
.global opus_comb_const
opus_comb_const:
    comb_enter
    mov a0l, r7
    mov [r7++], r1
    mov [r7++], r0
    mov [r7++], r2
    mov [r7++], r5
    mov [r7++], r4
    mov [r7], r6
    mov r4, r7
    modr [r6--]
    mov 0x1fff, a0h
    or 0xffff, a0
    mov a0, b0
    mov 0xe000, a0h
    or 0x0001u8, a0
    mov a0, b1
    push mod3
    dint
    bkrep r6, .Lconst_end
    mov [r0++], a0h
    or [r0++], a0
    sub 0x0001u8, a0
    mov [r2--], a1l
    addh [r2--], a1
    addh [r5++], a1
    addl [r5++], a1
    mov [r4++], y0
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    mov [r2--], a1l
    addh [r2--], a1
    addh [r5++], a1
    addl [r5--], a1
    mov [r4++], y0
    macsu y0, a1l, a0
    maa y0, a1h, a0
    mov [r2--], a1l
    addh [r2++s], a1
    mov [r4], y0
    macsu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    mov b0, a1
    cmp a1, a0
    copy a0, gt
    mov b1, a1
    cmp a1, a0
    copy a0, lt
    mov a0h, [r1++]
    mov a0l, [r1++]
    mov r7, r4
.Lconst_end:
    pop mod3
    comb_leave
    ret always

// a0l = {y, x, plus1, minus1, plus0, minus0, window, gains, count}
// gains = {g12, g11, g10, g02, g01, g00}
.section .text.opus_comb_overlap
.global opus_comb_overlap
opus_comb_overlap:
    comb_enter
    mov a0l, r6
    mov [r6++], r1
    mov [r6++], r0
    mov [r6++], r2
    mov [r6++], r5
    mov [r6++], r3
    mov [r6++], r7
    mov [r6++], r4
    mov [r6++], a1
    mov [r6], a0
    mov a1l, r6
    sub 0x0001u8, a0
    mov a0l, sv
.Loverlap_sample:
    push mod3
    dint
    // f = w*w >> 15, cf = 1 - f
    mov [r4], y0
    mpy y0, [r4++], a1
    // Hardware: p0h after multiply is stale.
    mov p*, b1
    shfi b1, b1, -16
    mov 0x7fff, b0
    sub b1, b0
    mov [r0++], a0h
    or [r0++], a0
    sub 0x0003u8, a0
    // T1 taps with f
    mov [r6++], y0
    mpy y0, b1l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    mov a1h, y0
    mov [r2--], a1l
    addh [r2--], a1
    addh [r5++], a1
    addl [r5++], a1
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    mov [r6++], y0
    mpy y0, b1l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    mov a1h, y0
    mov [r2--], a1l
    addh [r2--], a1
    addh [r5++], a1
    addl [r5--], a1
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    mov [r6++], y0
    mpy y0, b1l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    mov a1h, y0
    mov [r2--], a1l
    addh [r2++s], a1
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    // T0 taps with 1 - f
    mov [r6++], y0
    mpy y0, b0l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    mov a1h, y0
    mov [r3--], a1l
    addh [r3--], a1
    addh [r7++], a1
    addl [r7++], a1
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    mov [r6++], y0
    mpy y0, b0l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    mov a1h, y0
    mov [r3--], a1l
    addh [r3--], a1
    addh [r7++], a1
    addl [r7--], a1
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    mov [r6], y0
    mpy y0, b0l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    mov a1h, y0
    mov [r3--], a1l
    addh [r3++s], a1
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    pop mod3
    comb_saturate
    mov a0h, [r1++]
    mov a0l, [r1++]
    addv 0xfffb, r6
    mov sv, a0
    sub 0x0001u8, a0
    mov a0l, sv
    br .Loverlap_sample, ge
    comb_leave
    ret always
