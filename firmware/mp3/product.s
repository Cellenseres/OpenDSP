// SPDX-License-Identifier: BSD-2-Clause
// round(a*b / 2^shift); ties toward +infinity.
// Shift 16..31; caller guarantees an s32 result.
// IRQs stay masked while products are live.
.text
.global mp3_product
mp3_product:
    push y0
    push sv
    push mod0
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    set 0x0003, mod0
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r3
    mov b0l, r4
    rst 0x0c00, mod0
    dint
    mov r0, y0
    macuu y0, r2, a0
    clr a0, always
    add p*, a0
    mov a0l, r5
    shfi a0, a0, -16
    mov r1, y0
    mpysu y0, r2, a0
    mov r3, y0
    macsu y0, r0, a0
    add p*, a0
    mov r4, a1
    sub 0x10, a1
    br .Lmp3_round16, eq
    sub 0x1, a1
    mov a1l, sv
    mov 0x1, a1
    shfc a1, a1, always
    add a1, a0
    br .Lmp3_high, always
.Lmp3_round16:
    mov r5, a1
    shfi a1, a1, +24
    shfi a1, a1, -24
    add 0x0, a1
    br .Lmp3_high, ge
    add 0x1, a0
.Lmp3_high:
    mov a0, b1
    mov r1, y0
    mpy y0, r3, a1
    clr a1, always
    add p*, a1
    mov r4, b0
    mov 0x20, a0
    sub b0, a0
    mov a0l, sv
    shfc a1, a1, always
    sub 0x10, a0
    mov a0l, sv
    shfc b1, a0, always
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    eint
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop mod0
    pop sv
    pop y0
    ret always

// Bounded word copy used by short-window history; no live product or IRQ mask.
.global mp3_copy_words
mp3_copy_words:
    push r0
    push r1
    push r3
    mov a0l, r0
    mov a1l, r1
    mov b0l, r3
    cmpv 0x0, r3
    br .Lmp3_copy_done, eq
    modr [r3--]
    bkrep r3, .Lmp3_copy_end
    mov [r0++], a0
    mov a0l, [r1++]
.Lmp3_copy_end:
.Lmp3_copy_done:
    pop r3
    pop r1
    pop r0
    ret always

// Fixed-shift private ABI; r0/r1/r2/r4/y0 and all accumulators are scratch.
// Entry requires product shift 0, arithmetic shifts, non-saturating moves.

.global mp3_mul27_native
mp3_mul27_native:
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r4
    dint
    mov r0, y0
    macuu y0, r2, a0
    clr a0, always
    add p*, a0
    shfi a0, a0, -16
    mov r1, y0
    mpysu y0, r2, a0
    mov r4, y0
    macsu y0, r0, a0
    add p*, a0
    add 0x0400, a0
    mov a0, b0
    mov r1, y0
    mpy y0, r4, a1
    clr a1, always
    add p*, a1
    shfi a1, a1, +5
    shfi b0, a0, -11
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    eint
    ret always

.global mp3_mul28_native
mp3_mul28_native:
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r4
    dint
    mov r0, y0
    macuu y0, r2, a0
    clr a0, always
    add p*, a0
    shfi a0, a0, -16
    mov r1, y0
    mpysu y0, r2, a0
    mov r4, y0
    macsu y0, r0, a0
    add p*, a0
    add 0x0800, a0
    mov a0, b0
    mov r1, y0
    mpy y0, r4, a1
    clr a1, always
    add p*, a1
    shfi a1, a1, +4
    shfi b0, a0, -12
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    eint
    ret always

.global mp3_mul29_native
mp3_mul29_native:
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r4
    dint
    mov r0, y0
    macuu y0, r2, a0
    clr a0, always
    add p*, a0
    shfi a0, a0, -16
    mov r1, y0
    mpysu y0, r2, a0
    mov r4, y0
    macsu y0, r0, a0
    add p*, a0
    add 0x1000, a0
    mov a0, b0
    mov r1, y0
    mpy y0, r4, a1
    clr a1, always
    add p*, a1
    shfi a1, a1, +3
    shfi b0, a0, -13
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    eint
    ret always

.global mp3_mul30_native
mp3_mul30_native:
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r4
    dint
    mov r0, y0
    macuu y0, r2, a0
    clr a0, always
    add p*, a0
    shfi a0, a0, -16
    mov r1, y0
    mpysu y0, r2, a0
    mov r4, y0
    macsu y0, r0, a0
    add p*, a0
    add 0x2000, a0
    mov a0, b0
    mov r1, y0
    mpy y0, r4, a1
    clr a1, always
    add p*, a1
    shfi a1, a1, +2
    shfi b0, a0, -14
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    eint
    ret always

.global mp3_mul31_native
mp3_mul31_native:
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r4
    dint
    mov r0, y0
    macuu y0, r2, a0
    clr a0, always
    add p*, a0
    shfi a0, a0, -16
    mov r1, y0
    mpysu y0, r2, a0
    mov r4, y0
    macsu y0, r0, a0
    add p*, a0
    add 0x4000, a0
    mov a0, b0
    mov r1, y0
    mpy y0, r4, a1
    clr a1, always
    add p*, a1
    shfi a1, a1, +1
    shfi b0, a0, -15
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    eint
    ret always

.global mp3_mul30
mp3_mul30:
    push r0
    push r1
    push r2
    push r4
    push y0
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    call mp3_mul30_native, always
    pop mod0
    pop y0
    pop r4
    pop r2
    pop r1
    pop r0
    ret always

// Word-address stride product, modulo 2^16. Always entered from DSP C with
// interrupts enabled. The product lifetime spans three instructions.
.global mp3_index
mp3_index:
    push y0
    push mod0
    rst 0x0c00, mod0
    set 0x0003, mod0
    mov a1l, y0
    dint
    mpysu y0, a0l, a0
    clr a0, always
    add p*, a0
    eint
    and 0xffff, a0
    pop mod0
    pop y0
    ret always

// index * align_up(frames * stereo_pcm16_bytes, 32), index <= 8,
// 256 <= frames <= 16384. Full 32-bit result, bounded product lifetime.
.global mp3_slot_bytes
mp3_slot_bytes:
    push y0
    push mod0
    rst 0x0c00, mod0
    set 0x0003, mod0
    add 0x7, a1
    and 0xfff8, a1
    mov a1l, y0
    dint
    mpysu y0, a0l, a0
    clr a0, always
    add p*, a0
    eint
    shfi a0, a0, +2
    pop mod0
    pop y0
    ret always

// Private constant-coefficient ABI: A0=x, X1=signed low, X0=adjusted high.
// c = X0*65536 + X1. Full signed-32 x; c must have signed-16 high.
// Two dual-MAC launches form all four partial products exactly. The first
// discarded sum is immaterial; the second sums the cross products. B1 stays
// intact. Both product registers are live only inside this bounded IRQ mask.
// 0x5f26 = MMA A0, signed/signed lane 0, signed/unsigned lane 1, zero base.
// The patched LLVM assembler lacks MMA; encode the documented Teakra form.

.global mp3_coeff27_native
mp3_coeff27_native:
    mov a0h, y0
    mov a0l, y1
    dint
    .short 0x5f26
    .short 0x5f26
    clr b0, always
    add p1, b0
    shfi b0, b0, -16
    add b0, a0
    add 0x0400, a0
    shfi a0, a0, -11
    clr b0, always
    add p0, b0
    shfi b0, b0, +5
    add b0, a0
    eint
    ret always

.global mp3_coeff28_native
mp3_coeff28_native:
    mov a0h, y0
    mov a0l, y1
    dint
    .short 0x5f26
    .short 0x5f26
    clr b0, always
    add p1, b0
    shfi b0, b0, -16
    add b0, a0
    add 0x0800, a0
    shfi a0, a0, -12
    clr b0, always
    add p0, b0
    shfi b0, b0, +4
    add b0, a0
    eint
    ret always

.global mp3_coeff29_native
mp3_coeff29_native:
    mov a0h, y0
    mov a0l, y1
    dint
    .short 0x5f26
    .short 0x5f26
    clr b0, always
    add p1, b0
    shfi b0, b0, -16
    add b0, a0
    add 0x1000, a0
    shfi a0, a0, -13
    clr b0, always
    add p0, b0
    shfi b0, b0, +3
    add b0, a0
    eint
    ret always

.global mp3_coeff30_native
mp3_coeff30_native:
    mov a0h, y0
    mov a0l, y1
    dint
    .short 0x5f26
    .short 0x5f26
    clr b0, always
    add p1, b0
    shfi b0, b0, -16
    add b0, a0
    add 0x2000, a0
    shfi a0, a0, -14
    clr b0, always
    add p0, b0
    shfi b0, b0, +2
    add b0, a0
    eint
    ret always

.global mp3_coeff31_native
mp3_coeff31_native:
    mov a0h, y0
    mov a0l, y1
    dint
    .short 0x5f26
    .short 0x5f26
    clr b0, always
    add p1, b0
    shfi b0, b0, -16
    add b0, a0
    add 0x4000, a0
    shfi a0, a0, -15
    clr b0, always
    add p0, b0
    shfi b0, b0, +1
    add b0, a0
    eint
    ret always
