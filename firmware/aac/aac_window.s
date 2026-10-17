// SPDX-License-Identifier: BSD-2-Clause
// AAC windowing and output, bit-exact with library/source/aac/filterbank.c.
// s32 values high word first.
.text

// void aac_window_seg(const u16 *job): fwd, bwd, win, wstep, pairs, shift,
// (unused), out, variant
// Pairs of imdct() outputs straight from the post-twiddled z: the even one
// walks forward through z (fwd, +4 words), the odd one backward (bwd, -4).
// Each value v (sign from the variant) becomes
//   shift_sat(mul_q15(v, w), shift)       w = *win, win += wstep
// or shift_sat(v, shift) when flat; written as s32 to out.
// variant bit 0: even negative and odd positive (else the other way round),
// bit 1: flat (no window).
.global aac_window_seg
aac_window_seg:
    push    r0
    push    r2
    push    r3
    push    r4
    push    r6
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
    br      .Lws_rounding, ge
    neg     a0, always
    sub     0x1, a0
    mov     a0l, sv
    mov     0x1, b1
    shfc    b1, b1, always
    add     0x1, a0
    neg     a0, always
    mov     a0l, sv
.Lws_rounding:
    modr    [r6++]                  // unused job word
    mov     [r6++], a0              // out
    mov     [r6], a1                // variant
    mov     a0l, r6
    modr    [r3--]
    set     0x0800, mod0            // product shift 1
    cmpv    0x0, a1l
    br      .Lws_pos_neg, eq
    cmpv    0x1, a1l
    br      .Lws_neg_pos, eq
    cmpv    0x2, a1l
    br      .Lws_flat_pos_neg, eq
    br      .Lws_flat_neg_pos, always

.Lws_pos_neg:
    dint
    bkrep   r3, .Lws_pn_end
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
    mov     a0h, [r6++]
    mov     a0l, [r6++]
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
    mov     a0h, [r6++]
    mov     a0l, [r6++]
    addv    0xfffc, r2
.Lws_pn_end:
    eint
    br      .Lws_done, always

.Lws_neg_pos:
    dint
    bkrep   r3, .Lws_np_end
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
    mov     a0h, [r6++]
    mov     a0l, [r6++]
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
    mov     a0h, [r6++]
    mov     a0l, [r6++]
    addv    0xfffc, r2
.Lws_np_end:
    eint
    br      .Lws_done, always

.Lws_flat_pos_neg:
    bkrep   r3, .Lws_fpn_end
    mov     [arrn0+ars0], a0        // +fwd
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    mov     a0h, [r6++]
    mov     a0l, [r6++]
    addv    0x0004, r0
    mov     [arrn2+ars0], a0        // -bwd
    neg     a0, always
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    mov     a0h, [r6++]
    mov     a0l, [r6++]
    addv    0xfffc, r2
.Lws_fpn_end:
    br      .Lws_done, always

.Lws_flat_neg_pos:
    bkrep   r3, .Lws_fnp_end
    mov     [arrn0+ars0], a0        // -fwd
    neg     a0, always
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    mov     a0h, [r6++]
    mov     a0l, [r6++]
    addv    0x0004, r0
    mov     [arrn2+ars0], a0        // +bwd
    add     b1, a0
    shfc    a0, a0, always
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    mov     a0h, [r6++]
    mov     a0l, [r6++]
    addv    0xfffc, r2
.Lws_fnp_end:

.Lws_done:
    rst     0x0800, mod0
    pop     cfgj
    pop     sv
    pop     y0
    pop     r6
    pop     r4
    pop     r3
    pop     r2
    pop     r0
    ret     always

// void aac_pcm_out(const u16 *job): t, overlap, pcm, count
// pcm[i] = clamp16((add_sat(overlap[i], t[i]) + 128) >> 8). A plain 32-bit
// limit suffices here: past +-2^31 the result clamps either way.
.global aac_pcm_out
aac_pcm_out:
    push    r0
    push    r2
    push    r3
    push    r6
    push    cfgi
    mov     a0l, r6
    mov     [r6++], r0              // t
    mov     [r6++], r2              // overlap
    mov     [r6++], r3              // pcm, held in r3 until the end of setup
    mov     [r6], a0                // count
    mov     r3, r6
    mov     a0l, r3
    modr    [r3--]
    mov     0x2, cfgi
    bkrep   r3, .Lpc_end
    mov     [arrn0+ars0], a0
    mov     [arrn2+ars0], b0
    add     b0, a0
    lim     a0, a0
    add     0x0080, a0
    lim     a0, a0
    shfi    a0, a0, +8
    lim     a0, a0
    mov     a0h, [r6++]
    modr    [r0++s]
    modr    [r2++s]
.Lpc_end:
    pop     cfgi
    pop     r6
    pop     r3
    pop     r2
    pop     r0
    ret     always

// void aac_add_sat(const u16 *job): acc, t, count
// acc[i] = add_sat(acc[i], t[i]), s32.
.global aac_add_sat
aac_add_sat:
    push    r0
    push    r2
    push    r3
    push    r6
    push    cfgi
    mov     a0l, r6
    mov     [r6++], r0              // acc
    mov     [r6++], r2              // t
    mov     [r6], r3                // count
    modr    [r3--]
    mov     0x2, cfgi
    bkrep   r3, .Las_end
    mov     [arrn0+ars0], a0
    mov     [arrn2+ars0], b0
    add     b0, a0
    lim     a0, a0
    neg     a0, always
    lim     a0, a0
    neg     a0, always
    mov     a0, [arrn0+ars0]
    modr    [r0++s]
    modr    [r2++s]
.Las_end:
    pop     cfgi
    pop     r6
    pop     r3
    pop     r2
    pop     r0
    ret     always
