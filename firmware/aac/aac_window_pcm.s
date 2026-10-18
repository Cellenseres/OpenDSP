// SPDX-License-Identifier: BSD-2-Clause
// Long-window PCM fusion. Same intermediate saturation as window_seg + pcm_out.
// job[6] = old overlap, job[9] = PCM. Other fields match aac_window_seg.
.text
.global aac_window_emit
aac_window_emit:
    push r0
    mov a0l, r0
    addv 0x6, r0
    mov [r0], a1
    pop r0
    cmpv 0x0, a1l
    br .Lemit_fused, neq
    call aac_window_seg, always
    ret always
.Lemit_fused:
    call aac_window_pcm, always
    ret always

.global aac_window_pcm
aac_window_pcm:
    push    r0
    push    r2
    push    r3
    push    r4
    push    r6
    push    r5
    push    y0
    push    sv
    push    cfgj
    mov     a0l, r6
    mov     [r6++], r0              // fwd
    mov     [r6++], r2              // bwd
    mov     [r6++], r4              // window
    mov     [r6++], a0
    mov     a0l, cfgj               // window step
    mov     [r6++], r3              // pairs
    mov     [r6++], a0
    mov     a0l, sv                 // shift
    clr     b1, always              // rounding term: 1 << (-shift - 1) for shift < 0
    add     0x0, a0
    br      .Lwf_rounding, ge
    neg     a0, always
    sub     0x1, a0
    mov     a0l, sv
    mov     0x1, b1
    shfc    b1, b1, always
    add     0x1, a0
    neg     a0, always
    mov     a0l, sv
.Lwf_rounding:
    mov     [r6++], r5              // old overlap, s32
    modr    [r6++]                  // unused temporary output
    mov     [r6++], a1              // variant
    mov     [r6], a0                // direct PCM output
    mov     a0l, r6
    modr    [r3--]
    set     0x0800, mod0            // product shift 1
    cmpv    0x0, a1l
    br      .Lwf_pos_neg, eq
    cmpv    0x1, a1l
    br      .Lwf_neg_pos, eq
    cmpv    0x2, a1l
    br      .Lwf_flat_pos_neg, eq
    br      .Lwf_flat_neg_pos, always

.Lwf_pos_neg:
    dint
    bkrep   r3, .Lwf_pn_end
    mov     [r4++s], y0
    mov     [arrn0+ars0], a1        // +fwd
    clr     a0, always
    mpysu   y0, a1l, a0
    maa     y0, a1h, a0
    add     p*, a0
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0                  // saturate to +-(2^31 - 1)
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    // Preserve the stored-window saturation before adding the old overlap.
    mov     [arrn3+ars0], a1
    add     a1, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r5++]
    modr    [r5++]
    addv    0x0004, r0
    mov     [r4++s], y0
    mov     [arrn2+ars0], a1        // -bwd
    neg     a1, always
    clr     a0, always
    mpysu   y0, a1l, a0
    maa     y0, a1h, a0
    add     p*, a0
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    // Preserve the stored-window saturation before adding the old overlap.
    mov     [arrn3+ars0], a1
    add     a1, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r5++]
    modr    [r5++]
    addv    0xfffc, r2
.Lwf_pn_end:
    eint
    br      .Lwf_done, always

.Lwf_neg_pos:
    dint
    bkrep   r3, .Lwf_np_end
    mov     [r4++s], y0
    mov     [arrn0+ars0], a1        // -fwd
    neg     a1, always
    clr     a0, always
    mpysu   y0, a1l, a0
    maa     y0, a1h, a0
    add     p*, a0
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    // Preserve the stored-window saturation before adding the old overlap.
    mov     [arrn3+ars0], a1
    add     a1, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r5++]
    modr    [r5++]
    addv    0x0004, r0
    mov     [r4++s], y0
    mov     [arrn2+ars0], a1        // +bwd
    clr     a0, always
    mpysu   y0, a1l, a0
    maa     y0, a1h, a0
    add     p*, a0
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    // Preserve the stored-window saturation before adding the old overlap.
    mov     [arrn3+ars0], a1
    add     a1, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r5++]
    modr    [r5++]
    addv    0xfffc, r2
.Lwf_np_end:
    eint
    br      .Lwf_done, always

.Lwf_flat_pos_neg:
    bkrep   r3, .Lwf_fpn_end
    mov     [arrn0+ars0], a0        // +fwd
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    // Preserve the stored-window saturation before adding the old overlap.
    mov     [arrn3+ars0], a1
    add     a1, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r5++]
    modr    [r5++]
    addv    0x0004, r0
    mov     [arrn2+ars0], a0        // -bwd
    neg     a0, always
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    // Preserve the stored-window saturation before adding the old overlap.
    mov     [arrn3+ars0], a1
    add     a1, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r5++]
    modr    [r5++]
    addv    0xfffc, r2
.Lwf_fpn_end:
    br      .Lwf_done, always

.Lwf_flat_neg_pos:
    bkrep   r3, .Lwf_fnp_end
    mov     [arrn0+ars0], a0        // -fwd
    neg     a0, always
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    // Preserve the stored-window saturation before adding the old overlap.
    mov     [arrn3+ars0], a1
    add     a1, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r5++]
    modr    [r5++]
    addv    0x0004, r0
    mov     [arrn2+ars0], a0        // +bwd
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    // Preserve the stored-window saturation before adding the old overlap.
    mov     [arrn3+ars0], a1
    add     a1, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r5++]
    modr    [r5++]
    addv    0xfffc, r2
.Lwf_fnp_end:

.Lwf_done:
    rst     0x0800, mod0
    pop     cfgj
    pop     sv
    pop     y0
    pop     r5
    pop     r6
    pop     r4
    pop     r3
    pop     r2
    pop     r0
    ret     always
