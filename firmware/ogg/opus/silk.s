// SPDX-License-Identifier: BSD-2-Clause
// SILK decoder loops, bit-exact with libopus fixed point.

// one allpass section, b1 in, a0 out
.macro up2_pass c
    mov \c, y0
    mov b1, a0
    subh [r0++], a0
    subl [r0--], a0
    clr a1, always
    mpysu y0, a0l, a1
    maa y0, a0h, a1
    add p*, a1
    mov a1, a0
    addh [r0++], a0
    addl [r0--], a0
    add b1, a1
    mov a1h, [r0++]
    mov a1l, [r0++]
    mov a0, b1
.endm

// section with coefficient offset by one
.macro up2_last c
    mov \c, y0
    mov b1, a0
    subh [r0++], a0
    subl [r0--], a0
    mov a0, a1
    mpysu y0, a0l, a1
    maa y0, a0h, a1
    add p*, a1
    mov a1, a0
    addh [r0++], a0
    addl [r0--], a0
    add b1, a1
    mov a1h, [r0++]
    mov a1l, [r0++]
.endm

.macro up2_out
    add 0x0200, a0
    shfi a0, a0, +6
    lim a0, a0
    mov a0h, [r1++]
.endm

// S, out, in, len
.section .text.opus_s_up2
.global opus_s_up2
opus_s_up2:
    push r7
    push y0
    push r0
    push r1
    push r2
    push r3
    mov a0l, r3
    mov a1l, r1
    mov b0l, r2
    mov b1l, r7
    cmpv 0x0000, r7
    br .Lup2_done, eq
.Lup2_chunk:
    mov r7, a1
    sub 0x0020u8, a1
    brr .Lup2_last, le
    mov a1l, r7
    mov 0x001f, a1
    brr .Lup2_run, always
.Lup2_last:
    mov r7, a1
    sub 0x0001u8, a1
    mov 0x0000, r7
.Lup2_run:
    bkrep a1l, .Lup2_end
    push mod3
    dint
    mov r3, r0
    mov [r2], a0
    shfi a0, a0, +10
    mov a0, b1
    up2_pass 0x06d2
    up2_pass 0x3a8a
    up2_last 0x98ab
    up2_out
    mov [r2++], a0
    shfi a0, a0, +10
    mov a0, b1
    up2_pass 0x1ac6
    up2_pass 0x64a9
    up2_last 0xd8f6
    up2_out
    pop mod3
    // Hardware: no flag or short body at block end.
    nop
.Lup2_end:
    call ogg_opus_poll, always
    cmpv 0x0000, r7
    br .Lup2_chunk, neq
.Lup2_done:
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop r7
    ret always

// block: out buf table maxhi maxlo inchi inclo
.section .text.opus_s_firint
.global opus_s_firint
opus_s_firint:
    push r7
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    mov a0l, r5
    mov [r5++], r2
    mov [r5++], r6
    mov [r5++], r3
    mov [r5++], a1h
    or [r5++], a1
    mov [r5++], a0h
    or [r5++], a0
    mov a0, b1
    mov r3, a0
    add r3, a0
    add 0x002fu8, a0
    mov a0l, r5
    clr b0, always
    mov 0x0040, r7
    cmp a1, b0
    br .Lfi_done, ge
.Lfi_loop:
    push mod3
    dint
    mov 0x000c, y0
    mpysu y0, b0l, a0
    // Hardware: p0h after multiply is stale.
    mov p*, a0
    shfi a0, a0, -16
    shfi a0, a0, +2
    add r3, a0
    mov a0l, r1
    neg a0, always
    add r5, a0
    mov a0l, r4
    mov b0h, a0l
    add r6, a0
    mov a0l, r0
    clr a0, always
    mov [r1++], y0
    mpy y0, [r0++], a0
    mov [r1++], y0
    mac y0, [r0++], a0
    mov [r1++], y0
    mac y0, [r0++], a0
    mov [r1++], y0
    mac y0, [r0++], a0
    mov [r4--], y0
    mac y0, [r0++], a0
    mov [r4--], y0
    mac y0, [r0++], a0
    mov [r4--], y0
    mac y0, [r0++], a0
    mov [r4--], y0
    mac y0, [r0++], a0
    add p*, a0
    pop mod3
    add 0x4000, a0
    shfi a0, a0, +1
    lim a0, a0
    mov a0h, [r2++]
    add b1, b0
    addv 0xffff, r7
    cmpv 0x0000, r7
    brr .Lfi_more, neq
    call ogg_opus_poll, always
    mov 0x0040, r7
.Lfi_more:
    cmp a1, b0
    br .Lfi_loop, lt
.Lfi_done:
    mov r2, a0
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop r7
    ret always

// block: exc pulses n-1 seedhi seedlo offset
.section .text.opus_s_exc
.global opus_s_exc
opus_s_exc:
    push y0
    push r1
    push r2
    push r3
    push r5
    mov a0l, r3
    mov [r3++], r2
    mov [r3++], r1
    mov [r3++], a1
    mov [r3++], a0h
    or [r3++], a0
    mov a0, b0
    mov [r3], r5
    bkrep a1l, .Lex_end
    push mod3
    dint
    mov 0x8435, y0
    mpysu y0, b0l, a1
    mov p*, b1
    mpy y0, b0h, a1
    mov p*, a1
    mov 0x0bb4, y0
    mpysu y0, b0l, a0
    add p*, a1
    pop mod3
    add 0x3619, a1
    shfi a1, a1, +16
    add b1, a1
    add 0x636b, a1
    shfi a1, a1, +8
    shfi a1, a1, -8
    mov a1, b0
    mov [r1], a0
    brr .Lex_zero, eq
    shfi a0, a0, +14
    brr .Lex_neg, lt
    sub 0x0500, a0
    brr .Lex_zero, always
.Lex_neg:
    add 0x0500, a0
.Lex_zero:
    add r5, a0
    cmp 0x0000u8, a1
    neg a0, lt
    mov a0h, [r2++]
    mov a0l, [r2++]
    mov [r1++], a0
    add a0, b0
.Lex_end:
    call ogg_opus_poll, always
    pop r5
    pop r3
    pop r2
    pop r1
    pop y0
    ret always

// block: pred b pexc pres ltp n-1
.section .text.opus_s_ltp
.global opus_s_ltp
opus_s_ltp:
    push cfgi
    push cfgj
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    mov a0l, r3
    mov [r3++], r4
    modr [r4++]
    mov [r3++], r0
    mov [r3++], r1
    mov [r3++], r2
    mov [r3++], a1
    mov [r3], a0
    mov a1l, r3
    mov 0x007c, a1
    mov a1l, cfgi
    mov 0x000b, a1
    mov a1l, cfgj
    bkrep a0l, .Lltp_end
    push mod3
    dint
    mov 0x0002, a0
    mov [r0++], y0
    mpysu y0, [r4--], a0
    maa y0, [r4--], a0
    mov [r0++], y0
    macsu y0, [r4--], a0
    maa y0, [r4--], a0
    mov [r0++], y0
    macsu y0, [r4--], a0
    maa y0, [r4--], a0
    mov [r0++], y0
    macsu y0, [r4--], a0
    maa y0, [r4--], a0
    mov [r0++s], y0
    macsu y0, [r4--], a0
    maa y0, [r4++s], a0
    add p*, a0
    pop mod3
    shfi a0, a0, +1
    addh [r1++], a0
    addl [r1++], a0
    mov a0h, [r2++]
    mov a0l, [r2++]
    shfi a0, a0, +1
    mov a0h, [r3++]
    mov a0l, [r3++]
.Lltp_end:
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgj
    pop cfgi
    ret always

// block: s a pres xq gainlo gainhi order-3 n-1 step order/2
.section .text.opus_s_lpc
.global opus_s_lpc
opus_s_lpc:
    push cfgi
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    call ogg_opus_poll, always
    mov a0l, r3
    mov [r3++], r2
    mov [r3++], r0
    mov [r3++], r1
    mov [r3++], r6
    mov r3, r4
    addv 0x0002, r4
    mov [r4++], r5
    mov [r4++], a1
    mov [r4++], a0
    mov a0l, cfgi
    mov [r4], sv
    mov 0x07ff, a0h
    or 0xffff, a0
    mov a0, b0
    mov 0xf800, a0h
    mov a0, b1
    bkrep a1l, .Llpc_end
    push mod3
    dint
    mov r2, r4
    modr [r4--]
    mov sv, a0
    mov [r0++], y0
    mpysu y0, [r4--], a0
    maa y0, [r4--], a0
    bkrep r5, .Llpc_tap
    mov [r0++], y0
    macsu y0, [r4--], a0
    maa y0, [r4--], a0
.Llpc_tap:
    mov [r0++s], y0
    macsu y0, [r4--], a0
    maa y0, [r4--], a0
    add p*, a0
    cmp b0, a0
    brr .Llpc_slow, gt
    cmp b1, a0
    brr .Llpc_fast, ge
.Llpc_slow:
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov b0, a1
    cmp a1, a0
    copy a0, gt
    mov b1, a1
    cmp a1, a0
    copy a0, lt
.Llpc_fast:
    shfi a0, a0, +4
    addh [r1++], a0
    addl [r1++], a0
    lim a0, a0
    mov a0h, [r2++]
    mov a0l, [r2]
    clr a0, always
    mov [r3++], y0
    mpysu y0, [r2--], a0
    maa y0, [r2++], a0
    mov [r3--], y0
    macsu y0, [r2--], a0
    mac y0, [r2++], a0
    mov p*, a1
    shfi a1, a1, +16
    add a1, a0
    pop mod3
    modr [r2++]
    shfi a0, a0, +8
    rnd a0, always
    not a0, v
    lim a0, a0
    mov a0h, [r6++]
.Llpc_end:
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop cfgi
    ret always

// block: out in b n-1 d-3 stepb stepin
.section .text.opus_s_ana
.global opus_s_ana
opus_s_ana:
    push cfgi
    push cfgj
    push y0
    push r0
    push r2
    push r4
    push r5
    mov a0l, r4
    mov [r4++], r2
    mov [r4++], r5
    mov [r4++], r0
    mov [r4++], a1
    mov [r4++], a0
    mov [r4++], b0
    mov b0l, cfgi
    mov [r4], b0
    mov b0l, cfgj
    mov r5, r4
    mov a0l, r5
    bkrep a1l, .Lana_end
    push mod3
    dint
    clr a0, always
    mov [r0++], y0
    mpy y0, [r4--], a0
    bkrep r5, .Lana_tap
    mov [r0++], y0
    mac y0, [r4--], a0
.Lana_tap:
    mov [r0++s], y0
    mac y0, [r4++s], a0
    add p*, a0
    pop mod3
    mov [r4], a1
    shfi a1, a1, +12
    sub a0, a1
    shfi a1, a1, +8
    shfi a1, a1, -8
    add 0x0800, a1
    shfi a1, a1, +4
    lim a1, a1
    mov a1h, [r2++]
.Lana_end:
    call ogg_opus_poll, always
    pop r5
    pop r4
    pop r2
    pop r0
    pop y0
    pop cfgj
    pop cfgi
    ret always

// tmpA - frac(tmpB, rc), times mult, shifted
// r2 tmpA, r5 tmpB, r3 temp, r4 rc low, r6 mult low
.section .text.opus_s_gain_row
.Lig_term:
    clr a0, always
    mov [r4--], y0
    modr [r5++]
    mpysu y0, [r5--], a0
    maa y0, [r5++], a0
    mov [r4++], y0
    macsu y0, [r5--], a0
    mac y0, [r5], a0
    add 0x4000, a0
    shfi a0, a0, -15
    mov p*, a1
    shfi a1, a1, +1
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov [r2++], a1h
    or [r2--], a1
    sub a0, a1
    lim a1, a1
    mov a1h, [r3++]
    mov a1l, [r3]
    mov [r6--], y0
    macuu y0, [r3--], a1
    // Hardware: p0h after multiply is stale.
    mov p*, a0
    shfi a0, a0, -16
    macus y0, [r3++], a1
    mov [r6++], y0
    macsu y0, [r3--], a0
    mac y0, [r3], a0
    modr [r6++]
    addl [r6++], a0
    mov [r6++], sv
    shfc a0, a0, always
    mov [r6], sv
    addv 0xfffd, r6
    mov p*, a1
    shfc a1, a1, always
    add a1, a0
    ret always

// block: a k rchi rclo mhi mlo round -s 16-s temp temp
.global opus_s_gain_row
opus_s_gain_row:
    push r7
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    mov a0l, r3
    mov [r3++], r0
    mov [r3++], b0
    mov r3, r4
    modr [r4++]
    mov r3, r6
    addv 0x0003, r6
    addv 0x0007, r3
    mov b0, a0
    mov r0, a1
    add b0, a1
    add b0, a1
    sub 0x0002u8, a1
    mov a1l, r1
    add 0x0001u8, a0
    shfi a0, a0, -1
    mov a0l, r7
.Lig_pair:
    push mod3
    dint
    mov r0, r2
    mov r1, r5
    call .Lig_term, always
    brr .Lig_bad, e
    mov a0, b0
    mov r1, r2
    mov r0, r5
    call .Lig_term, always
    brr .Lig_bad, e
    pop mod3
    mov b0h, [r0++]
    mov b0l, [r0++]
    mov a0h, [r1++]
    mov a0l, [r1]
    addv 0xfffd, r1
    addv 0xffff, r7
    cmpv 0x0000, r7
    brr .Lig_pair, neq
    clr a0, always
.Lig_out:
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop r7
    ret always
.Lig_bad:
    pop mod3
    mov 0x0001, a0
    brr .Lig_out, always

// block: x1 x2 pred0 pred1 delta0 delta1 n-1
.section .text.opus_s_ms
.global opus_s_ms
opus_s_ms:
    push y0
    push r0
    push r1
    push r5
    push r6
    mov a0l, r5
    mov [r5++], r0
    mov [r5++], r1
    mov [r5++], r6
    mov [r5++], a1
    mov [r5++], b0
    mov [r5++], b1
    mov [r5], a0
    mov a1l, r5
    bkrep a0l, .Lms_end
    mov r6, a0
    add b0, a0
    mov a0l, r6
    mov r5, a0
    add b1, a0
    mov a0l, r5
    mov [r0++], a0
    mov [r0++], a1
    add [r0--], a0
    add a1, a0
    add a1, a0
    shfi a0, a0, +9
    mov [r1], a1
    shfi a1, a1, +8
    push mod3
    dint
    mov r6, y0
    mpysu y0, a0l, a1
    maa y0, a0h, a1
    add p*, a1
    mov [r0], a0
    shfi a0, a0, +11
    mov r5, y0
    mpysu y0, a0l, a1
    maa y0, a0h, a1
    add p*, a1
    pop mod3
    add 0x0080u8, a1
    shfi a1, a1, +8
    lim a1, a1
    mov a1h, [r1++]
.Lms_end:
    pop r6
    pop r5
    pop r1
    pop r0
    pop y0
    ret always

// x1, x2, n-1: mid side to left right
.section .text.opus_s_lr
.global opus_s_lr
opus_s_lr:
    call ogg_opus_poll, always
    push r0
    push r1
    mov a0l, r0
    mov a1l, r1
    bkrep b0l, .Llr_end
    mov [r0], a0
    mov a0, a1
    add [r1], a0
    sub [r1], a1
    shfi a0, a0, +16
    lim a0, a0
    mov a0h, [r0++]
    shfi a1, a1, +16
    lim a1, a1
    mov a1h, [r1++]
.Llr_end:
    pop r1
    pop r0
    ret always

// out, cLSF, dd: NLSF polynomial
.section .text.opus_s_poly
.Lp_mul:
    mov r6, y0
    modr [r2++]
    mpysu y0, [r2--], a0
    mov p*, a0
    rnd a0, always
    shfi a0, a0, -16
    mpy y0, [r2++], a1
    mov r5, y0
    macsu y0, [r2--], a0
    mac y0, [r2], a0
    mov p*, a1
    shfi a1, a1, +16
    add a1, a0
    ret always

.global opus_s_poly
opus_s_poly:
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    mov a0l, r0
    mov a1l, r1
    mov b0l, r7
    addv 0xffff, r7
    mov r0, r2
    mov 0x0001, a0
    mov a0l, [r2++]
    clr a0, always
    mov a0l, [r2++]
    mov [r1++], a0h
    or [r1--], a0
    neg a0, always
    mov a0h, [r2++]
    mov a0l, [r2]
    mov r0, r3
    addv 0x0002, r3
    clr b0, always
    cmpv 0x0000, r7
    br .Lp_done, eq
.Lp_k:
    addv 0x0004, r1
    mov [r1++], a0h
    or [r1--], a0
    mov a0, b1
    rnd a0, always
    mov a0h, r5
    mov b1l, r6
    mov b0l, r4
    mov r3, r2
    push mod3
    dint
    call .Lp_mul, always
    pop mod3
    addv 0xfffe, r2
    mov [r2++], a1h
    or [r2++], a1
    shfi a1, a1, +1
    sub a0, a1
    addv 0x0002, r2
    mov a1h, [r2++]
    mov a1l, [r2]
    addv 0xfffb, r2
    cmpv 0x0000, r4
    brr .Lp_tail, eq
.Lp_n:
    push mod3
    dint
    call .Lp_mul, always
    pop mod3
    addv 0x0002, r2
    mov [r2++], a1h
    or [r2--], a1
    sub a0, a1
    addv 0xfffc, r2
    addh [r2++], a1
    addl [r2--], a1
    addv 0x0004, r2
    mov a1h, [r2++]
    mov a1l, [r2]
    addv 0xfffb, r2
    addv 0xffff, r4
    cmpv 0x0000, r4
    brr .Lp_n, neq
.Lp_tail:
    mov r0, r2
    addv 0x0002, r2
    mov [r2++], a1h
    or [r2--], a1
    sub b1, a1
    mov a1h, [r2++]
    mov a1l, [r2]
    addv 0x0002, r3
    mov b0, a0
    add 0x0001u8, a0
    mov a0, b0
    addv 0xffff, r7
    cmpv 0x0000, r7
    br .Lp_k, neq
.Lp_done:
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    ret always
