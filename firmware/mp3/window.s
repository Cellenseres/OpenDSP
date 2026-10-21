// SPDX-License-Identifier: BSD-2-Clause
// Four polyphase lanes; signed 17-bit window coefficients.
// Job: z32, coefficients (low, correction), output a[4]/b[4], lane step.
// cross = xh*wl + (xl*wl >> 16) + correction*x.
// Round with (cross+1)>>1; mask IRQs during products.
.text

.macro MP3_WINDOW_PRODUCT ptr, array
    clr a0, always
    mpysu [r4++], [\ptr--], a0
    maa y0, [\ptr++], a0
    add p*, a0
    mov [r4++], r6
    cmpv 0x0, r6
    br .Lwc_zero\@, eq
    modr [\ptr--]
    mov [\array+ars0], a1
    modr [\ptr++]
    cmpv 0x0, r6
    br .Lwc_negative\@, lt
    add a1, a0
    br .Lwc_zero\@, always
.Lwc_negative\@:
    sub a1, a0
.Lwc_zero\@:
    add 0x1, a0
    shfi a0, a0, -1
.endm

.global mp3_window_span
mp3_window_span:
    push r0
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push y0
    push mod0
    addv 0xfffd, sp
    mov sp, r7
    nop
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r6
    mov [r6++], a0
    mov a0l, [r7+0x0000s7]
    mov [r6++], a0
    mov a0l, [r7+0x0001s7]
    mov [r6++], a0
    mov a0l, [r7+0x0002s7]
    mov [r6], a0
    mov a0l, r1
    mov 0x0, r5
.Lwindow_lane:
    mov [r7+0x0000s7], a0
    mov r5, a1
    add a1, a0
    mov a0l, r0
    modr [r0++]                 // low word for the first unsigned product
    mov a0l, r2
    addv 0xf881, r2              // y low = z - 15*64 s32 + 1 word
    mov [r7+0x0001s7], a0
    mov a0l, r4
    mov 0x8, r3
    clr b0, always
    clr b1, always
.Lwindow_pair:
    dint
    MP3_WINDOW_PRODUCT r0, arrn0
    tstb r3, 0
    br .Lw_z_negative, eq
    add a0, b0
    br .Lw_z_done, always
.Lw_z_negative:
    sub a0, b0
.Lw_z_done:
    MP3_WINDOW_PRODUCT r0, arrn0
    add a0, b1
    addv 0xfffc, r4
    MP3_WINDOW_PRODUCT r2, arrn2
    add a0, b1
    MP3_WINDOW_PRODUCT r2, arrn2
    tstb r3, 0
    br .Lw_y_positive, eq
    sub a0, b0
    br .Lw_y_done, always
.Lw_y_positive:
    add a0, b0
.Lw_y_done:
    eint
    addv 0xff80, r0
    addv 0x0080, r2
    modr [r3--]
    cmpv 0x0, r3
    br .Lwindow_pair, neq
    mov [r7+0x0002s7], a0
    mov r5, a1
    add a1, a0
    mov a0l, r2
    mov b0, [arrn2+ars0]
    cmpv 0x4, r1
    br .Lwindow_stereo_a, neq
    addv 0x2, r2
    mov b0, [arrn2+ars0]
    addv 0x6, r2
    br .Lwindow_store_b, always
.Lwindow_stereo_a:
    addv 0x8, r2
.Lwindow_store_b:
    mov b1, [arrn2+ars0]
    cmpv 0x4, r1
    br .Lwindow_stereo_next, neq
    addv 0x2, r2
    mov b1, [arrn2+ars0]
    addv 0x4, r5
    br .Lwindow_lane_next, always
.Lwindow_stereo_next:
    addv 0x2, r5
.Lwindow_lane_next:
    cmpv 0x8, r5
    br .Lwindow_lane, neq
    mov r7, sp
    addv 0x3, sp
    pop mod0
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
