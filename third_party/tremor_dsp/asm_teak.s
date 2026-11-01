// SPDX-License-Identifier: BSD-2-Clause
// Preserve interrupt and product modes.
.section .text.tremor_multiply_high
.global tremor_multiply_high
tremor_multiply_high:
    push mod3
    dint
    push mod0
    push y0
    push r0
    push r1
    push r2
    push r4
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r4
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
    mov a0, b0
    mov r1, y0
    mpy y0, r4, a1
    clr a1, always
    add p*, a1
    shfi b0, a0, -16
    add a1, a0
    pop r4
    pop r2
    pop r1
    pop r0
    pop y0
    pop mod0
    pop mod3
    ret always

.section .text.tremor_multiply_shift15
.global tremor_multiply_shift15
tremor_multiply_shift15:
    push mod3
    dint
    push mod0
    push y0
    push r0
    push r1
    push r2
    push r4
    push r5
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r0
    mov a0h, r1
    mov a1l, r2
    mov a1h, r4
    mov r0, y0
    macuu y0, r2, a0
    clr a0, always
    add p*, a0
    mov a0l, r5
    shfi a0, a0, -16
    mov r1, y0
    mpysu y0, r2, a0
    mov r4, y0
    macsu y0, r0, a0
    add p*, a0
    mov a0, b0
    mov r1, y0
    mpy y0, r4, a1
    clr a1, always
    add p*, a1
    shfi a1, a1, +17
    shfi b0, a0, +1
    add a1, a0
    mov r5, a1
    and 0xffff, a1
    shfi a1, a1, -15
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    pop r5
    pop r4
    pop r2
    pop r1
    pop r0
    pop y0
    pop mod0
    pop mod3
    ret always

// Stub for tests; firmware/ogg/task.s overrides.
.section .text.output_kick
.weak output_kick
output_kick:
    ret always

// d[x] = MULT31_SHIFT15(d[x], lookup[y]) along a floor line.
// a0l = d, a1l = lookup + y0,
// b0l = {count, ady, adx, sy * 2, base * 2}.
.section .text.tremor_floor_line
.global tremor_floor_line
tremor_floor_line:
    push mod0
    push x0
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
    mov a1l, r1
    mov b0l, r7
    mov [r7++], r2
    mov 0x0000, r3
.Lfloor_sample:
    mov [r0++], r5
    mov [r0], r4
    addv 0x0001, r1
    mov [r1], r6
    addv 0xffff, r1
    push mod3
    dint
    mov r4, y0
    macuu y0, r6, a0
    clr a0, always
    add p*, a0
    mov a0l, sv
    shfi a0, a0, -16
    mov r5, y0
    mpysu y0, r6, a0
    mov [r1], y0
    macsu y0, r4, a0
    add p*, a0
    mov a0, b0
    mpy y0, r5, a1
    clr a1, always
    add p*, a1
    pop mod3
    shfi a1, a1, +17
    shfi b0, a0, +1
    add a1, a0
    mov sv, a1
    and 0xffff, a1
    shfi a1, a1, -15
    add a1, a0
    shfi a0, a0, +8
    shfi a0, a0, -8
    mov a0l, [r0--]
    mov a0h, [r0]
    addv 0x0002, r0
    mov r2, a1
    sub 0x0001, a1
    br .Lfloor_done, eq
    mov a1l, r2
    // FIFO top-up every 64 samples
    and 0x003f, a1
    br .Lfloor_step, neq
    call output_kick, always
.Lfloor_step:
    mov r3, a1
    add [r7++], a1
    cmp [r7], a1
    br .Lfloor_base, lt
    sub [r7++], a1
    mov a1l, r3
    mov r1, a1
    add [r7], a1
    mov a1l, r1
    addv 0xfffe, r7
    br .Lfloor_sample, always
.Lfloor_base:
    mov a1l, r3
    mov r1, a1
    addv 0x0002, r7
    add [r7], a1
    mov a1l, r1
    addv 0xfffd, r7
    br .Lfloor_sample, always
.Lfloor_done:
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
    pop x0
    pop mod0
    ret always

// a0 / a1 for a0 < a1 << 15.
.section .text.tremor_divide
.global tremor_divide
tremor_divide:
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a1, b0
    shfi b0, b0, +16
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +1
    mov a0, a1
    sub b0, a1
    copy a0, ge
    inc a0, ge
    shfi a0, a0, +24
    shfi a0, a0, -24
    pop mod0
    ret always
