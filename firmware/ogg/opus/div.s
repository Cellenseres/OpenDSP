// SPDX-License-Identifier: BSD-2-Clause
// Unsigned 40-bit division, steps only over quotient bits.
.section .text.__teak_divmod40
.global __teak_divmod40
.type __teak_divmod40,@function
__teak_divmod40:
    push r0
    push r1
    push mod0
    set 0x0003, mod0
    mov b0l, r0
    cmp a1, a0
    br .Ldiv_small, c
    cmp 0x0000u8, a1
    br .Ldiv_wide, le
    cmp 0x0000u8, a0
    br .Ldiv_wide, lt
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
    mov sv, a0
    cmp 0xfffa, a0
    br .Ldiv_wide0, lt
    neg a0, always
    add r1, a0
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
    cmpv 0x0, r0
    brr .Ldiv_rem, neq
    mov 0x0001, a1
    shfc a1, a1, always
    sub 0x0001u8, a1
    and a1, a0, a0
    br .Ldiv_return, always
.Ldiv_rem:
    mov sv, a1
    neg a1, always
    mov a1l, sv
    shfc a0, a0, always
    br .Ldiv_return, always
// numerator too wide for the combined register
.Ldiv_wide0:
    mov b0, a0
// values with bit 39 set or zero divisor
.Ldiv_wide:
    clr b0, always
    mov 0x28, r1
.Ldiv_bit:
    shfi a0, a0, +1
    // ROL b0, always
    .short 0x6f50
    br .Ldiv_subtract, c
    cmp a1, b0
    br .Ldiv_next, c
.Ldiv_subtract:
    sub a1, b0
    or 0x0001u8, a0
.Ldiv_next:
    addv 0xffff, r1
    br .Ldiv_bit, neq
    cmpv 0x0, r0
    br .Ldiv_return, eq
    mov b0, a0
    br .Ldiv_return, always
.Ldiv_small:
    cmpv 0x0, r0
    br .Ldiv_return, neq
    clr a0, always
.Ldiv_return:
    pop mod0
    pop r1
    pop r0
    ret always
.size __teak_divmod40, .-__teak_divmod40

// Signed quotient through the unsigned divider
.section .text.__teak_sdiv40
.global __teak_sdiv40
.type __teak_sdiv40,@function
__teak_sdiv40:
    push r0
    mov 0x0000, r0
    cmp 0x0000u8, a0
    brr .Lsdiv_num, ge
    neg a0, always
    modr [r0++]
.Lsdiv_num:
    cmp 0x0000u8, a1
    brr .Lsdiv_den, ge
    neg a1, always
    modr [r0++]
.Lsdiv_den:
    clr b0, always
    call __teak_divmod40, always
    cmpv 0x0001, r0
    neg a0, eq
    pop r0
    ret always
.size __teak_sdiv40, .-__teak_sdiv40

