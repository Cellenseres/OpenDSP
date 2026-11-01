// SPDX-License-Identifier: BSD-2-Clause
.macro high_product left_low, left_high, right_low, right_high
    mov \left_low, y0
    macuu y0, \right_low, a0
    clr a0, always
    add p*, a0
    shfi a0, a0, -16
    mov \left_high, y0
    mpysu y0, \right_low, a0
    mov \right_high, y0
    macsu y0, \left_low, a0
    add p*, a0
    mov a0, b1
    mpy y0, \left_high, a1
    clr a1, always
    add p*, a1
    shfi b1, a0, -16
    add a1, a0
.endm

.macro cross_product name, first_operation, second_operation
.section .text.\name
.global \name
.type \name,@function
\name:
    push mod3
    dint
    push mod0
    push y0
    push sv
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r3
    mov b0l, r4
    mov b0h, r5
    mov b1l, r6
    mov b1h, sv
    high_product r0, r1, r4, r5
    mov a0, b0
    high_product r2, r3, r6, sv
    \first_operation a0, b0
    shfi b0, a0, +1
    mov sp, r7
    addv 0x0e, r7
    mov [r7], r7
    mov a0h, [r7++]
    mov a0l, [r7]
    high_product r2, r3, r4, r5
    mov a0, b0
    high_product r0, r1, r6, sv
    \second_operation a0, b0
    shfi b0, a0, +1
    mov sp, r7
    addv 0x0f, r7
    mov [r7], r7
    mov a0h, [r7++]
    mov a0l, [r7]
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop sv
    pop y0
    pop mod0
    pop mod3
    ret always
.size \name, .-\name
.endm

cross_product tremor_cross_product, add, sub
cross_product tremor_cross_product_negative, sub, add
