// SPDX-License-Identifier: BSD-2-Clause
// LUT requantization stops before magnitudes >=129.
// Job: spectrum, count <=64, shift, quarter, pow43, quarter table.
.text
.global mp3_requant_small
mp3_requant_small:
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
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r0
    mov [r0++], a0
    mov a0l, r1
    mov [r0++], a0
    mov a0l, r2
    mov [r0++], a0
    mov a0l, sv
    mov [r0++], a0
    mov a0l, r6
    mov [r0++], a0
    mov a0l, r5
    mov [r0], a0
    mov a0l, r7
    mov 0x0, r3
.Lrequant_loop:
    cmpv 0x0, r2
    br .Lrequant_done, eq
    mov r1, r0
    mov [arrn0+ars0], a0
    cmp 0x0u8, a0
    br .Lrequant_next, eq
    mov 0x0, r4
    br .Lrequant_positive, gt
    mov 0x1, r4
    neg a0, always
.Lrequant_positive:
    cmp 0x81u8, a0
    br .Lrequant_done, ge
    shfi a0, a0, +1
    mov r5, a1
    add a1, a0
    mov a0l, r0
    mov [arrn0+ars0], a0
    cmpv 0x0, r6
    br .Lrequant_quarter, neq
    add 0x1, a0
    shfi a0, a0, -1
    br .Lrequant_shift, always
.Lrequant_quarter:
    // Prepare only for nonzero values with a fractional gain. Avoid setup
    // on silent spans and preserve callee-owned Y registers around this use.
    // c = X0*65536 + signed(X1), exactly as in the tested coefficient adapter.
    push y0
    push y1
    mov r7, r0
    mov [arrn0+ars0], a1
    mov a1l, x1
    mov 0x1, b0
    shfi b0, b0, +15
    add b0, a1
    shfi a1, a1, -16
    mov a1l, x0
    // R registers and SV survive; only the bounded helper masks IRQs.
    call mp3_coeff30_native, always
    pop y1
    pop y0
.Lrequant_shift:
    mov sv, a1
    cmp 0x1e, a1
    br .Lrequant_zero, gt
    cmp 0xffe1, a1
    br .Lrequant_zero, lt
    shfc a0, a0, always
    shfi a0, a0, +8
    shfi a0, a0, -8
    br .Lrequant_sign, always
.Lrequant_zero:
    clr a0, always
.Lrequant_sign:
    cmpv 0x0, r4
    br .Lrequant_store, eq
    neg a0, always
.Lrequant_store:
    mov r1, r0
    mov a0, [arrn0+ars0]
.Lrequant_next:
    addv 0x2, r1
    modr [r3++]
    modr [r2--]
    br .Lrequant_loop, always
.Lrequant_done:
    mov r3, a0
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
