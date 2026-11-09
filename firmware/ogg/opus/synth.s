// SPDX-License-Identifier: BSD-2-Clause
// CELT synthesis loops, bit-exact with libopus fixed point.

// CELT deemphasis for one channel
.section .text.opus_deemph_run
.global opus_deemph_run
opus_deemph_run:
    push mod0
    push cfgi
    push y0
    push r0
    push r1
    push r2
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    rst 0x0001, mod0
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r1
    addv 0x0002, r7
    mov [r7], cfgi
    addv 0x0001, r7
    mov [r7], y0
    mov 0x1fff, a0h
    or 0xffff, a0
    mov a0, b0
    mov 0xe000, a0h
    or 0x0001u8, a0
    mov a0, b1
    addv 0x0001, r7
    mov [r7], a1h
    addv 0x0001, r7
    or [r7], a1
    addv 0xfffc, r7
    mov [r7], r2
.Ldeemph_chunk:
    mov r2, a0
    sub 0x0050u8, a0
    brr .Ldeemph_last, le
    mov a0l, r2
    mov 0x004f, a0
    brr .Ldeemph_run, always
.Ldeemph_last:
    mov r2, a0
    sub 0x0001u8, a0
    mov 0x0000, r2
.Ldeemph_run:
    push mod3
    dint
    bkrep a0l, .Ldeemph_end
    mov [r0++], a0h
    or [r0++], a0
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov b0, a1
    cmp a1, a0
    copy a0, gt
    mov b1, a1
    cmp a1, a0
    copy a0, lt
    clr a1, always
    mpysu y0, a0l, a1
    maa y0, a0h, a1
    add p*, a1
    add 0x0800, a0
    shfi a0, a0, +4
    mov a0h, [r1++s]
.Ldeemph_end:
    pop mod3
    cmpv 0x0000, r2
    brr .Ldeemph_chunk, neq
    addv 0x0003, r7
    mov a1h, [r7]
    addv 0x0001, r7
    mov a1l, [r7]
    pop r7
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgi
    pop mod0
    ret always

// deemphasis added onto existing pcm
.section .text.opus_deemph_acc
.global opus_deemph_acc
opus_deemph_acc:
    push mod0
    push cfgi
    push y0
    push r0
    push r1
    push r2
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    rst 0x0001, mod0
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r1
    addv 0x0002, r7
    mov [r7], cfgi
    addv 0x0001, r7
    mov [r7], y0
    mov 0x1fff, a0h
    or 0xffff, a0
    mov a0, b0
    mov 0xe000, a0h
    or 0x0001u8, a0
    mov a0, b1
    addv 0x0001, r7
    mov [r7], a1h
    addv 0x0001, r7
    or [r7], a1
    addv 0xfffc, r7
    mov [r7], r2
.Ldeacc_chunk:
    mov r2, a0
    sub 0x0050u8, a0
    brr .Ldeacc_last, le
    mov a0l, r2
    mov 0x004f, a0
    brr .Ldeacc_run, always
.Ldeacc_last:
    mov r2, a0
    sub 0x0001u8, a0
    mov 0x0000, r2
.Ldeacc_run:
    push mod3
    dint
    bkrep a0l, .Ldeacc_end
    mov [r0++], a0h
    or [r0++], a0
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov b0, a1
    cmp a1, a0
    copy a0, gt
    mov b1, a1
    cmp a1, a0
    copy a0, lt
    clr a1, always
    mpysu y0, a0l, a1
    maa y0, a0h, a1
    add p*, a1
    add 0x0800, a0
    shfi a0, a0, +4
    mov a0h, sv
    mov sv, a0
    add [r1], a0
    shfi a0, a0, +16
    mov a0h, [r1++s]
.Ldeacc_end:
    pop mod3
    cmpv 0x0000, r2
    brr .Ldeacc_chunk, neq
    addv 0x0003, r7
    mov a1h, [r7]
    addv 0x0001, r7
    mov a1l, [r7]
    pop r7
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgi
    pop mod0
    ret always

// CELT band denormalisation
.section .text.opus_denorm_band
.global opus_denorm_band
opus_denorm_band:
    push mod0
    push y0
    push r0
    push r1
    push r2
    push r3
    push r7
    rst 0x0c80, mod0
    set 0x0803, mod0
    mov a0l, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r1
    addv 0x0007, r7
    mov [r7], a0
    mov a0l, r2
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r3
    addv 0xfffc, r7
    mov [r7], sv
    addv 0x0001, r7
    mov [r7], a0h
    addv 0x0001, r7
    or [r7], a0
    mov a0, b1
    addv 0xfffb, r7
    mov [r7], a0
    sub 0x0001u8, a0
    bkrep a0l, .Ldenorm_end
    push mod3
    dint
    mov [r0++], a1h
    or [r0++], a1
    shfi a1, a1, +6
    clr a0, always
    mov [r2], y0
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    mov a1h, y0
    mpysu y0, [r3], a0
    maa y0, a1l, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    add b1, a0
    shfi a0, a0, +8
    shfc a0, a0, always
    mov a0h, [r1++]
    mov a0l, [r1++]
    pop mod3
    // Hardware: no flag or short body at block end.
    nop
.Ldenorm_end:
    pop r7
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop mod0
    ret always
