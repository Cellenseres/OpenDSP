// SPDX-License-Identifier: BSD-2-Clause
// s32 mul16(s16 a, s16 b)
.text
.global mul16
mul16:
    push    y0
    mov     a0l, y0
    dint
    mpy     y0, a1l, a0
    clr     a0, always
    mac     y0, a1l, a0             // a0 = product
    eint
    pop     y0
    ret     always
