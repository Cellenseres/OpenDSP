// SPDX-License-Identifier: BSD-2-Clause
// Bounded lattice TNS: job = samples, count (<=8), state, coefficients, order, step.
// Descending recurrence, symmetric saturation after each operation, as in C.
.text
.global aac_tns_span
aac_tns_span:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    mov a0l, r6
    addv 0xfffe, sp
    mov sp, r7
    nop
    mov [r6++], r0
    mov [r6++], r1
    mov [r6++], a0
    mov a0l, [r7+0x0000s7]
    mov [r6++], a0
    mov a0l, [r7+0x0001s7]
    mov [r6++], r4
    mov [r6], r5
    mov r1, a0
    cmpv 0x0, a0l
    br .Ltn_done, eq
    mov r4, a0
    cmpv 0x0, a0l
    br .Ltn_done, eq
.Ltn_sample:
    mov [arrn0+ars0], b1
    mov r4, a0
    sub 0x1, a0
    mov a0, a1
    shfi a0, a0, +1
    add [r7+0x0000s7], a0
    mov a0l, r2
    add [r7+0x0001s7], a1
    mov a1l, r3
    mov r4, r6
.Ltn_order:
    mov [arrn2+ars0], a0
    mov [r3], a1
    call aac_mul_q15, always
    mov a0, a1
    mov b1, a0
    call aac_diff_sat, always
    mov a0, b1
    mov r6, a0
    mov r4, a1
    cmp a1, a0
    br .Ltn_skip_state, eq
    mov b1, a0
    mov [r3], a1
    call aac_mul_q15, always
    mov a0, a1
    mov [arrn2+ars0], a0
    call aac_sum_sat, always
    modr [r2++]
    modr [r2++]
    mov a0, [arrn2+ars0]
    modr [r2--]
    modr [r2--]
.Ltn_skip_state:
    modr [r2--]
    modr [r2--]
    modr [r3--]
    modr [r6--]
    mov r6, a0
    cmpv 0x0, a0l
    br .Ltn_order, neq
    mov [r7+0x0000s7], a0
    mov a0l, r2
    mov b1, [arrn2+ars0]
    mov b1, [arrn0+ars0]
    mov r0, a0
    mov r5, a1
    add a1, a0
    mov a0l, r0
    modr [r1--]
    mov r1, a0
    cmpv 0x0, a0l
    br .Ltn_sample, neq
.Ltn_done:
    addv 0x2, sp
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always
