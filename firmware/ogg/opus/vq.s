// SPDX-License-Identifier: BSD-2-Clause
// CELT inner product, bit-exact with libopus fixed point.

// exact 64-bit dot product >> 20
.section .text.opus_inner_prod_shift
.global opus_inner_prod_shift
opus_inner_prod_shift:
    push mod0
    push cfgi
    push cfgj
    push sv
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
    mov a0l, r7
    rst 0x0c00, mod0
    push mod3
    dint
    addv 0x0007, r7
    mov [r7], cfgi
    mov [r7], cfgj
    addv 0xfff9, r7
    mov [r7], a0
    mov a0l, r0
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r4
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r6
    modr [r6--]
    clr a0, always
    clr a1, always
    mov 0x0000, y0
    mpy y0, [r0], a0
    bkrep r6, .Lip_cross
    mov [r0++], y0
    mac y0, [r4++], a1
    macsu y0, [r4--], a0
    mov [r0++], y0
    macus y0, [r4++s], a1
.Lip_cross:
    add p*, a1
    shfi a0, a0, +16
    add a0, a1
    addv 0xfffe, r7
    mov [r7], a0
    mov a0l, r0
    modr [r0++]
    addv 0x0001, r7
    mov [r7], a0
    mov a0l, r4
    modr [r4++]
    addv 0x0002, r7
    mov [r7], a0
    mov a0l, r6
    modr [r6--]
    clr a0, always
    mov 0x0000, y0
    mpy y0, [r0], a0
    bkrep r6, .Lip_low1
    mov [r0++s], y0
    macuu y0, [r4++s], a0
.Lip_low1:
    addv 0x0002, r7
    mov a0l, [r7]
    shfi a0, a0, -16
    add a0, a1
    addv 0xffff, r7
    mov [r7], a0
    mov a0l, r6
    modr [r6--]
    addv 0x0001, r7
    cmp 0x0000u8, a0
    br .Lip_one, eq
    clr a0, always
    or [r7], a0
    bkrep r6, .Lip_low2
    mov [r0++s], y0
    macuu y0, [r4++s], a0
.Lip_low2:
    br .Lip_fold, always
.Lip_one:
    clr a0, always
    or [r7], a0
.Lip_fold:
    add p*, a0
    shfi a0, a0, -16
    add a0, a1
    shfi a1, a1, -4
    mov a1h, [r7]
    addv 0x0001, r7
    mov a1l, [r7]
    pop mod3
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    pop y0
    pop sv
    pop cfgj
    pop cfgi
    pop mod0
    ret always
