// SPDX-License-Identifier: BSD-2-Clause
// Small CELT loops, task environment only.

// MULT16_32_Q15 over N words pairs
.section .text.opus_k_scale
.global opus_k_scale
opus_k_scale:
    push y0
    push r0
    push r1
    push cfgi
    mov a0l, y0
    mov a1l, r0
    addv 0x0001, r0
    mov b0l, r1
    mov b1, a1
    sub 0x0001u8, a1
    mov 0x0003, a0
    mov a0l, cfgi
    push mod3
    dint
    set 0x0800, mod0
    bkrep a1l, .Lscale_end
    clr a0, always
    mpysu y0, [r0--], a0
    maa y0, [r0++s], a0
    add p*, a0
    mov a0h, [r1++]
    mov a0l, [r1++]
.Lscale_end:
    rst 0x0800, mod0
    pop mod3
    pop cfgi
    pop r1
    pop r0
    pop y0
    ret always

// SATURATE(x, SIG_SAT) in place
.section .text.opus_k_sat
.global opus_k_sat
opus_k_sat:
    push r0
    mov a0l, r0
    sub 0x0001u8, a1
    mov 0xe000, a0h
    or 0x0001u8, a0
    mov a0, b1
    mov 0xffff, b0l
    set 0x1fff, b0h
    mov [r0++], a0h
    bkrep a1l, .Lsat_end
    or [r0++], a0
    cmp b0, a0
    brr .Lsat_hi, gt
    cmp b1, a0
    brr .Lsat_next, ge
    mov b1, a0
    brr .Lsat_write, always
.Lsat_hi:
    mov b0, a0
.Lsat_write:
    addv 0xfffe, r0
    mov a0h, [r0++]
    mov a0l, [r0++]
.Lsat_next:
    mov [r0++], a0h
.Lsat_end:
    pop r0
    ret always

// Sequential source, strided destination
.section .text.opus_k_ilv
.global opus_k_ilv
opus_k_ilv:
    push r0
    push r1
    push cfgi
    mov a0l, r0
    mov a1l, r1
    mov b1, a0
    shfi a0, a0, +1
    sub 0x0001u8, a0
    mov a0l, cfgi
    mov b0, a0
    sub 0x0001u8, a0
    bkrep a0l, .Lilv_end
    mov [r0++], a0
    mov a0l, [r1++]
    mov [r0++], a0
    mov a0l, [r1++s]
.Lilv_end:
    pop cfgi
    pop r1
    pop r0
    ret always

// Strided source, sequential destination
.section .text.opus_k_dilv
.global opus_k_dilv
opus_k_dilv:
    push r0
    push r1
    push cfgi
    mov a0l, r0
    mov a1l, r1
    mov b1, a0
    shfi a0, a0, +1
    sub 0x0001u8, a0
    mov a0l, cfgi
    mov b0, a0
    sub 0x0001u8, a0
    bkrep a0l, .Ldilv_end
    mov [r0++], a0
    mov a0l, [r1++]
    mov [r0++s], a0
    mov a0l, [r1++]
.Ldilv_end:
    pop cfgi
    pop r1
    pop r0
    ret always

// Bit by bit square root
.section .text.isqrt32
.global isqrt32
isqrt32:
    mov 0xffff, b0l
    set 0xffff, b0h
    and b0, a0, a0
    nop
    exp a0
    // Hardware: exp writes sv late.
    nop
    nop
    mov sv, a1
    neg a1, always
    add 0x001eu8, a1
    shfi a1, a1, -1
    mov a1l, sv
    clr b0, always
    mov 0x0001, b1l
    shfc b1, b1, always
    push r0
    mov a1l, r0
    bkrep r0, .Lisqrt_end
    mov b0, a1
    shfi a1, a1, +1
    add b1, a1
    shfc a1, a1, always
    cmp a1, a0
    brr .Lisqrt_skip, lt
    add b1, b0
    sub a1, a0
.Lisqrt_skip:
    addv 0xffff, sv
    shfi b1, b1, -1
.Lisqrt_end:
    pop r0
    mov b0, a0
    ret always

// bits2pulses search on a cache row
.section .text.opus_k_b2p
.global opus_k_b2p
opus_k_b2p:
    push r0
    push r1
    mov a0l, r0
    shfi a1, a1, +16
    shfi a1, a1, -16
    sub 0x0001u8, a1
    mov r0, b0
    mov [r0], a0
    add b0, a0
    mov a0, b1
    mov b0, a0
    mov 0x0005, r1
    bkrep r1, .Lb2p_end
    add b1, a0
    add 0x0001u8, a0
    shfi a0, a0, -1
    mov a0l, r1
    cmp [r1], a1
    brr .Lb2p_hi, le
    mov a0, b0
    brr .Lb2p_next, always
.Lb2p_hi:
    mov a0, b1
.Lb2p_next:
    mov b0, a0
.Lb2p_end:
    mov b0l, r1
    mov r0, a0
    cmp b0, a0
    brr .Lb2p_first, eq
    mov [r1], a0
    brr .Lb2p_low, always
.Lb2p_first:
    clr a0, always
    sub 0x0001u8, a0
.Lb2p_low:
    neg a0, always
    add a1, a0
    add a1, a0
    mov b1l, r1
    sub [r1], a0
    brr .Lb2p_lo, le
    mov b1, a0
    brr .Lb2p_out, always
.Lb2p_lo:
    mov b0, a0
.Lb2p_out:
    mov r0, b0
    sub b0, a0
    and 0x00ff, a0
    pop r1
    pop r0
    ret always

// Same arithmetic as mathops.c, 16-bit truncations included
.section .text.celt_rsqrt_norm
.global celt_rsqrt_norm
celt_rsqrt_norm:
    push y0
    chng 0x8000, a0l
    mov a0l, a1l
    mov 0x1a39, y0
    mpy y0, a1l, a0
    mov p*, a0
    shfi a0, a0, -15
    addv 0xcb4e, a0l
    mov a1l, y0
    mpy y0, a0l, a0
    mov p*, a0
    shfi a0, a0, -15
    mov a0l, y0
    addv 0x5c05, y0
    mpy y0, y0, a0
    mov p*, a0
    mov y0, b0l
    shfi a0, a0, -15
    mov a1l, y0
    mpy y0, a0l, a0
    mov p*, a1
    shfi a1, a1, -15
    add a0, a1
    mov a1l, a0l
    shfi a0, a0, +1
    chng 0x8000, a0l
    mov 0x3000, y0
    mpy y0, a0l, a0
    mov p*, a1
    shfi a1, a1, -15
    mov a1l, y0
    addv 0xc000, y0
    mpy y0, a0l, a0
    mov p*, a0
    shfi a0, a0, -15
    mov b0l, y0
    mpy y0, a0l, a0
    mov p*, a0
    shfi a0, a0, -15
    mov y0, a1l
    add a0, a1
    mov a1l, a0
    pop y0
    ret always

// Mirrors the approximate MULT32_32_Q31 terms
.section .text.celt_rsqrt_norm32
.global celt_rsqrt_norm32
celt_rsqrt_norm32:
    push y0
    push r0
    push r1
    push r2
    push r3
    mov a0l, r3
    mov a0h, r2
    shfi a0, a0, -15
    call celt_rsqrt_norm, always
    shfi a0, a0, +15
    mov a0l, r1
    mov a0h, r0
    mov r0, y0
    mpysu y0, r1, a0
    mov p*, a0
    shfi a0, a0, -15
    mpy y0, y0, a1
    add p*, a0
    mov a0, b1
    mov r2, y0
    mpysu y0, b1l, a0
    mov p*, a0
    shfi a0, a0, -15
    mov b1h, y0
    mpysu y0, r3, a1
    mov p*, a1
    shfi a1, a1, -15
    add a1, a0
    mpy y0, r2, a1
    add p*, a0
    add p*, a0
    mov 0x0c00, a1h
    sub a0, a1
    mov a1, b1
    mov r0, y0
    mpysu y0, b1l, a0
    mov p*, a0
    shfi a0, a0, -15
    mov b1h, y0
    mpysu y0, r1, a1
    mov p*, a1
    shfi a1, a1, -15
    add a1, a0
    mpy y0, r0, a1
    add p*, a0
    add p*, a0
    shfi a0, a0, +4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    ret always

// a0 wide, a1l signed word, low 32 bits valid
.section .text.__teak_mulword40
.global __teak_mulword40
__teak_mulword40:
    push y0
    mov a1l, y0
    mpysu y0, a0l, a1
    mov p*, a1
    mpy y0, a0h, a0
    mov p*, a0
    shfi a0, a0, +16
    add a1, a0
    pop y0
    ret always

// Low 32 bits of a0 times a1
.section .text.__teak_mul40
.global __teak_mul40
__teak_mul40:
    push y0
    mov a0l, y0
    mpysu y0, a1l, a1
    mov p*, b0
    mpy y0, a1h, a1
    mov p*, b1
    rnd a0, always
    mov a0h, y0
    mpy y0, a1l, a1
    mov p*, a0
    add b1, a0
    shfi a0, a0, +16
    add b0, a0
    pop y0
    ret always

// Sum of one bisection step in interp_bits2pulses
// args: bits1 lo, bits2 lo, thresh lo, cap, count, mid, floor
.section .text.opus_k_alloc_interp
.global opus_k_alloc_interp
opus_k_alloc_interp:
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    mov a0l, r4
    mov [r4++], r1
    mov [r4++], r0
    mov [r4++], r2
    mov [r4++], r3
    mov [r4++], r5
    mov [r4++], y0
    mov [r4], b1
    clr b0, always
.Linterp_low:
    mpysu y0, [r0--], a0
    mov p*, a0
    mpy y0, [r0--], a1
    mov p*, a1
    shfi a1, a1, +16
    add a1, a0
    shfi a0, a0, -6
    addl [r1--], a0
    addh [r1--], a0
    mov a0, a1
    subl [r2--], a1
    subh [r2--], a1
    brr .Linterp_hit, ge
    modr [r3--]
    cmp b1, a0
    brr .Linterp_next, lt
    add b1, b0
.Linterp_next:
    addv 0xffff, r5
    brr .Linterp_low, neq
    brr .Linterp_done, always
.Linterp_high:
    mpysu y0, [r0--], a0
    mov p*, a0
    mpy y0, [r0--], a1
    mov p*, a1
    shfi a1, a1, +16
    add a1, a0
    shfi a0, a0, -6
    addl [r1--], a0
    addh [r1--], a0
.Linterp_hit:
    mov [r3--], a1
    cmp a1, a0
    brr .Linterp_keep, le
    mov a1, a0
.Linterp_keep:
    add a0, b0
    addv 0xffff, r5
    brr .Linterp_high, neq
.Linterp_done:
    mov b0, a0
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    ret always

// Sum of one allocation vector trial
// args: trim lo, offsets, thresh lo, cap, count, floor, CN, vector
.section .text.opus_k_alloc_trial
.global opus_k_alloc_trial
opus_k_alloc_trial:
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    mov a0l, r4
    mov [r4++], r1
    mov [r4++], r5
    mov [r4++], r2
    mov [r4++], r3
    mov [r4++], sv
    mov [r4++], b1
    mov [r4++], r0
    mov [r4], r4
    clr b0, always
.Ltrial_low:
    mov [r0--], y0
    mpy y0, [r4--], a0
    mov p*, a0
    shfi a0, a0, -2
    cmp 0x0000u8, a0
    brr .Ltrial_flat, le
    addl [r1--], a0
    addh [r1--], a0
    clr a0, lt
    brr .Ltrial_add, always
.Ltrial_flat:
    modr [r1--]
    modr [r1--]
.Ltrial_add:
    mov [r5--], a1
    add a1, a0
    mov a0, a1
    subl [r2--], a1
    subh [r2--], a1
    brr .Ltrial_hit, ge
    modr [r3--]
    cmp b1, a0
    brr .Ltrial_next, lt
    add b1, b0
.Ltrial_next:
    addv 0xffff, sv
    brr .Ltrial_low, neq
    brr .Ltrial_done, always
.Ltrial_high:
    mov [r0--], y0
    mpy y0, [r4--], a0
    mov p*, a0
    shfi a0, a0, -2
    cmp 0x0000u8, a0
    brr .Ltrial_hflat, le
    addl [r1--], a0
    addh [r1--], a0
    clr a0, lt
    brr .Ltrial_hadd, always
.Ltrial_hflat:
    modr [r1--]
    modr [r1--]
.Ltrial_hadd:
    mov [r5--], a1
    add a1, a0
.Ltrial_hit:
    mov [r3--], a1
    cmp a1, a0
    brr .Ltrial_keep, le
    mov a1, a0
.Ltrial_keep:
    add a0, b0
    addv 0xffff, sv
    brr .Ltrial_high, neq
.Ltrial_done:
    mov b0, a0
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    ret always

// Band loop of denormalise_bands, normal gain range only
// block: eBands, means, logE, x, f, M, bands left
// stops early at a band needing the C path
.section .text.opus_k_denorm_bands
.global opus_k_denorm_bands
opus_k_denorm_bands:
    push mod0
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
    mov a0l, r5
    mov [r5++], r2
    mov [r5++], r3
    mov [r5++], r4
    mov [r5++], r0
    mov [r5++], r1
    mov [r5++], r6
.Ldb_band:
    cmpv 0x0000, [r5]
    br .Ldb_done, eq
    // lg, wraps at 32 bits
    mov [r3], a1
    shfi a1, a1, +16
    shfi a1, a1, +4
    mov [r4++], a0h
    or [r4++], a0
    add a1, a0
    // shift = 17 - (lg >> 24)
    mov a0h, a1
    shfi a1, a1, -8
    sub 0x0011u8, a1
    add 0x001fu8, a1
    cmp 0x0000u8, a1
    br .Ldb_done, le
    cmp 0x001fu8, a1
    br .Ldb_done, gt
    // sv = shift, b1 = round
    sub 0x001fu8, a1
    neg a1, always
    mov a1l, sv
    mov 0x0001, b0
    shfc b0, b1, always
    shfi b1, b1, -1
    add 0x0008u8, a1
    neg a1, always
    mov a1l, sv
    // bit 13 rounds, low 8 bits cannot matter
    shfi a0, a0, -8
    and 0xffff, a0
    add 0x0020u8, a0
    shfi a0, a0, -6
    shfi a0, a0, +4
    mov a0l, y0
    // gain, low word stays zero
    mov 0x27dc, a1
    mpy y0, a1l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    shfi a1, a1, -16
    add 0x39e3, a1
    mpy y0, a1l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    shfi a1, a1, -16
    add 0x5914, a1
    mpy y0, a1l, a1
    // Hardware: p0h after multiply is stale.
    mov p*, a1
    shfi a1, a1, -16
    add 0x3fff, a1
    mov a1l, a0l
    // samples in band - 1
    mov [r2++], a1
    mov [r2], b0
    sub a1, b0
    mov r6, y0
    mpy y0, b0l, a1
    mov p*, a1
    shfi a1, a1, -1
    sub 0x0001u8, a1
    mov a1l, r7
    mov a0l, y0
    modr [r3++]
    push mod3
    dint
    bkrep r7, .Ldb_end
    mov [r0++], a1h
    or [r0++], a1
    shfi a1, a1, +6
    clr a0, always
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    add b1, a0
    shfi a0, a0, +8
    shfc a0, a0, always
    mov a0h, [r1++]
    mov a0l, [r1++]
.Ldb_end:
    pop mod3
    addv 0xffff, [r5]
    br .Ldb_band, always
.Ldb_done:
    modr [r5--]
    mov r1, [r5--]
    mov r0, [r5]
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop mod0
    ret always

// CELT_PVQ_U: a0 n, a1 k, r1 row table; a0 result
.section .text.opus_k_unq_norm
.Lpvq_u:
    cmp a1, a0
    brr .Lu_order, lt
    mov a0, b0
    mov a1, a0
    mov b0, a1
.Lu_order:
    add r1, a0
    mov a0l, r3
    mov [r3], a0
    shfi a1, a1, +1
    add a1, a0
    mov a0l, r3
    mov [r3++], a0h
    or [r3], a0
    ret always

// decode_pulses plus normalise_residual
// block: iy, N, K, i hi, i lo, row, Ryy hi, Ryy lo,
// gain hi, gain lo, dec, iy, X, N, g hi, g lo, sv
.global opus_k_unq_norm
opus_k_unq_norm:
    push y0
    push r0
    push r1
    push r3
    push r4
    mov a0l, r4
    mov a0l, r0
    addv 0x0005, r0
    mov [r0], r1
    addv 0x0005, r0
    mov [r0], r0
    mov r4, r3
    modr [r3++]
    mov [r3++], a0
    mov [r3], a1
    call .Lpvq_u, always
    mov a0, b1
    mov r4, r3
    modr [r3++]
    mov [r3++], a0
    mov [r3], a1
    add 0x0001u8, a1
    call .Lpvq_u, always
    add b1, a0
    mov a0, a1
    mov r0, a0
    call ec_dec_uint, always
    mov r4, r0
    addv 0x0003, r0
    mov a0h, [r0++]
    mov a0l, [r0]
    mov r4, a0
    call opus_k_cwrsi, always
    mov r4, r0
    addv 0x0006, r0
    mov [r0++], a0h
    or [r0++], a0
    mov a0, b1
    call opus_k_clz32, always
    mov 0x001f, a1
    sub a0, a1
    shfi a1, a1, -1
    mov a1l, r1
    shfi a1, a1, +1
    neg a1, always
    add 0x001du8, a1
    push sv
    mov a1l, sv
    mov b1, a0
    shfc a0, a0, always
    pop sv
    call celt_rsqrt_norm32, always
    mov a0, a1
    push mod0
    push mod3
    dint
    rst 0x0c80, mod0
    set 0x0803, mod0
    clr a0, always
    mov [r0++], y0
    mpysu y0, a1l, a0
    maa y0, a1h, a0
    add p*, a0
    mov a1h, y0
    mpysu y0, [r0], a0
    maa y0, a1l, a0
    pop mod3
    pop mod0
    addv 0x0005, r0
    mov a0h, [r0++]
    mov a0l, [r0++]
    mov r1, a0
    neg a0, always
    add 0x0001u8, a0
    mov a0l, [r0]
    mov r4, a0
    add 0x000bu8, a0
    call opus_k_normres, always
    pop r4
    pop r3
    pop r1
    pop r0
    pop y0
    ret always

// No-split case of quant_partition plus alg_unquant
// a0l ctx, a1l X, b0l N, b1l b; stack B, LM, gain hi, lo
// -1 before any side effect when C must handle it
.section .text.opus_k_qp_leaf
.global opus_k_qp_leaf
opus_k_qp_leaf:
    push r7
    mov sp, r7
    push y0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    mov a0l, r4
    mov a1l, r5
    mov b0l, r6
    mov a0l, r0
    addv 0x0002, r0
    mov [r0], r0
    mov [r7+0x0004s7], a0
    add 0x0001u8, a0
    mov a0l, y0
    mov r0, r1
    addv 0x0003, r1
    mpy y0, [r1], a0
    mov p*, a0
    mov r4, r1
    addv 0x0003, r1
    add [r1], a0
    mov r0, r1
    addv 0x0019, r1
    mov [r1++], a1
    add a0, a1
    mov a1l, r2
    mov [r2], a1
    add [r1], a1
    mov a1l, r2
    // split test
    mov y0, a0
    cmp 0x0000u8, a0
    brr .Lqp_nosplit, eq
    mov r6, a0
    cmp 0x0002u8, a0
    brr .Lqp_nosplit, le
    mov [r2], a0
    add r2, a0
    mov a0l, r3
    mov [r3], a0
    add 0x000cu8, a0
    mov b1l, a1
    mov a1l, a1
    cmp a0, a1
    br .Lqp_fallback, gt
.Lqp_nosplit:
    mov r2, a0
    mov b1l, a1
    call opus_k_b2p, always
    mov a0l, a0
    cmp 0x0000u8, a0
    br .Lqp_fallback, le
    mov a0, b0
    add r2, a0
    mov a0l, r3
    mov [r3], a0
    add 0x0001u8, a0
    mov r4, r1
    addv 0x0008, r1
    mov [r1++], a1h
    or [r1], a1
    sub a0, a1
    br .Lqp_fallback, lt
    mov a1l, [r1]
    modr [r1--]
    mov a1h, [r1]
    call ogg_opus_poll, always
    // get_pulses
    mov b0, a0
    cmp 0x0008u8, a0
    brr .Lqp_k, lt
    mov a0, a1
    and 0x0007, a1
    add 0x0008u8, a1
    shfi a0, a0, -3
    sub 0x0001u8, a0
    mov a0l, sv
    shfc a1, a0, always
.Lqp_k:
    mov a0l, r3
    // block below iy, both on the stack
    mov sp, a1
    mov r6, a0
    add 0x0011u8, a0
    sub a0, a1
    mov a1l, sp
    nop
    mov sp, r1
    mov sp, a0
    add 0x0011u8, a0
    mov a0l, [r1++]
    mov r6, [r1++]
    mov r3, [r1++]
    modr [r1++]
    modr [r1++]
    mov r4, r0
    addv 0x0013, r0
    mov [r0], a1
    mov a1l, [r1++]
    modr [r1++]
    modr [r1++]
    mov [r7+0x0005s7], a1
    mov a1l, [r1++]
    mov [r7+0x0006s7], a1
    mov a1l, [r1++]
    mov r4, r0
    addv 0x0007, r0
    mov [r0], a1
    mov a1l, [r1++]
    mov a0l, [r1++]
    mov r5, [r1++]
    mov r6, [r1]
    mov sp, a0
    call opus_k_unq_norm, always
    // extract_collapse_mask, B is a power of two
    mov 0x0001, b0
    mov [r7+0x0003s7], a0
    cmp 0x0001u8, a0
    brr .Lqp_rot, le
    mov a0l, r1
    mov r6, a1
.Lqp_n0:
    shfi a1, a1, -1
    shfi a0, a0, -1
    cmp 0x0001u8, a0
    brr .Lqp_n0, gt
    sub 0x0001u8, a1
    mov a1l, r2
    mov sp, a0
    add 0x0011u8, a0
    mov a0l, r0
    clr b0, always
    mov 0x0001, b1
.Lqp_part:
    clr a0, always
    bkrep r2, .Lqp_or
    or [r0++], a0
    // Hardware: no flag or short body at block end.
    nop
.Lqp_or:
    cmp 0x0000u8, a0
    brr .Lqp_zero, eq
    add b1, b0
.Lqp_zero:
    shfi b1, b1, +1
    addv 0xffff, r1
    cmpv 0x0000, r1
    brr .Lqp_part, neq
.Lqp_rot:
    // rotation runs in C, K zero skips it
    clr a0, always
    mov r4, r0
    addv 0x0005, r0
    mov [r0], a1
    cmp 0x0000u8, a1
    brr .Lqp_rk, eq
    mov r3, a1
    shfi a1, a1, +1
    mov r6, a0
    cmp a0, a1
    brr .Lqp_kz, ge
    mov r3, a0
    brr .Lqp_rk, always
.Lqp_kz:
    clr a0, always
.Lqp_rk:
    addv 0x000d, r0
    mov a0l, [r0]
    mov sp, a0
    mov r6, a1
    add 0x0011u8, a1
    add a1, a0
    mov a0l, sp
    nop
    mov b0, a0
    brr .Lqp_out, always
.Lqp_fallback:
    mov 0xffff, a0
.Lqp_out:
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

// a0l dst, a1l words; zero fill
.section .text.opus_k_zero
.global opus_k_zero
opus_k_zero:
    push r0
    mov a0l, r0
    mov a1l, a1
    mov a1, b0
    clr a0, always
    shfi a1, a1, -1
    cmp 0x0000u8, a1
    brr .Lz_odd, eq
    sub 0x0001u8, a1
    bkrep a1l, .Lz_end
    mov a0l, [r0++]
    mov a0l, [r0++]
.Lz_end:
.Lz_odd:
    mov b0, a1
    and 0x0001, a1
    brr .Lz_done, eq
    mov a0l, [r0]
.Lz_done:
    pop r0
    ret always

// bitexact_cos: a0l x, y0 and a0 clobbered; a1 result
.section .text.opus_k_theta
.Lth_cos:
    mov a0l, y0
    mpy y0, a0l, a0
    mov p*, a0
    add 0x1000, a0
    shfi a0, a0, -13
    mov a0l, y0
    mov 0xfd8e, a1
    mpy y0, a1l, a1
    mov p*, a1
    add 0x4000, a1
    shfi a1, a1, -15
    add 0x2055, a1
    mpy y0, a1l, a1
    mov p*, a1
    add 0x4000, a1
    shfi a1, a1, -15
    add 0xe21d, a1
    mpy y0, a1l, a1
    mov p*, a1
    add 0x4000, a1
    shfi a1, a1, -15
    mov y0, a0
    sub a0, a1
    add 0x7fff, a1
    add 0x0001u8, a1
    ret always

// log2tan term: a0 normalised value; a1 result
.Lth_term:
    mov a0l, y0
    mov 0xf5db, a1
    mpy y0, a1l, a1
    mov p*, a1
    add 0x4000, a1
    shfi a1, a1, -15
    add 0x1efc, a1
    mpy y0, a1l, a1
    mov p*, a1
    add 0x4000, a1
    shfi a1, a1, -15
    ret always

// a0l sctx, a1l itheta, b0l N; stores imid, iside, delta
.global opus_k_theta
opus_k_theta:
    push y0
    push r0
    push r1
    push mod3
    dint
    mov a0l, r0
    mov b0l, r1
    modr [r0++]
    mov a1l, b1
    mov a1l, a0
    call .Lth_cos, always
    mov a1l, [r0++]
    // b0 keeps imid while iside runs
    mov a1, b0
    mov 0x4000, a0
    sub b1, a0
    call .Lth_cos, always
    mov a1l, [r0++]
    // a1 iside, b0 imid; ls-lc in b1
    mov a1, a0
    nop
    exp a0
    // Hardware: exp writes sv late.
    nop
    nop
    mov sv, b1
    mov sv, a1
    sub 0x0010u8, a1
    mov a1l, sv
    shfc a0, a0, always
    call .Lth_term, always
    exp b0
    // Hardware: exp writes sv late.
    nop
    nop
    mov b1, a0
    mov a1, b1
    mov sv, a1
    sub a0, a1
    shfi a1, a1, +11
    add a1, b1
    mov b0, a0
    mov sv, a1
    sub 0x0010u8, a1
    mov a1l, sv
    shfc a0, a0, always
    call .Lth_term, always
    sub a1, b1
    // delta
    mov r1, a0
    sub 0x0001u8, a0
    shfi a0, a0, +7
    mov a0l, y0
    mpy y0, b1l, a0
    mov p*, a0
    add 0x4000, a0
    shfi a0, a0, -15
    mov a0l, [r0]
    pop mod3
    pop r1
    pop r0
    pop y0
    ret always
