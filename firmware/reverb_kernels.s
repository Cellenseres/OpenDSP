// SPDX-License-Identifier: BSD-2-Clause
// Reverb inner loops. Product shift 1, interrupts off: callers keep the
// spans short.
.text

// void reverb_diffuse(const u16 *job): buffer, delay line, count, gain
// allpass in place, w = x + g d, y = d - g w; reads one word past the line
.global reverb_diffuse
reverb_diffuse:
    push    r0
    push    r1
    push    r3
    push    y0
    push    sv
    mov     0x8000, sv              // rounding
    mov     a0l, r3
    mov     [r3++], r0              // samples, in place
    mov     [r3++], r1              // delay line
    mov     [r3++], a1              // count
    mov     [r3], y0                // gain
    cmpv    0x0, a1l
    br      .Ldf_done, eq
    sub     0x1, a1
    dint
    set     0x0800, mod0            // product shift 1
    mpy     y0, [r1], a0            // p = g d
    bkrep   a1l, .Ldf_end
    mov     [r0], a0h
    addl    sv, a0
    add     p*, a0
    lim     a0, a0                  // w
    mpy     y0, a0h, a1             // p = g w
    mov     [r1], a1h
    addl    sv, a1
    mov     a0h, [r1++]
    msu     [r1], a1                // y = d - g w, p = g d of the next sample
    lim     a1, a1
    mov     a1h, [r0++]
.Ldf_end:
    rst     0x0800, mod0
    eint
.Ldf_done:
    pop     sv
    pop     y0
    pop     r3
    pop     r1
    pop     r0
    ret     always

// void reverb_tank(const u16 *job): in, out, line 0, 2, 1, 3, count, table
// table: dither, damping, lp0, g0, lp2, g2, lp1, g1, lp3, g3, same, cross
// Rounding uses a fresh xorshift value per sample, so tails die out cleanly.
// Damping >= 0 keeps the lowpass and the L/R averages in range: no lim.
.global reverb_tank
reverb_tank:
    push    r0
    push    r1
    push    r2
    push    r3
    push    r4
    push    r5
    push    r6
    push    r7
    push    y0
    push    sv
    mov     a0l, r6
    mov     [r6++], r4              // input
    mov     [r6++], r5              // output, L R pairs
    mov     [r6++], r0
    mov     [r6++], r2
    mov     [r6++], r1
    mov     [r6++], r3
    mov     [r6++], a1              // count
    mov     [r6], r7                // table
    cmpv    0x0, a1l
    br      .Ltk_done, eq
    sub     0x1, a1
    push    cfgj
    dint
    set     0x0800, mod0            // product shift 1
    mov     0x2, cfgj               // [r6++s] skips the gain next to a lowpass state
    bkrep   a1l, .Ltk_end
    // dither: x ^= x << 7, x ^= x >> 9, x ^= x << 8
    mov     r7, r6
    mov     [r6], a0l
    shfi    a0, b0, +7
    xor     b0l, a0
    shfi    a0, b0, -9
    xor     b0l, a0
    shfi    a0, b0, +8
    xor     b0l, a0
    mov     a0l, [r6++]
    mov     a0l, sv
    mov     [r6++], y0              // damping
    clr     a0, always              // sum of the new lowpass states << 16

    // lowpass: lp = out + d (lp - out), lines 0, 2, 1, 3
    mov     [r0], b0h
    mov     [r6], a1h
    sub     b0, a1
    lim     a1, a1
    mpy     y0, a1h, a1
    mov     b0, a1
    addl    sv, a1
    add     p*, a1
    mov     a1h, [r6++s]
    addh    a1h, a0
    mov     [r2], b0h
    mov     [r6], a1h
    sub     b0, a1
    lim     a1, a1
    mpy     y0, a1h, a1
    mov     b0, a1
    addl    sv, a1
    add     p*, a1
    mov     a1h, [r6++s]
    addh    a1h, a0
    mov     [r1], b0h
    mov     [r6], a1h
    sub     b0, a1
    lim     a1, a1
    mpy     y0, a1h, a1
    mov     b0, a1
    addl    sv, a1
    add     p*, a1
    mov     a1h, [r6++s]
    addh    a1h, a0
    mov     [r3], b0h
    mov     [r6], a1h
    sub     b0, a1
    lim     a1, a1
    mpy     y0, a1h, a1
    mov     b0, a1
    addl    sv, a1
    add     p*, a1
    mov     a1h, [r6++s]
    addh    a1h, a0

    // feedback, table walked backwards: line = x + g (lp - sum / 2)
    shfi    a0, b1, -1              // sum << 15
    mov     [r4++], a1h
    addl    sv, a1
    mov     a1, b0                  // x << 16 + dither
    modr    [r6--]
    mov     [r6--], y0              // g3
    mov     [r6--], a0h             // lp3
    sub     b1, a0
    addl    sv, a0
    lim     a0, a0
    mpy     y0, a0h, a0
    mov     b0, a1
    mov     [r6--], y0              // g1
    mov     [r6--], a0h             // lp1
    sub     b1, a0
    addl    sv, a0
    lim     a0, a0
    mac     y0, a0h, a1
    lim     a1, a1
    mov     a1h, [r3++]
    mov     b0, a1
    mov     [r6--], y0              // g2
    mov     [r6--], a0h             // lp2
    sub     b1, a0
    addl    sv, a0
    lim     a0, a0
    mac     y0, a0h, a1
    lim     a1, a1
    mov     a1h, [r1++]
    mov     b0, a1
    mov     [r6--], y0              // g0
    mov     [r6--], a0h             // lp0
    sub     b1, a0
    addl    sv, a0
    lim     a0, a0
    mac     y0, a0h, a1
    lim     a1, a1
    mov     a1h, [r2++]
    mov     b0, a1
    add     p*, a1
    lim     a1, a1
    mov     a1h, [r0++]

    // left = (lp0 + lp2) / 2, right = (lp1 + lp3) / 2
    modr    [r6++]
    mov     [r6++s], a0h
    addh    [r6++s], a0
    mov     [r6++s], a1h
    addh    [r6++s], a1
    shfi    a0, a0, -1
    addl    sv, a0
    shfi    a1, a1, -1
    addl    sv, a1
    mov     a0, b0
    mov     a1, b1

    // width: L = same left + cross right, R = same right + cross left
    mov     sv, a0l
    mov     sv, a1l
    mov     [r6++], y0              // same
    mpy     y0, b0h, a0             // p = same left
    mac     y0, b1h, a0             // L += p, p = same right
    mov     [r6], y0                // cross
    mac     y0, b1h, a1             // R += p, p = cross right
    mac     y0, b0h, a0             // L += p, p = cross left
    add     p*, a1                  // R += p
    lim     a0, a0
    lim     a1, a1
    mov     a0h, [r5++]
    mov     a1h, [r5++]
.Ltk_end:
    pop     cfgj
    rst     0x0800, mod0
    eint
.Ltk_done:
    pop     sv
    pop     y0
    pop     r7
    pop     r6
    pop     r5
    pop     r4
    pop     r3
    pop     r2
    pop     r1
    pop     r0
    ret     always

// void reverb_input(const u16 *job): front L, front R, rear L, rear R, mono, gain
// mono = gain * (FL + FR + RL + RR) / 2, 80 s32 samples at stride 2
// gain >= 0, so the product fits 32 bits
.global reverb_input
reverb_input:
    push    r0
    push    r1
    push    r2
    push    r3
    push    r4
    push    y0
    push    sv
    mov     0x8000, sv              // rounding
    mov     a0l, r4
    mov     [r4++], r0
    mov     [r4++], r1
    mov     [r4++], r2
    mov     [r4++], r3
    mov     [r4++], a1              // mono
    mov     [r4], y0                // gain
    mov     a1l, r4
    mov     0x4f, a1
    push    cfgi
    dint
    set     0x0800, mod0            // product shift 1
    mov     0x3, cfgi               // [rN++s]: from the low word to the next sample
    bkrep   a1l, .Lin_end
    mov     [r0++], a0h
    addl    [r0++s], a0
    addh    [r1++], a0
    addl    [r1++s], a0
    addh    [r2++], a0
    addl    [r2++s], a0
    addh    [r3++], a0
    addl    [r3++s], a0
    shfi    a0, a0, +15             // sum / 2
    addl    sv, a0
    lim     a0, a0
    mpy     y0, a0h, a1
    mov     sv, a0l
    add     p*, a0
    mov     a0h, [r4++]
.Lin_end:
    pop     cfgi
    rst     0x0800, mod0
    eint
    pop     sv
    pop     y0
    pop     r4
    pop     r3
    pop     r2
    pop     r1
    pop     r0
    ret     always

// void reverb_output(const u16 *job): L R pairs, front L, front R, rear L, rear R
// wet to the front, rear cleared; interrupts stay on (C code never uses ++s)
.global reverb_output
reverb_output:
    push    r0
    push    r1
    push    r2
    push    r3
    push    r4
    mov     a0l, r4
    modr    [r4++]
    mov     [r4++], r0
    mov     [r4++], r1
    mov     [r4++], r2
    mov     [r4], r3
    mov     a0l, r4
    mov     [r4], r4                // stereo pairs
    push    cfgi
    mov     0x3, cfgi               // [rN++s]: from the low word to the next sample
    clr     a1, always
    mov     0x9f, b0
    bkrep   b0l, .Lout_end
    mov     [r4++], a0
    mov     a0h, [r0++]
    mov     a0l, [r0++s]
    mov     [r4++], a0
    mov     a0h, [r1++]
    mov     a0l, [r1++s]
    mov     a1l, [r2++]
    mov     a1l, [r2++s]
    mov     a1l, [r3++]
    mov     a1l, [r3++s]
.Lout_end:
    pop     cfgi
    pop     r4
    pop     r3
    pop     r2
    pop     r1
    pop     r0
    ret     always

// u16 reverb_or(const u16 *job): buffer, count > 0; returns the OR of all words
.global reverb_or
reverb_or:
    push    r0
    mov     a0l, r0
    mov     [r0++], b1              // buffer
    mov     [r0], a1                // count
    sub     0x1, a1
    mov     b1l, r0
    clr     a0, always
    bkrep   a1l, .Lor_end
    or      [r0++], a0
    nop                             // the hardware needs two instructions per block
.Lor_end:
    pop     r0
    ret     always
