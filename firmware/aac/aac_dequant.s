// SPDX-License-Identifier: BSD-2-Clause
// Bit-exact spectral dequantization over a caller-bounded span (<=16 values).
// job: spectrum word address, count, Q14 gain, signed shift, pow43 word address.
// Retains the scalar helpers' interpolation, truncation and symmetric saturation.
// Interrupts stay enabled except inside the existing multiply helper.
.text
.global aac_dequant_span
aac_dequant_span:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    mov a0l, r6
    mov [r6++], r0
    mov [r6++], r1
    mov [r6++], r3
    mov [r6++], r4
    mov [r6], r2
    mov r1, a0
    cmpv 0x0, a0l
    br .Ldq_done, eq
.Ldq_next:
    mov [arrn0+ars0], a0
    add 0x0, a0
    br .Ldq_advance, eq
    mov 0x0, r5
    br .Ldq_magnitude, ge
    mov 0x1, r5
    neg a0, always
.Ldq_magnitude:
    and 0xffff, a0                // scalar code first converts magnitude to u16
    cmp 0x1fff, a0
    br .Ldq_clamped, le
    mov 0x1fff, a0
.Ldq_clamped:
    cmp 0x400, a0
    br .Ldq_interpolate, gt
    shfi a0, a0, +1
    mov r2, a1
    add a1, a0
    push r0
    mov a0l, r0
    mov [arrn0+ars0], a0
    pop r0
    mov r4, r6
    br .Ldq_scale, always
.Ldq_interpolate:
    mov a0l, r6                  // retain magnitude for fractional index
    shfi a0, a0, -3
    shfi a0, a0, +1
    mov r2, a1
    add a1, a0
    push r0
    mov a0l, r0
    mov [arrn0+ars0], a0          // lower table value
    pusha a0
    modr [r0++]
    modr [r0++]
    mov [arrn0+ars0], a0          // upper table value
    popa a1
    pusha a1
    sub a1, a0
    mov r6, a1
    and 0x7, a1
    mov 0x3, b0
    call aac_mul_shift, always
    popa a1
    add a1, a0
    pop r0
    mov r4, a1
    add 0x4, a1
    mov a1l, r6
.Ldq_scale:
    mov r3, a1
    call aac_mul_q16, always
    mov r6, a1
    call aac_shift_sat, always
    mov r5, a1
    cmpv 0x0, a1l
    br .Ldq_store, eq
    neg a0, always
.Ldq_store:
    mov a0, [arrn0+ars0]
.Ldq_advance:
    modr [r0++]
    modr [r0++]
    modr [r1--]
    mov r1, a0
    cmpv 0x0, a0l
    br .Ldq_next, neq
.Ldq_done:
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always
