// SPDX-License-Identifier: BSD-2-Clause
// Range decoder, bit-exact with libopus entdec.c.
// ec_ctx words: 0 buf, 3 storage, 5 end_offs, 6 window,
// 8 nend, 9 nbits, b offs, c rng, e val, 10 ext, 12 rem, 13 error
.section .text.opus_k_clz32
.global opus_k_clz32
opus_k_clz32:
    shfi a0, a0, +8
    br .Lclz_top, lt
    br .Lclz_zero, eq
    push sv
    exp a0
    // Hardware: exp writes sv late.
    nop
    nop
    mov sv, a0
    pop sv
    add 0x0009u8, a0
    ret always
.Lclz_top:
    clr a0, always
    ret always
.Lclz_zero:
    mov 0x0020, a0
    ret always

.section .text.ec_tell_frac
.global ec_tell_frac
ec_tell_frac:
    push r7
    mov a0l, r7
    clr a0, always
    mov [r7+0x0009s7], a1
    shfi a1, a1, +3
    or [r7+0x000cs7], a0
    shfi a0, a0, +16
    or [r7+0x000ds7], a0
    nop
    exp a0
    // Hardware: exp writes sv late.
    nop
    nop
    shfc a0, a0, always
    shfi a0, a0, -15
    and 0x7fff, a0
    cmp 0x3504, a0
    br .Ltf_hi, gt
    cmp 0x1837, a0
    br .Ltf_2, gt
    cmp 0x0b95, a0
    br .Ltf_out, le
    sub 0x0001u8, a1
    br .Ltf_out, always
.Ltf_2:
    cmp 0x25ff, a0
    br .Ltf_3, gt
    sub 0x0002u8, a1
    br .Ltf_out, always
.Ltf_3:
    sub 0x0003u8, a1
    br .Ltf_out, always
.Ltf_hi:
    cmp 0x5745, a0
    br .Ltf_6, gt
    cmp 0x4567, a0
    br .Ltf_5, gt
    sub 0x0004u8, a1
    br .Ltf_out, always
.Ltf_5:
    sub 0x0005u8, a1
    br .Ltf_out, always
.Ltf_6:
    cmp 0x6ac1, a0
    br .Ltf_7, gt
    sub 0x0006u8, a1
    br .Ltf_out, always
.Ltf_7:
    sub 0x0007u8, a1
.Ltf_out:
    mov sv, a0
    shfi a0, a0, +3
    add a1, a0
    sub 0x00f8u8, a0
    pop r7
    ret always

// a0 / a1 unsigned, keeps r0, uses r1
.section .text.ec_decode
.global ec_decode
.global ec_decode_bin
.Lec_div:
    cmp a1, a0
    br .Ldiv_zero, c
    mov a0, b0
    exp a1
    // Hardware: exp writes sv late.
    nop
    nop
    mov sv, r1
    exp a0
    // Hardware: exp writes sv late.
    nop
    nop
    mov r1, a0
    sub sv, a0
    mov a0l, r1
    add 0x0001u8, a0
    mov a0l, sv
    shfc a1, a1, always
    sub 0x0001u8, a1
    mov a1, a0
    mov b0, a1
    mov a0, b0
    mov a1, a0
    // remainder above, quotient bits below
    bkrep r1, .Ldiv_end
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, gt
.Ldiv_end:
    mov 0x0001, a1
    shfc a1, a1, always
    sub 0x0001u8, a1
    and a1, a0, a0
    ret always
.Ldiv_zero:
    clr a0, always
    ret always

ec_decode_bin:
    push r7
    push r0
    push r1
    mov a0l, r7
    mov a1l, sv
    mov 0x0001, a1
    shfc a1, a1, always
    mov a1l, r0
    mov sv, a1
    neg a1, always
    mov a1l, sv
    clr a0, always
    or [r7+0x000cs7], a0
    shfi a0, a0, +16
    or [r7+0x000ds7], a0
    shfc a0, a0, always
    br .Lec_scale, always

ec_decode:
    push r7
    push r0
    push r1
    mov a0l, r7
    mov a1l, r0
    mov r0, a0
    cmp 0x0101, a0
    brr .Lec_slow, gt
    cmp 0x0003u8, a0
    brr .Lec_slow, lt
    mov [r7+0x000cs7], a1
    cmp 0x0000u8, a1
    brr .Lec_slow, lt
    // rng/ft: estimate from floor(2^32/ft), low by at most 2
    push y0
    shfi a0, a0, +1
    mov 0x45fb, r1
    add [r1], a0
    mov a0l, r1
    mov a1l, y0
    mpysu y0, [r1++], a0
    mov p*, a0
    mov [r1], y0
    mov [r7+0x000ds7], a1
    mpysu y0, a1l, a1
    add p*, a0
    shfi a0, a0, -16
    mov [r7+0x000cs7], a1
    mpy y0, a1l, a1
    add p*, a0
    // remainder fits 16 bits
    mov r0, y0
    mpy y0, a0l, a1
    mov p*, b0
    mov [r7+0x000ds7], a1
    sub b0, a1
    mov a1l, a1
    mov r0, b0
.Lec_fix:
    cmp b0, a1
    brr .Lec_fixed, lt
    sub b0, a1
    add 0x0001u8, a0
    brr .Lec_fix, always
.Lec_fixed:
    pop y0
    br .Lec_scale, always
.Lec_slow:
    mov r0, a1l
    clr a0, always
    or [r7+0x000cs7], a0
    shfi a0, a0, +16
    or [r7+0x000ds7], a0
    call .Lec_div, always
.Lec_scale:
    mov a0l, [r7+0x0011s7]
    mov a0, a1
    shfi a0, a0, -16
    mov a0l, [r7+0x0010s7]
    mov [r7+0x000es7], a0
    shfi a0, a0, +16
    or [r7+0x000fs7], a0
    call .Lec_div, always
    add 0x0001u8, a0
    mov r0, a1l
    sub a0, a1
    br .Lec_in, ge
    clr a1, always
.Lec_in:
    mov a1, a0
    pop r1
    pop r0
    pop r7
    ret always

// b0 rng, a1 val, r7 ctx, sv result
.section .text.ec_dec_update
.global ec_dec_update
.global ec_dec_bit_logp
.global ec_dec_icdf
.Lec_norm:
    mov 0x0080, b1h
    cmp b1, b0
    br .Lnorm_loop, le
    mov b0, a0
    mov a0l, [r7+0x000ds7]
    shfi a0, a0, -16
    mov a0l, [r7+0x000cs7]
    mov a1l, [r7+0x000fs7]
    shfi a1, a1, -16
    mov a1l, [r7+0x000es7]
    mov sv, a0
    pop r7
    ret always
.Lnorm_loop:
    push r0
.Lnorm_next:
    mov [r7+0x0009s7], a0
    add 0x0008u8, a0
    mov a0l, [r7+0x0009s7]
    shfi b0, b0, +8
    mov [r7+0x0012s7], a0
    mov a0, b1
    mov [r7+0x000bs7], a0
    cmp [r7+0x0003s7], a0
    br .Lnorm_none, ge
    add 0x0001u8, a0
    mov a0l, [r7+0x000bs7]
    add [r7+0x0000s7], a0
    sub 0x0001u8, a0
    mov a0l, r0
    mov [r0], a0l
    and 0x00ff, a0
    br .Lnorm_have, always
.Lnorm_none:
    clr a0, always
.Lnorm_have:
    mov a0l, [r7+0x0012s7]
    shfi b1, b1, +8
    or b1, a0, a0
    shfi a0, a0, -1
    and 0x00ff, a0
    xor 0x00ffu8, a0
    shfi a1, a1, +8
    or a0, a1, a1
    mov 0x0080, b1h
    cmp b1, b0
    br .Lnorm_next, le
    pop r0
    mov a1l, [r7+0x000fs7]
    shfi a1, a1, -16
    and 0x7fff, a1
    mov a1l, [r7+0x000es7]
    mov b0, a0
    mov a0l, [r7+0x000ds7]
    shfi a0, a0, -16
    mov a0l, [r7+0x000cs7]
    mov sv, a0
    pop r7
    ret always

ec_dec_update:
    push r7
    push y0
    push r0
    mov a0l, r7
    mov a0l, r0
    addv 0x0010, r0
    sub b0, b1
    mov b1l, y0
    push mod3
    dint
    mpysu y0, [r0++], a0
    clr a0, always
    add p*, a0
    shfi a0, a0, +16
    mpysu y0, [r0--], a0
    add p*, a0
    cmpv 0x0000, a1l
    br .Lup_rest, eq
    sub a1, b0
    mov b0l, y0
    mpysu y0, [r0++], a1
    clr a1, always
    add p*, a1
    shfi a1, a1, +16
    mpysu y0, [r0], a1
    add p*, a1
    pop mod3
    mov a1, b0
    br .Lup_val, always
.Lup_rest:
    pop mod3
    clr a1, always
    or [r7+0x000cs7], a1
    shfi a1, a1, +16
    or [r7+0x000ds7], a1
    sub a0, a1
    mov a1, b0
.Lup_val:
    mov [r7+0x000es7], a1
    shfi a1, a1, +16
    or [r7+0x000fs7], a1
    sub a0, a1
    pop r0
    pop y0
    br .Lec_norm, always

ec_dec_bit_logp:
    push r7
    mov a0l, r7
    clr a0, always
    or [r7+0x000cs7], a0
    shfi a0, a0, +16
    or [r7+0x000ds7], a0
    mov a0, b0
    neg a1, always
    mov a1l, sv
    shfc a0, a0, always
    mov [r7+0x000es7], a1
    shfi a1, a1, +16
    or [r7+0x000fs7], a1
    cmp a0, a1
    br .Lbit_one, lt
    sub a0, a1
    sub a0, b0
    mov 0x0000, sv
    br .Lec_norm, always
.Lbit_one:
    mov a0, b0
    mov 0x0001, sv
    br .Lec_norm, always

ec_dec_icdf:
    push r7
    push y0
    push r0
    mov a0l, r7
    mov a1l, r0
    mov [r7+0x000es7], a0
    shfi a0, a0, +16
    or [r7+0x000fs7], a0
    mov a0, b1
    clr a0, always
    or [r7+0x000cs7], a0
    shfi a0, a0, +16
    or [r7+0x000ds7], a0
    mov b0, a1
    neg a1, always
    mov a1l, sv
    mov a0, a1
    shfc a1, a1, always
    mov 0xffff, sv
    push mod3
    dint
.Licdf_next:
    mov a0, b0
    addv 0x0001, sv
    mov [r0++], a0l
    and 0x00ff, a0
    mov a0l, y0
    mpysu y0, a1h, a0
    clr a0, always
    add p*, a0
    shfi a0, a0, +16
    mpysu y0, a1l, a0
    add p*, a0
    cmp a0, b1
    br .Licdf_next, lt
    pop mod3
    mov b1, a1
    sub a0, a1
    sub a0, b0
    pop r0
    pop y0
    br .Lec_norm, always

.section .text.ec_dec_bits
.global ec_dec_bits
ec_dec_bits:
    push r7
    push r0
    mov a0l, r7
    mov a1l, r0
    mov r0, a1l
    mov [r7+0x0008s7], a0
    mov a0, b0
    clr a0, always
    or [r7+0x0006s7], a0
    shfi a0, a0, +16
    or [r7+0x0007s7], a0
    cmp a1, b0
    br .Lbits_go, ge
    push r1
    mov a0, b1
.Lbits_fill:
    mov [r7+0x0005s7], a0
    cmp [r7+0x0003s7], a0
    br .Lbits_none, ge
    add 0x0001u8, a0
    mov a0l, [r7+0x0005s7]
    neg a0, always
    add [r7+0x0003s7], a0
    add [r7+0x0000s7], a0
    mov a0l, r1
    mov [r1], a0l
    and 0x00ff, a0
    br .Lbits_or, always
.Lbits_none:
    clr a0, always
.Lbits_or:
    mov b0l, sv
    shfc a0, a0, always
    or b1, a0, a0
    mov a0, b1
    mov b0, a0
    add 0x0008u8, a0
    mov a0, b0
    cmp 0x0018u8, a0
    br .Lbits_fill, le
    mov b1, a0
    pop r1
.Lbits_go:
    mov r0, sv
    mov 0x0001, a1
    shfc a1, a1, always
    sub 0x0001u8, a1
    and a0, a1, a1
    mov a1, b1
    mov r0, a1
    neg a1, always
    mov a1l, sv
    shfc a0, a0, always
    mov a0l, [r7+0x0007s7]
    shfi a0, a0, -16
    mov a0l, [r7+0x0006s7]
    mov b0, a0
    add a1, a0
    mov a0l, [r7+0x0008s7]
    mov [r7+0x0009s7], a0
    sub a1, a0
    mov a0l, [r7+0x0009s7]
    mov b1, a0
    pop r0
    pop r7
    ret always

.section .text.ec_dec_uint
.global ec_dec_uint
ec_dec_uint:
    push r4
    push r5
    push r6
    mov a0l, r4
    sub 0x0001u8, a1
    shfi a1, a1, +8
    set 0x0080, mod0
    shfi a1, a1, -8
    rst 0x0080, mod0
    exp a1
    // Hardware: exp writes sv late.
    nop
    nop
    mov sv, a0
    cmp 0x0016u8, a0
    br .Luint_small, gt
    sub 0x0017u8, a0
    mov a0l, sv
    neg a0, always
    mov a0l, r6
    pusha a1
    mov a1, a0
    shfc a0, a0, always
    add 0x0001u8, a0
    mov a0l, r5
    mov r5, a1
    mov r4, a0
    call ec_decode, always
    mov a0, a1
    add 0x0001u8, a0
    mov a0, b0
    mov r5, b1
    mov a1l, r5
    mov r4, a0
    call ec_dec_update, always
    mov r4, a0
    mov r6, a1
    call ec_dec_bits, always
    mov r5, a1l
    mov r6, sv
    shfc a1, a1, always
    or a0, a1, a0
    popa a1
    shfi a1, a1, +8
    set 0x0080, mod0
    shfi a1, a1, -8
    rst 0x0080, mod0
    cmp a1, a0
    br .Luint_out, le
    mov r4, a0
    add 0x0013u8, a0
    mov a0l, r6
    mov 0x0001, a0
    mov a0l, [r6]
    mov a1, a0
.Luint_out:
    pop r6
    pop r5
    pop r4
    ret always
.Luint_small:
    add 0x0001u8, a1
    mov a1l, r5
    mov r4, a0
    call ec_decode, always
    mov a0, a1
    mov a1l, r6
    add 0x0001u8, a0
    mov a0, b0
    mov r5, b1
    mov r4, a0
    call ec_dec_update, always
    mov r6, a0
    pop r6
    pop r5
    pop r4
    ret always
