// SPDX-License-Identifier: BSD-2-Clause
// Original 17-bit window coefficients and exact PCM rounding are retained.
.text
.global mp3_pcm_span
mp3_pcm_span:
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
    mov a0l, r7
    nop                         // R7 short-offset address hazard
    mov [r7+0x0002s7], a0
    mov a0l, r1
    mov [r7+0x0005s7], a0
    mov a0l, r4
    mov [r7+0x0006s7], a0
    mov a0l, r5
    mov [r7+0x0007s7], a0
    mov a0l, r3
    mov [r7+0x0004s7], a0
    mov a0l, r6
    clr b0, always
    cmpv 0x0, r6
    br .Lspan_shift_ready, ge
    cmpv 0xffe1, r6
    br .Lspan_shift_ready, lt
    mov r6, a0
    neg a0, always
    sub 0x1, a0
    mov a0l, sv
    mov 0x1, b0
    shfc b0, b0, always
.Lspan_shift_ready:
    mov r6, sv
    mov r1, r0
    addv 0x0000, r0
    mov [arrn0+ars0], a0
    call .Lspan_scale, always
    mov r4, r0
    mov a0l, [r0]
    mov r1, r0
    addv 0x0008, r0
    mov [arrn0+ars0], a0
    call .Lspan_scale, always
    mov r5, r0
    mov a0l, [r0]
    cmpv 0x1, r3
    br .Lspan_lane_1_done, eq
    mov r1, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    call .Lspan_scale, always
    mov r4, r0
    modr [r0++]
    mov a0l, [r0]
    mov r1, r0
    addv 0x000a, r0
    mov [arrn0+ars0], a0
    call .Lspan_scale, always
    mov r5, r0
    modr [r0++]
    mov a0l, [r0]
.Lspan_lane_1_done:
    mov r3, a0
    shfi a0, a0, +5
    mov a0l, r2
    mov r1, r0
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    call .Lspan_scale, always
    mov r4, a1
    mov r2, b1
    add b1, a1
    mov a1l, r0
    mov a0l, [r0]
    mov r1, r0
    addv 0x000c, r0
    mov [arrn0+ars0], a0
    call .Lspan_scale, always
    mov r5, a1
    mov r2, b1
    add b1, a1
    mov a1l, r0
    mov a0l, [r0]
    cmpv 0x1, r3
    br .Lspan_lane_3_done, eq
    mov r3, a0
    shfi a0, a0, +5
    add 0x1, a0
    mov a0l, r2
    mov r1, r0
    addv 0x0006, r0
    mov [arrn0+ars0], a0
    call .Lspan_scale, always
    mov r4, a1
    mov r2, b1
    add b1, a1
    mov a1l, r0
    mov a0l, [r0]
    mov r1, r0
    addv 0x000e, r0
    mov [arrn0+ars0], a0
    call .Lspan_scale, always
    mov r5, a1
    mov r2, b1
    add b1, a1
    mov a1l, r0
    mov a0l, [r0]
.Lspan_lane_3_done:
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
// Fast path shares the rounding constant and shift across all eight lanes.
// Positive shifts retain the fully general, independently tested helper.
.Lspan_scale:
    cmpv 0x0, r6
    br .Lspan_scale_general, gt
    cmpv 0xffe1, r6
    br .Lspan_scale_zero, lt
    add b0, a0
    shfc a0, a0, always
    cmp 0x7fff, a0
    br .Lspan_scale_positive, gt
    cmp 0x8000, a0
    br .Lspan_scale_negative, lt
    ret always
.Lspan_scale_general:
    mov r6, a1
    call mp3_scale_pcm, always
    ret always
.Lspan_scale_zero:
    clr a0, always
    ret always
.Lspan_scale_positive:
    mov 0x7fff, a0
    ret always
.Lspan_scale_negative:
    mov 0x8000, a0
    ret always

// Eight-word input job: xl,xr,lins,pcm,math,window coefficients,channels,shift.
// Fifteen spans, at most four consecutive spans per scheduling slice.
.global mp3_synth_window
mp3_synth_window:
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
    addv 0xfff8, sp
    mov sp, r7
    mov [r0++], a0
    mov a0l, r1
    mov [r0++], a1
    sub a0, a1
    mov a1l, r3
    mov r1, r2
    addv 0x0264, r1
    addv 0x021c, r2
    mov [r0++], a0
    mov a0l, r5
    addv 0x07f0, r5
    mov [r0++], a0
    mov a0l, [r7+0x0005s7]
    mov a0l, [r7+0x0006s7]
    mov [r0++], a0
    mov a0l, [r7+0x0002s7]
    mov [r0++], a0
    mov a0l, [r7+0x0001s7]
    mov [r0++], a0
    mov a0l, [r7+0x0007s7]
    mov a0, b0
    mov [r7+0x0005s7], a1
    add a0, a1
    mov a1l, [r7+0x0005s7]
    shfi a0, a0, +5
    sub b0, a0
    mov [r7+0x0006s7], a1
    add a0, a1
    mov a1l, [r7+0x0006s7]
    mov 0x2, a1
    mov [r7+0x0007s7], a0
    cmp 0x1u8, a0
    br .Lsynth_lane_ready, neq
    mov 0x4, a1
.Lsynth_lane_ready:
    mov a1l, [r7+0x0003s7]
    mov [r0++], a0
    mov a0l, [r7+0x0004s7]
    mov 0xe, r6
.Lsynth_span:
    mov r5, a0
    mov a0l, [r7+0x0000s7]
    mov r1, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    mov a0, [arrn1+ars0]
    mov r1, a0
    mov r3, a1
    add a1, a0
    mov a0l, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0002, r4
    mov a0, [arrn1+ars0]
    mov r1, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0004, r4
    mov a0, [arrn1+ars0]
    mov r1, a0
    mov r3, a1
    add a1, a0
    mov a0l, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0006, r4
    mov a0, [arrn1+ars0]
    mov r2, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0080, r4
    mov a0, [arrn1+ars0]
    mov r2, a0
    mov r3, a1
    add a1, a0
    mov a0l, r0
    addv 0x0002, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0x0082, r4
    mov a0, [arrn1+ars0]
    mov r2, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0xff84, r4
    mov a0, [arrn1+ars0]
    mov r2, a0
    mov r3, a1
    add a1, a0
    mov a0l, r0
    mov [arrn0+ars0], a0
    mov r5, r4
    addv 0xff86, r4
    mov a0, [arrn1+ars0]
    mov r7, a0
    mov r6, a1
    neg a1, always
    add 0xe, a1
    mov a1l, [r7+0x0001s7]
    call mp3_window_row, always
    mov r7, a0
    call mp3_pcm_span, always
    mov r6, a0
    and 0x3, a0
    cmp 0x0u8, a0
    br .Lsynth_no_yield, neq
    // No live product or block loop across the cooperative C callback.
    push mod0
    mov [r7+0x0008s7], a0
    mov a0l, mod0
    push r1
    push r2
    push r3
    push r5
    push r6
    push r7
    call mp3_codec_yield, always
    pop r7
    pop r6
    pop r5
    pop r3
    pop r2
    pop r1
    pop mod0
.Lsynth_no_yield:
    cmpv 0x0, r6
    br .Lsynth_done, eq
    modr [r6--]
    addv 0x24, r1
    addv 0xffdc, r2
    addv 0xfff8, r5
    mov [r7+0x0001s7], a0
    add 0x20, a0
    mov a0l, [r7+0x0001s7]
    mov [r7+0x0007s7], a0
    mov [r7+0x0005s7], a1
    add a0, a1
    mov a1l, [r7+0x0005s7]
    mov [r7+0x0006s7], a1
    sub a0, a1
    mov a1l, [r7+0x0006s7]
    br .Lsynth_span, always
.Lsynth_done:
    mov r7, sp
    addv 0x8, sp
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

.global mp3_window_row
mp3_window_row:
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
    mov [r7+0x0001s7], a0
    mov a0l, r6
    mov 0x0, r5
.Lrow_lane:
    mov [r7+0x0000s7], a0
    mov r5, a1
    add a1, a0
    mov a0l, r0
    mov a0l, r4
    addv 0xf880, r4
    clr b0, always
    clr b1, always
    cmpv 0x0, r6
    br .Lrow_0, eq
    cmpv 0x1, r6
    br .Lrow_1, eq
    cmpv 0x2, r6
    br .Lrow_2, eq
    cmpv 0x3, r6
    br .Lrow_3, eq
    cmpv 0x4, r6
    br .Lrow_4, eq
    cmpv 0x5, r6
    br .Lrow_5, eq
    cmpv 0x6, r6
    br .Lrow_6, eq
    cmpv 0x7, r6
    br .Lrow_7, eq
    cmpv 0x8, r6
    br .Lrow_8, eq
    cmpv 0x9, r6
    br .Lrow_9, eq
    cmpv 0xa, r6
    br .Lrow_10, eq
    cmpv 0xb, r6
    br .Lrow_11, eq
    cmpv 0xc, r6
    br .Lrow_12, eq
    cmpv 0xd, r6
    br .Lrow_13, eq
    br .Lrow_14, always
.Lrow_0:
    dint
    mov 0xffff, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x001a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffe1, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x00d0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00da, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0191, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfdf9, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x080f, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x07d0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x12b4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xea73, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x1bde, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x1747, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x8b38, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x6658, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    addv 0xff80, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x24f0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_1:
    dint
    mov 0xffff, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0018, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffdd, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x00ca, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00de, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x015b, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfdbb, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0820, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x07a0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x1149, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe909, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x1dd8, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x14a8, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x83ff, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x5f28, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    addv 0xff80, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x2468, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_2:
    dint
    mov 0xffff, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0015, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffda, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x00c4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00e1, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0126, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfd7b, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0827, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x0765, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0fdf, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe7a3, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x1f9c, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x11d1, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x7ccb, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x5802, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x2386, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_3:
    dint
    mov 0xffff, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0013, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffd7, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x00be, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00e3, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x00f4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfd39, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0825, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x071e, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0e79, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe643, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x212c, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x0ec0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x75a0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x50eb, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x2249, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_4:
    dint
    mov 0xffff, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0011, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffd3, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x00b7, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00e4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x00c5, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfcf5, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x081b, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x06cb, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0d17, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe4e9, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x2288, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x0b77, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x6e81, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x49e7, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x20b4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_5:
    dint
    mov 0xffff, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0010, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffcf, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x00b0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00e4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0099, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfcb0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0809, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x066c, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0bbc, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe399, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x23b3, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x07f5, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x6772, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x42fa, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x1ec7, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_6:
    dint
    mov 0xfffe, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x000e, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffcb, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x00a9, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00e3, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x006f, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfc69, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x07f0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x05ff, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0a67, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe253, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x24ad, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x043a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x6076, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x3c27, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x1c83, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_7:
    dint
    mov 0xfffe, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x000d, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffc6, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x00a1, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00e0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0048, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfc21, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x07d1, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x0586, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x091a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe11a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x2578, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x0046, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x5991, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x3573, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x19e9, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_8:
    dint
    mov 0xfffe, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x000b, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffc1, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x009a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00dd, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0024, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfbd8, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x07aa, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x0500, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x07d6, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xdfef, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x2616, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfc1a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x52c5, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x2ee2, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x16fc, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_9:
    dint
    mov 0xfffe, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x000a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffbc, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0093, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00d7, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0002, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfb8f, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x077f, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x046b, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x069c, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xded5, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x2687, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xf7b6, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x4c16, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x2876, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x13be, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_10:
    dint
    mov 0xfffd, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0009, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffb7, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x008b, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00d0, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0xffe3, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfb46, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x074e, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x03ca, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x056c, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xddcd, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x26cf, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xf31c, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x4587, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x2236, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x102f, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_11:
    dint
    mov 0xfffd, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0008, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffb1, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0084, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00c8, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0xffc7, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfafd, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0719, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x031a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0447, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xdcda, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x26ee, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xee4b, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x3f1b, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x1c23, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0c54, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_12:
    dint
    mov 0xfffc, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0007, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffab, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x007d, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00bd, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0xffad, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfab4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x06df, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x025d, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x032e, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xdbfd, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x26e7, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe946, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x38d4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x1642, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x082d, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_13:
    dint
    mov 0xfffc, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0007, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xffa5, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0075, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00b1, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0xff96, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfa6c, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x06a2, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x0192, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0221, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xdb38, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x26bc, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xe40e, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x32b4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x1097, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x03be, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_14:
    dint
    mov 0xfffb, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0006, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xff9f, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x006f, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00a3, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0xff81, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xfa26, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x0662, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x00b9, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x0120, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xda8f, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0x266e, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0xdea4, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    mov 0x2cbf, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    addv 0xff80, r0
    addv 0x0080, r4
    mov 0x0b24, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    sub a0, b0
    mov 0xff0a, a1
    mov a1l, y0
    mov a1l, y1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    sub a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94f7
    mov r0, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b1
    clr a0, always
    .short 0x94e7
    mov r4, r2
    mov [arrn2+ars0], a1
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    br .Lrow_store, always
.Lrow_store:
    mov [r7+0x0002s7], a0
    mov r5, a1
    add a1, a0
    mov a0l, r2
    mov b0, [arrn2+ars0]
    cmpv 0x4, r1
    br .Lrow_stereo_a, neq
    addv 0x2, r2
    mov b0, [arrn2+ars0]
    addv 0x6, r2
    br .Lrow_store_b, always
.Lrow_stereo_a:
    addv 0x8, r2
.Lrow_store_b:
    mov b1, [arrn2+ars0]
    cmpv 0x4, r1
    br .Lrow_stereo_next, neq
    addv 0x2, r2
    mov b1, [arrn2+ars0]
    addv 0x4, r5
    br .Lrow_lane_next, always
.Lrow_stereo_next:
    addv 0x2, r5
.Lrow_lane_next:
    cmpv 0x8, r5
    br .Lrow_lane, neq
    mov r7, sp
    addv 0x3, sp
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

.global mp3_synth_pair
mp3_synth_pair:
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
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r7
    nop
    mov [r7+0x0000s7], a0
    mov a0l, r1
    mov [r7+0x0001s7], a0
    mov a0l, r5
    mov [r7+0x0003s7], a0
    mov a0l, r3
    clr b0, always
    mov r1, r0
    addv 0x0700, r0
    mov [arrn0+ars0], a0
    mov r1, r0
    addv 0x0000, r0
    mov [arrn0+ars0], a1
    sub a1, a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x001d, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0080, r0
    mov [arrn0+ars0], a0
    mov r1, r0
    addv 0x0680, r0
    mov [arrn0+ars0], a1
    add a1, a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x00d5, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0600, r0
    mov [arrn0+ars0], a0
    mov r1, r0
    addv 0x0100, r0
    mov [arrn0+ars0], a1
    sub a1, a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x01cb, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0180, r0
    mov [arrn0+ars0], a0
    mov r1, r0
    addv 0x0580, r0
    mov [arrn0+ars0], a1
    add a1, a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x07f5, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0500, r0
    mov [arrn0+ars0], a0
    mov r1, r0
    addv 0x0200, r0
    mov [arrn0+ars0], a1
    sub a1, a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x1421, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0280, r0
    mov [arrn0+ars0], a0
    mov r1, r0
    addv 0x0480, r0
    mov [arrn0+ars0], a1
    add a1, a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x19ae, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0400, r0
    mov [arrn0+ars0], a0
    mov r1, r0
    addv 0x0300, r0
    mov [arrn0+ars0], a1
    sub a1, a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x9271, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0380, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x251e, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov b0, a0
    mov [r7+0x0002s7], a1
    call mp3_scale_pcm, always
    mov r5, r0
    mov a0l, [r0]
    clr b0, always
    mov r1, r0
    addv 0x0704, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x0068, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0604, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x061f, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0504, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x25ff, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0404, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0xfa13, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add a1, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0304, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0xd909, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0204, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0xffd3, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0104, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0x0092, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov r1, r0
    addv 0x0004, r0
    mov [arrn0+ars0], a0
    mov a0, a1
    mov a0l, r4
    mov a0h, r6
    mov 0xfffb, y0
    clr a0, always
    dint
    mpysu y0, r4, a0
    maa y0, r6, a0
    add p*, a0
    add 0x1, a0
    shfi a0, a0, -1
    add a0, b0
    eint
    mov b0, a0
    mov [r7+0x0002s7], a1
    call mp3_scale_pcm, always
    mov r5, r0
    mov r3, a1
    shfi a1, a1, +4
    mov a0, b1
    mov r5, a0
    add a1, a0
    mov a0l, r0
    mov b1, a0
    mov a0l, [r0]
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
