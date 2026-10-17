// SPDX-License-Identifier: BSD-2-Clause
// Scalar spectral helpers. Multiplier lifetimes are interrupt-protected.
.text
.global aac_lcg
aac_lcg:
    mov a0, b1
    shfi a0, a0, +20
    shfi b1, a1, +19
    add a1, a0
    shfi b1, a1, +16
    add a1, a0
    shfi b1, a1, +14
    add a1, a0
    shfi b1, a1, +13
    add a1, a0
    shfi b1, a1, +10
    add a1, a0
    shfi b1, a1, +9
    add a1, a0
    shfi b1, a1, +3
    add a1, a0
    shfi b1, a1, +2
    add a1, a0
    add b1, a0
    mov 0x3c6e, a1
    shfi a1, a1, +16
    or 0xf35f, a1
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    ret always

.global aac_load32
aac_load32:
    push r0
    mov a0l, r0
    mov [arrn0+ars0], a0
    pop r0
    ret always

.global aac_store32
aac_store32:
    push r0
    mov a0l, r0
    mov a1, [arrn0+ars0]
    pop r0
    ret always

// s32 aac_mul_shift(s32 a, s16 b, s16 shift): floor(a*b/2^shift).
.global aac_mul_shift
aac_mul_shift:
    push y0
    push sv
    push mod0
    mov a1l, y0
    mov a0, a1
    shfi b0, b0, +24
    shfi b0, b0, -24
    rst 0x0c00, mod0
    dint
    clr a0, always
    mpysu y0, a1l, a0
    mac y0, a1h, a0
    mov b0, a1
    neg a1, always
    mov a1l, sv
    shfc a0, a0, always
    mov a0, b1
    mov b0, a0
    neg a0, always
    add 0x10, a0
    mov a0l, sv
    clr a1, always
    add p*, a1
    shfc a1, a1, always
    mov b1, a0
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    eint
    pop mod0
    pop sv
    pop y0
    ret always

.global aac_sum_sat
aac_sum_sat:
    add a1, a0
    br .Lar_limit, always
.global aac_diff_sat
aac_diff_sat:
    sub a1, a0
.Lar_limit:
    lim a0, a0
    neg a0, always
    lim a0, a0
    neg a0, always
    ret always

.global aac_shift_sat
aac_shift_sat:
    push sv
    shfi a1, a1, +24
    shfi a1, a1, -24
    add 0x0, a1
    br .Lar_right, lt
    lim a0, a0
    neg a0, always
    lim a0, a0
    neg a0, always
    cmpv 0x1f, a1l
    br .Lar_large, gt
.Lar_left_loop:
    add 0x0, a1
    br .Lar_shift_done, eq
    shfi a0, a0, +1
    lim a0, a0
    neg a0, always
    lim a0, a0
    neg a0, always
    sub 0x1, a1
    br .Lar_left_loop, always
.Lar_large:
    add 0x0, a0
    br .Lar_shift_done, eq
    mov 0x7fff, a1
    shfi a1, a1, +16
    or 0xffff, a1
    add 0x0, a0
    br .Lar_large_positive, gt
    neg a1, always
.Lar_large_positive:
    mov a1, a0
    br .Lar_shift_done, always
.Lar_right:
    cmpv 0xffe1, a1l
    br .Lar_zero, lt
    neg a1, always
    sub 0x1, a1
    mov a1l, sv
    mov 0x1, b0
    shfc b0, b0, always
    add b0, a0
    add 0x1, a1
    neg a1, always
    mov a1l, sv
    shfc a0, a0, always
    br .Lar_shift_done, always
.Lar_zero:
    clr a0, always
.Lar_shift_done:
    pop sv
    ret always
