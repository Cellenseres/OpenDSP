// SPDX-License-Identifier: BSD-2-Clause
// Exact fixed-point dataflows; product lifetimes protected in mp3_product.
// X/Y legality: separate native double-word loads, register multiplies.
.text
.global mp3_dct32
mp3_dct32:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push y0
    push y1
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r6
    mov a1l, r5
    mov r6, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x045c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0024, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0048, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x006c, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0090, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0076, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x00b4, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0074, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x00d8, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0072, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x00fc, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0070, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0120, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0144, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x006c, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0168, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x006a, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x018c, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0068, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x01b0, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0066, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x01d4, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0064, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x01f8, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x021c, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0060, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0062, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0xffe4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov b1, a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xe9ef, a1
    mov a1l, x1
    mov 0x4546, a1
    mov a1l, x0
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
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xba45, a1
    mov a1l, x1
    mov 0x539f, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xfb46, a1
    mov a1l, x1
    mov 0x4141, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xde88, a1
    mov a1l, x1
    mov 0x4cf9, a1
    mov a1l, x0
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
    addv 0xfff6, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x6bbf, a1
    mov a1l, x1
    mov 0x7332, a1
    mov a1l, x0
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
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x6742, a1
    mov a1l, x1
    mov 0x5203, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xe9ef, a1
    mov a1l, x1
    mov 0x4546, a1
    mov a1l, x0
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
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xba45, a1
    mov a1l, x1
    mov 0x539f, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov b1, a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff6, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0024, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x4672, a1
    mov a1l, x1
    mov 0x404f, a1
    mov a1l, x0
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
    addv 0xffe2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x3c10, a1
    mov a1l, x1
    mov 0x42e1, a1
    mov a1l, x0
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
    addv 0xffe6, r0
    mov a0, [arrn0+ars0]
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x9f44, a1
    mov a1l, x1
    mov 0x4892, a1
    mov a1l, x0
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
    addv 0xffea, r0
    mov a0, [arrn0+ars0]
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x0e63, a1
    mov a1l, x1
    mov 0x52cb, a1
    mov a1l, x0
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
    addv 0xffee, r0
    mov a0, [arrn0+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x402e, a1
    mov a1l, x1
    mov 0x64e2, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x24a9, a1
    mov a1l, x1
    mov 0x43e2, a1
    mov a1l, x0
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
    addv 0xfff6, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x92c1, a1
    mov a1l, x1
    mov 0x6e3d, a1
    mov a1l, x0
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
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x4e04, a1
    mov a1l, x1
    mov 0x519e, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0010, r0
    mov a0, [arrn0+ars0]
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0010, r0
    mov a0, [arrn0+ars0]
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0010, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xe9ef, a1
    mov a1l, x1
    mov 0x4546, a1
    mov a1l, x0
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
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xba45, a1
    mov a1l, x1
    mov 0x539f, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffee, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xfb46, a1
    mov a1l, x1
    mov 0x4141, a1
    mov a1l, x0
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
    addv 0x000e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xde88, a1
    mov a1l, x1
    mov 0x4cf9, a1
    mov a1l, x0
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
    addv 0x000a, r0
    mov a0, [arrn0+ars0]
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x6bbf, a1
    mov a1l, x1
    mov 0x7332, a1
    mov a1l, x0
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
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x6742, a1
    mov a1l, x1
    mov 0x5203, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xe9ef, a1
    mov a1l, x1
    mov 0x4546, a1
    mov a1l, x0
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
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xba45, a1
    mov a1l, x1
    mov 0x539f, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    add a1, a0
    mov a0, [arrn0+ars0]
    mov b1, a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe6, r0
    mov a0, [arrn0+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    addv 0x000e, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0xffea, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0xfff6, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    mov r6, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x045c, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xc251, a1
    mov a1l, x1
    mov 0x4014, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0068, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0024, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x45bd, a1
    mov a1l, x1
    mov 0x40b3, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0048, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x2d6d, a1
    mov a1l, x1
    mov 0x41fa, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0066, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x006c, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x3421, a1
    mov a1l, x1
    mov 0x43f9, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0060, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0090, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x1bc4, a1
    mov a1l, x1
    mov 0x46cc, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0076, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x00b4, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x9cf0, a1
    mov a1l, x1
    mov 0x4a9e, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0070, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x00d8, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x3711, a1
    mov a1l, x1
    mov 0x4fae, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x00fc, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x1ea7, a1
    mov a1l, x1
    mov 0x5660, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0120, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xf6eb, a1
    mov a1l, x1
    mov 0x5f4d, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0056, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0144, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xcf26, a1
    mov a1l, x1
    mov 0x6b70, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0050, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0168, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x1db3, a1
    mov a1l, x1
    mov 0x7c7d, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0058, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x018c, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x1a97, a1
    mov a1l, x1
    mov 0x4ad8, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x005e, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x01b0, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x8d96, a1
    mov a1l, x1
    mov 0x5efd, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0048, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x01d4, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x5790, a1
    mov a1l, x1
    mov 0x41d9, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x004e, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x01f8, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x20cf, a1
    mov a1l, x1
    mov 0x6d0b, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0046, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x021c, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x22fb, a1
    mov a1l, x1
    mov 0x5185, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0040, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0030, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0046, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffce, r0
    mov a0, [arrn0+ars0]
    addv 0x002a, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffd4, r0
    mov a0, [arrn0+ars0]
    addv 0x0026, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffd8, r0
    mov a0, [arrn0+ars0]
    addv 0x003e, r0
    mov [arrn0+ars0], a0
    addv 0x0016, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffc0, r0
    mov a0, [arrn0+ars0]
    addv 0x003a, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffc4, r0
    mov a0, [arrn0+ars0]
    addv 0x0044, r0
    mov [arrn0+ars0], a0
    addv 0xfff8, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffba, r0
    mov a0, [arrn0+ars0]
    addv 0x004c, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffb2, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xe9ef, a1
    mov a1l, x1
    mov 0x4546, a1
    mov a1l, x0
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
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xba45, a1
    mov a1l, x1
    mov 0x539f, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xfb46, a1
    mov a1l, x1
    mov 0x4141, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xde88, a1
    mov a1l, x1
    mov 0x4cf9, a1
    mov a1l, x0
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
    addv 0xfff6, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x6bbf, a1
    mov a1l, x1
    mov 0x7332, a1
    mov a1l, x0
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
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x6742, a1
    mov a1l, x1
    mov 0x5203, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xe9ef, a1
    mov a1l, x1
    mov 0x4546, a1
    mov a1l, x0
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
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xba45, a1
    mov a1l, x1
    mov 0x539f, a1
    mov a1l, x0
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
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x002e, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x4672, a1
    mov a1l, x1
    mov 0x404f, a1
    mov a1l, x0
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
    addv 0xffd8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x002e, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x3c10, a1
    mov a1l, x1
    mov 0x42e1, a1
    mov a1l, x0
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
    addv 0xffd8, r0
    mov a0, [arrn0+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x9f44, a1
    mov a1l, x1
    mov 0x4892, a1
    mov a1l, x0
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
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x0e63, a1
    mov a1l, x1
    mov 0x52cb, a1
    mov a1l, x0
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
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0x002e, r0
    mov [arrn0+ars0], a0
    addv 0x0016, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x402e, a1
    mov a1l, x1
    mov 0x64e2, a1
    mov a1l, x0
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
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x24a9, a1
    mov a1l, x1
    mov 0x43e2, a1
    mov a1l, x0
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
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    addv 0xfff8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x92c1, a1
    mov a1l, x1
    mov 0x6e3d, a1
    mov a1l, x0
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
    addv 0xffd8, r0
    mov a0, [arrn0+ars0]
    addv 0x002e, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x4e04, a1
    mov a1l, x1
    mov 0x519e, a1
    mov a1l, x0
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
    addv 0xffd8, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0028, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffc8, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0032, r0
    mov a0, [arrn0+ars0]
    addv 0xffd6, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0022, r0
    mov a0, [arrn0+ars0]
    addv 0xffd8, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x002e, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffea, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    addv 0x0012, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffee, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    addv 0xfff6, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xe9ef, a1
    mov a1l, x1
    mov 0x4546, a1
    mov a1l, x0
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
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xba45, a1
    mov a1l, x1
    mov 0x539f, a1
    mov a1l, x0
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
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffc8, r0
    mov [arrn0+ars0], a0
    addv 0xffe6, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xfb46, a1
    mov a1l, x1
    mov 0x4141, a1
    mov a1l, x0
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
    addv 0x0016, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xde88, a1
    mov a1l, x1
    mov 0x4cf9, a1
    mov a1l, x0
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
    addv 0x000a, r0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x6bbf, a1
    mov a1l, x1
    mov 0x7332, a1
    mov a1l, x0
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
    addv 0x000a, r0
    mov a0, [arrn0+ars0]
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x6742, a1
    mov a1l, x1
    mov 0x5203, a1
    mov a1l, x0
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
    addv 0x0016, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffea, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0xfff6, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xe9ef, a1
    mov a1l, x1
    mov 0x4546, a1
    mov a1l, x0
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
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xba45, a1
    mov a1l, x1
    mov 0x539f, a1
    mov a1l, x0
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
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x799a, a1
    mov a1l, x1
    mov 0x5a82, a1
    mov a1l, x0
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
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov a0, a1
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0xfff6, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x000a, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    add a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffd8, r0
    mov [arrn0+ars0], a0
    addv 0x0020, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0032, r0
    mov [arrn0+ars0], a0
    addv 0xffd8, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffd8, r0
    mov [arrn0+ars0], a0
    addv 0x0016, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    addv 0x0022, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffb2, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    add a1, a0
    mov a0, [arrn0+ars0]
    mov b1, a0
    addv 0xffe4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x004c, r0
    mov a0, [arrn0+ars0]
    addv 0xffce, r0
    mov [arrn0+ars0], a0
    addv 0x000a, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0xffe6, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    addv 0x004c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x004c, r0
    mov [arrn0+ars0], a0
    addv 0xffba, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffba, r0
    mov [arrn0+ars0], a0
    addv 0x001e, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0xffd4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffd4, r0
    mov [arrn0+ars0], a0
    addv 0x0042, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0042, r0
    mov [arrn0+ars0], a0
    addv 0xffd4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffd4, r0
    mov [arrn0+ars0], a0
    addv 0x0014, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffdc, r0
    mov [arrn0+ars0], a0
    addv 0x0046, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0046, r0
    mov [arrn0+ars0], a0
    addv 0xffc4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffc4, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x005a, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffee, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x0026, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe4, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x0046, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffda, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x003a, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe4, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x0056, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x002a, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x004a, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe4, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0x0036, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    addv 0xffee, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov a0, [arrn1+ars0]
    pop mod0
    pop y1
    pop y0
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always

.global mp3_alias
mp3_alias:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push y0
    push y1
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r6
    mov a1l, r5
    mov r6, r0
    mov 0x1f, r3
    clr a0, always
    bkrep r3, .Lalias_scan_end
    mov [r0++], a1
    or a1, a0
.Lalias_scan_end:
    cmp 0x0u8, a0
    br .Lalias_done, eq
    mov r6, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0x2a02, a1
    mov a1l, x1
    mov 0x36e1, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0x7f9b, a1
    mov a1l, x1
    mov 0x20ed, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0x7f9b, a1
    mov a1l, x1
    mov 0x20ed, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0x2a02, a1
    mov a1l, x1
    mov 0x36e1, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    mov 0x7600, a1
    mov a1l, x1
    mov 0x386e, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0xdd37, a1
    mov a1l, x1
    mov 0x1e31, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    mov 0xdd37, a1
    mov a1l, x1
    mov 0x1e31, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0x7600, a1
    mov a1l, x1
    mov 0x386e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov r5, r0
    addv 0x0076, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    mov 0xa3ca, a1
    mov a1l, x1
    mov 0x3cc7, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0x604b, a1
    mov a1l, x1
    mov 0x140e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov r5, r0
    addv 0x0074, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    mov 0x604b, a1
    mov a1l, x1
    mov 0x140e, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xa3ca, a1
    mov a1l, x1
    mov 0x3cc7, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov r5, r0
    addv 0x0072, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov 0x65c2, a1
    mov a1l, x1
    mov 0x3eee, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov 0x772e, a1
    mov a1l, x1
    mov 0x0ba4, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov r5, r0
    addv 0x0070, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov 0x772e, a1
    mov a1l, x1
    mov 0x0ba4, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov 0x65c2, a1
    mov a1l, x1
    mov 0x3eee, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    mov 0x4752, a1
    mov a1l, x1
    mov 0x3fb6, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0x80e8, a1
    mov a1l, x1
    mov 0x060e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov r5, r0
    addv 0x006c, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    mov 0x80e8, a1
    mov a1l, x1
    mov 0x060e, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0x4752, a1
    mov a1l, x1
    mov 0x3fb6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov r5, r0
    addv 0x006a, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov 0xf110, a1
    mov a1l, x1
    mov 0x3ff2, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0x2e25, a1
    mov a1l, x1
    mov 0x029f, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov r5, r0
    addv 0x0068, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov 0x2e25, a1
    mov a1l, x1
    mov 0x029f, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0xf110, a1
    mov a1l, x1
    mov 0x3ff2, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov r5, r0
    addv 0x0066, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    mov 0x5936, a1
    mov a1l, x1
    mov 0x3ffe, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0xa178, a1
    mov a1l, x1
    mov 0x00e9, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov r5, r0
    addv 0x0064, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    mov 0xa178, a1
    mov a1l, x1
    mov 0x00e9, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0x5936, a1
    mov a1l, x1
    mov 0x3ffe, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov 0xe350, a1
    mov a1l, x1
    mov 0x4000, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    mov [arrn0+ars0], a0
    mov 0x9ed4, a1
    mov a1l, x1
    mov 0x003d, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov r5, r0
    addv 0x0060, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov 0x9ed4, a1
    mov a1l, x1
    mov 0x003d, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    mov [arrn0+ars0], a0
    mov 0xe350, a1
    mov a1l, x1
    mov 0x4000, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov r5, r0
    addv 0x005e, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x0010, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xfffe, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xfff6, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xfff2, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xffee, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0014, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xffea, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0018, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xffe6, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x001c, r4
    mov a0, [arrn1+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xffe2, r4
    mov a0, [arrn1+ars0]
.Lalias_done:
    pop mod0
    pop y1
    pop y0
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always

.global mp3_imdct_long
mp3_imdct_long:
    push r0
    push r1
    mov b0l, r0
    mov [r0], r1
    cmpv 0x4000, r1
    pop r1
    pop r0
    br mp3_imdct_stop, eq
    br mp3_imdct_normal, always
.global mp3_imdct_normal
mp3_imdct_normal:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push y0
    push y1
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r6
    mov a1l, r7
    mov b0l, r3
    mov b1l, r5
    mov r6, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    neg a0, always
    mov r5, r0
    addv 0x0076, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0074, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0072, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0070, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    neg a0, always
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x006c, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x006a, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0068, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    neg a0, always
    mov r5, r0
    addv 0x0066, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0064, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0060, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    neg a0, always
    mov r5, r0
    addv 0x005e, r0
    mov a0, [arrn0+ars0]
    mov b1, a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    add a1, a0
    addv 0xffde, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0066, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0xec84, a1
    mov a1l, x1
    mov 0x3c24, a1
    mov a1l, x0
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
    addv 0x0010, r0
    mov a0, [arrn0+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0xdf42, a1
    mov a1l, x1
    mov 0x3107, a1
    mov a1l, x0
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
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x0d42, a1
    mov a1l, x1
    mov 0x0b1d, a1
    mov a1l, x0
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
    addv 0xffea, r0
    mov a0, [arrn0+ars0]
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0018, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    addv 0xfff0, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x001a, r0
    mov a0, [arrn0+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    addv 0x0026, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffda, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    addv 0x0022, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0x1716, a1
    mov a1l, x1
    mov 0x3f07, a1
    mov a1l, x0
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
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xa871, a1
    mov a1l, x1
    mov 0x15e4, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0x6ea5, a1
    mov a1l, x1
    mov 0x2923, a1
    mov a1l, x0
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
    addv 0xfff4, r0
    mov a0, [arrn0+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfff8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xffec, r0
    mov a0, [arrn0+ars0]
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0022, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffde, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0012, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x001a, r0
    mov a0, [arrn0+ars0]
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    addv 0xffe4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0x0020, r0
    mov a0, [arrn0+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfff4, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0x001c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0026, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov r6, r0
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov r5, r0
    addv 0x0058, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    addv 0xfff6, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0074, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    addv 0xfff0, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0xec84, a1
    mov a1l, x1
    mov 0x3c24, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0xdf42, a1
    mov a1l, x1
    mov 0x3107, a1
    mov a1l, x0
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
    addv 0xffde, r0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0018, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x0d42, a1
    mov a1l, x1
    mov 0x0b1d, a1
    mov a1l, x0
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
    addv 0x000a, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0018, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0010, r0
    mov a0, [arrn0+ars0]
    addv 0xffe4, r0
    mov [arrn0+ars0], a0
    addv 0xffe6, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x001c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x000c, r0
    mov a0, [arrn0+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0018, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x001e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0xffea, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0x1716, a1
    mov a1l, x1
    mov 0x3f07, a1
    mov a1l, x0
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
    addv 0xffee, r0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0018, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xa871, a1
    mov a1l, x1
    mov 0x15e4, a1
    mov a1l, x0
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
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0x6ea5, a1
    mov a1l, x1
    mov 0x2923, a1
    mov a1l, x0
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
    addv 0x000c, r0
    mov a0, [arrn0+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0xfff8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    addv 0x0018, r0
    mov a0, [arrn0+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0012, r0
    mov a0, [arrn0+ars0]
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0xfff2, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    addv 0x000a, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0xfff2, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    addv 0x001a, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffdc, r0
    mov a0, [arrn0+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0xfff8, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x000c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0012, r0
    mov a0, [arrn0+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xffe6, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0xdeb8, a1
    mov a1l, x1
    mov 0x2b3d, a1
    mov a1l, x0
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
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov 0x8d4c, a1
    mov a1l, x1
    mov 0x2f30, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov 0x8d4c, a1
    mov a1l, x1
    mov 0x2f30, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov 0xdeb8, a1
    mov a1l, x1
    mov 0x2b3d, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    mov [arrn0+ars0], a0
    mov 0x67f2, a1
    mov a1l, x1
    mov 0x3ff0, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0072, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xa8f1, a1
    mov a1l, x1
    mov 0x02cb, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    mov [arrn0+ars0], a0
    mov 0xa8f1, a1
    mov a1l, x1
    mov 0x02cb, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0072, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0x67f2, a1
    mov a1l, x1
    mov 0x3ff0, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    addv 0x0016, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x005c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0018, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x005c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    mov 0x1de7, a1
    mov a1l, x1
    mov 0x2263, a1
    mov a1l, x0
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
    addv 0xffe2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0x2022, a1
    mov a1l, x1
    mov 0x35fa, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov 0x2022, a1
    mov a1l, x1
    mov 0x35fa, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov 0x1de7, a1
    mov a1l, x1
    mov 0x2263, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0xa243, a1
    mov a1l, x1
    mov 0x3e7c, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    mov 0x2586, a1
    mov a1l, x1
    mov 0x0dda, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0x2586, a1
    mov a1l, x1
    mov 0x0dda, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    mov 0xa243, a1
    mov a1l, x1
    mov 0x3e7c, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov 0x4a0b, a1
    mov a1l, x1
    mov 0x1d8d, a1
    mov a1l, x0
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
    addv 0x0024, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffda, r0
    mov [arrn0+ars0], a0
    mov 0xc913, a1
    mov a1l, x1
    mov 0x38c5, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0xc913, a1
    mov a1l, x1
    mov 0x38c5, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x4a0b, a1
    mov a1l, x1
    mov 0x1d8d, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0xb2d1, a1
    mov a1l, x1
    mov 0x3d0a, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0056, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xc38a, a1
    mov a1l, x1
    mov 0x133f, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0028, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0xc38a, a1
    mov a1l, x1
    mov 0x133f, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0056, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xb2d1, a1
    mov a1l, x1
    mov 0x3d0a, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffdc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0066, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0066, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov 0xc38a, a1
    mov a1l, x1
    mov 0x133f, a1
    mov a1l, x0
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
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xb2d1, a1
    mov a1l, x1
    mov 0x3d0a, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffde, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    mov 0xb2d1, a1
    mov a1l, x1
    mov 0x3d0a, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0xc38a, a1
    mov a1l, x1
    mov 0x133f, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xc913, a1
    mov a1l, x1
    mov 0x38c5, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x006a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe4, r0
    mov [arrn0+ars0], a0
    mov 0x4a0b, a1
    mov a1l, x1
    mov 0x1d8d, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0018, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0x4a0b, a1
    mov a1l, x1
    mov 0x1d8d, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x006a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe4, r0
    mov [arrn0+ars0], a0
    mov 0xc913, a1
    mov a1l, x1
    mov 0x38c5, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    mov 0x2586, a1
    mov a1l, x1
    mov 0x0dda, a1
    mov a1l, x0
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
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0xa243, a1
    mov a1l, x1
    mov 0x3e7c, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    mov 0xa243, a1
    mov a1l, x1
    mov 0x3e7c, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x2586, a1
    mov a1l, x1
    mov 0x0dda, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0x2022, a1
    mov a1l, x1
    mov 0x35fa, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffde, r0
    mov [arrn0+ars0], a0
    mov 0x1de7, a1
    mov a1l, x1
    mov 0x2263, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x001e, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0x1de7, a1
    mov a1l, x1
    mov 0x2263, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffde, r0
    mov [arrn0+ars0], a0
    mov 0x2022, a1
    mov a1l, x1
    mov 0x35fa, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    addv 0x000c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffd2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0024, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0034, r0
    mov [arrn0+ars0], a0
    mov 0xa8f1, a1
    mov a1l, x1
    mov 0x02cb, a1
    mov a1l, x0
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
    addv 0xffe4, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0x67f2, a1
    mov a1l, x1
    mov 0x3ff0, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffea, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0036, r0
    mov [arrn0+ars0], a0
    mov 0x67f2, a1
    mov a1l, x1
    mov 0x3ff0, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    mov 0xa8f1, a1
    mov a1l, x1
    mov 0x02cb, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0x8d4c, a1
    mov a1l, x1
    mov 0x2f30, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffca, r0
    mov [arrn0+ars0], a0
    mov 0xdeb8, a1
    mov a1l, x1
    mov 0x2b3d, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x001a, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0xdeb8, a1
    mov a1l, x1
    mov 0x2b3d, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffca, r0
    mov [arrn0+ars0], a0
    mov 0x8d4c, a1
    mov a1l, x1
    mov 0x2f30, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x002e, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffdc, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    mov r7, r4
    mov a0, [arrn1+ars0]
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    pop mod0
    pop y1
    pop y0
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always

.global mp3_imdct_stop
mp3_imdct_stop:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push y0
    push y1
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r6
    mov a1l, r7
    mov b0l, r3
    mov b1l, r5
    mov r6, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    neg a0, always
    mov r5, r0
    addv 0x0076, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0074, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0072, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0070, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    neg a0, always
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x006c, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x006a, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0068, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    neg a0, always
    mov r5, r0
    addv 0x0066, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0064, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0060, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    add a1, a0
    neg a0, always
    mov r5, r0
    addv 0x005e, r0
    mov a0, [arrn0+ars0]
    mov b1, a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    add a1, a0
    addv 0xffde, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0066, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0xec84, a1
    mov a1l, x1
    mov 0x3c24, a1
    mov a1l, x0
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
    addv 0x0010, r0
    mov a0, [arrn0+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0xdf42, a1
    mov a1l, x1
    mov 0x3107, a1
    mov a1l, x0
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
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x0d42, a1
    mov a1l, x1
    mov 0x0b1d, a1
    mov a1l, x0
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
    addv 0xffea, r0
    mov a0, [arrn0+ars0]
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0018, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    addv 0xfff0, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfffe, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x001a, r0
    mov a0, [arrn0+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    addv 0x0026, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xffdc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0024, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffda, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    addv 0x0022, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0x1716, a1
    mov a1l, x1
    mov 0x3f07, a1
    mov a1l, x0
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
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xa871, a1
    mov a1l, x1
    mov 0x15e4, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0x6ea5, a1
    mov a1l, x1
    mov 0x2923, a1
    mov a1l, x0
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
    addv 0xfff4, r0
    mov a0, [arrn0+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfff8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xffec, r0
    mov a0, [arrn0+ars0]
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0022, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffde, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0012, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x001a, r0
    mov a0, [arrn0+ars0]
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    addv 0xffe4, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0x0020, r0
    mov a0, [arrn0+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfff4, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0x001c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0026, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov r6, r0
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    add a1, a0
    mov r5, r0
    addv 0x0058, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    addv 0xfff6, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0074, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    addv 0xfff0, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0xec84, a1
    mov a1l, x1
    mov 0x3c24, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0xdf42, a1
    mov a1l, x1
    mov 0x3107, a1
    mov a1l, x0
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
    addv 0xffde, r0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0018, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0x0d42, a1
    mov a1l, x1
    mov 0x0b1d, a1
    mov a1l, x0
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
    addv 0x000a, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0018, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    addv 0x0010, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0010, r0
    mov a0, [arrn0+ars0]
    addv 0xffe4, r0
    mov [arrn0+ars0], a0
    addv 0xffe6, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x001c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x000c, r0
    mov a0, [arrn0+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0xffe8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0018, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x001e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0xffea, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0x1716, a1
    mov a1l, x1
    mov 0x3f07, a1
    mov a1l, x0
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
    addv 0xffee, r0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    addv 0x0018, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xa871, a1
    mov a1l, x1
    mov 0x15e4, a1
    mov a1l, x0
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
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    add a1, a0
    mov 0x6ea5, a1
    mov a1l, x1
    mov 0x2923, a1
    mov a1l, x0
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
    addv 0x000c, r0
    mov a0, [arrn0+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0xfff8, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    addv 0x0018, r0
    mov a0, [arrn0+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0012, r0
    mov a0, [arrn0+ars0]
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0xfff2, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    addv 0x000a, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0004, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0xfff2, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    addv 0x001a, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffdc, r0
    mov a0, [arrn0+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov b1, a1
    sub a1, a0
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0xfff8, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x000c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    addv 0xfffc, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0x0012, r0
    mov a0, [arrn0+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0xffe6, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0xdeb8, a1
    mov a1l, x1
    mov 0x2b3d, a1
    mov a1l, x0
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
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov 0x8d4c, a1
    mov a1l, x1
    mov 0x2f30, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov 0x8d4c, a1
    mov a1l, x1
    mov 0x2f30, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov 0xdeb8, a1
    mov a1l, x1
    mov 0x2b3d, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    addv 0x0016, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x001e, r0
    mov a0, [arrn0+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    mov 0x1de7, a1
    mov a1l, x1
    mov 0x2263, a1
    mov a1l, x0
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
    addv 0xffe2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0x2022, a1
    mov a1l, x1
    mov 0x35fa, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0012, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov 0x2022, a1
    mov a1l, x1
    mov 0x35fa, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov 0x1de7, a1
    mov a1l, x1
    mov 0x2263, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov 0x4a0b, a1
    mov a1l, x1
    mov 0x1d8d, a1
    mov a1l, x0
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
    addv 0x0024, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffda, r0
    mov [arrn0+ars0], a0
    mov 0xc913, a1
    mov a1l, x1
    mov 0x38c5, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0xc913, a1
    mov a1l, x1
    mov 0x38c5, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x4a0b, a1
    mov a1l, x1
    mov 0x1d8d, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov 0xc38a, a1
    mov a1l, x1
    mov 0x133f, a1
    mov a1l, x0
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
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xb2d1, a1
    mov a1l, x1
    mov 0x3d0a, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffe6, r0
    mov a0, [arrn0+ars0]
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    mov 0xb2d1, a1
    mov a1l, x1
    mov 0x3d0a, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0xc38a, a1
    mov a1l, x1
    mov 0x133f, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x2586, a1
    mov a1l, x1
    mov 0x0dda, a1
    mov a1l, x0
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
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0xa243, a1
    mov a1l, x1
    mov 0x3e7c, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov 0xa243, a1
    mov a1l, x1
    mov 0x3e7c, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x2586, a1
    mov a1l, x1
    mov 0x0dda, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x006e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    addv 0x000c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffe2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0024, r0
    mov [arrn0+ars0], a0
    mov 0xa8f1, a1
    mov a1l, x1
    mov 0x02cb, a1
    mov a1l, x0
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
    addv 0xffe4, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0x67f2, a1
    mov a1l, x1
    mov 0x3ff0, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0006, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov 0x67f2, a1
    mov a1l, x1
    mov 0x3ff0, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    mov 0xa8f1, a1
    mov a1l, x1
    mov 0x02cb, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    mov r5, r0
    addv 0x006a, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0024, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffe2, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    mov r7, r4
    mov a0, [arrn1+ars0]
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    pop mod0
    pop y1
    pop y0
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always

.global mp3_imdct_short
mp3_imdct_short:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push y0
    push y1
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r6
    mov a1l, r7
    mov b0l, r3
    mov b1l, r5
    mov r6, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x0006, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    sub a1, a0
    addv 0xfffa, r0
    mov a0, [arrn0+ars0]
    mov b1, a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x007a, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x0012, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov r6, r0
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    sub a1, a0
    mov r5, r0
    addv 0x0074, r0
    mov a0, [arrn0+ars0]
    mov b1, a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x007e, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007e, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    addv 0xfffa, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r7, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0076, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x0076, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    addv 0xfffc, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfff0, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r7, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff4, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfff4, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r7, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x000e, r0
    mov a0, [arrn0+ars0]
    mov r7, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x0008, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x006a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0068, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    sub a1, a0
    addv 0xffea, r0
    mov a0, [arrn0+ars0]
    mov b1, a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0068, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffec, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001a, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x0014, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0066, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov r6, r0
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    sub a1, a0
    mov r5, r0
    addv 0x0062, r0
    mov a0, [arrn0+ars0]
    mov b1, a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x007c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x007c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    addv 0xffea, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0016, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffec, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    addv 0x000a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffe8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0014, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    addv 0xffea, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x000e, r0
    mov a0, [arrn0+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe6, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    addv 0x0014, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe8, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    addv 0xfff2, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0016, r0
    mov a0, [arrn0+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x000a, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x006a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    addv 0x000c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x005c, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    sub a1, a0
    addv 0xffe2, r0
    mov a0, [arrn0+ars0]
    mov b1, a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    addv 0x0020, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x005c, r4
    mov [arrn1+ars0], a1
    add a1, a0
    addv 0xffe4, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    addv 0x000e, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0010, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    addv 0x0016, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x005a, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    mov r6, r0
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0xfff4, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe2, r0
    mov [arrn0+ars0], a0
    mov 0xf5cd, a1
    mov a1l, x1
    mov 0x376d, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    add 0x1, a0
    shfi a0, a0, -1
    mov a0, a1
    mov r6, r0
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    sub a1, a0
    mov r5, r0
    addv 0x0056, r0
    mov a0, [arrn0+ars0]
    mov b1, a1
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov r6, r0
    addv 0x0022, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0078, r4
    mov [arrn1+ars0], a1
    add a1, a0
    mov r5, r0
    addv 0x0078, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffde, r0
    mov [arrn0+ars0], a0
    addv 0xffe2, r4
    mov [arrn1+ars0], a1
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001e, r0
    mov [arrn0+ars0], a0
    neg a0, always
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffe, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xfffe, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    addv 0xfff8, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0018, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    addv 0xffe0, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0008, r0
    mov a0, [arrn0+ars0]
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    mov 0xe2a4, a1
    mov a1l, x1
    mov 0x187e, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffdc, r0
    mov [arrn0+ars0], a0
    mov 0xd79c, a1
    mov a1l, x1
    mov 0x3b21, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    addv 0x0012, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    addv 0x0012, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xffde, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    addv 0x0002, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    mov 0xd524, a1
    mov a1l, x1
    mov 0x3f74, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff0, r0
    mov [arrn0+ars0], a0
    mov 0x8a85, a1
    mov a1l, x1
    mov 0x085b, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    mov a0, [arrn0+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    addv 0x0004, r0
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    sub a1, a0
    addv 0x0020, r0
    mov a0, [arrn0+ars0]
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    mov 0xf280, a1
    mov a1l, x1
    mov 0x26f6, a1
    mov a1l, x0
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
    mov a0, [arrn0+ars0]
    mov a0, b1
    addv 0xfff6, r0
    mov [arrn0+ars0], a0
    mov 0x4d16, a1
    mov a1l, x1
    mov 0x32c6, a1
    mov a1l, x0
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
    mov a0, a1
    mov b1, a0
    add a1, a0
    mov a0, [arrn0+ars0]
    mov r7, r0
    mov [arrn0+ars0], a0
    mov r6, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    mov r5, r0
    addv 0x0074, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff2, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0010, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffea, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfff8, r0
    mov [arrn0+ars0], a0
    mov r7, r4
    mov a0, [arrn1+ars0]
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x001c, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffe0, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfffc, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xfffa, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0x0024, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    addv 0xffe2, r0
    mov [arrn0+ars0], a0
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    pop mod0
    pop y1
    pop y0
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0
    ret always
