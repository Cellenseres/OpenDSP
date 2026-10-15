// SPDX-License-Identifier: BSD-2-Clause
// Inner loops: resampling, mixing, filters, sample decoding.
// Loops using the multiplier run with interrupts off.
.text

// void resample(const resample_job_t *job)
.global resample
resample:
    push    r6
    push    r0
    push    r1
    push    r2
    push    r3
    push    r4
    push    r5
    push    y0
    mov     a0l, r2
    mov     [arrn2+ars0], a1        // step (32 bit)
    mov     a1, b1
    modr    [r2++]
    modr    [r2++]
    mov     [r2++], r3              // win
    modr    [r2++]
    mov     [r2++], r1              // out
    modr    [r2++]
    mov     [r2++], r4              // stereo
    mov     [r2++], r5              // interp
    clr     a0, always
    or      [r2++], a0              // frac (zero-extended)
    mov     [r2++], r0              // n
    cmpv    0x0, r5
    br      .Lpoly, eq
    mov     r0, r2
    modr    [r2--]                  // loop count
    mov     0xf, sv                 // movs shift
    cmpv    0x0, r4
    br      .Lstereo, neq
    // mono: b0 = window << 16 | frac
    mov     r3, a1
    shfi    a1, a1, +16
    add     a1, a0
    mov     a0, b0
    cmpv    0x2, r5
    br      .Lmono_none, eq
    dint
    bkrep   r2, .Lml_end
    mov     b0, a1
    shfi    a1, a1, -1
    and     0x7fff, a1
    mov     a1l, y0                 // frac >> 1
    mov     b0h, r0
    movs    [r0], a0                // a0 = s[i] << 15
    mpy     y0, [r0++], a1          // p = frac * s[i]
    msu     [r0], a0
    add     p*, a0
    shfi    a0, a0, +1
    mov     a0h, [r1++]
    mov     a0h, [r1++]
    add     b1, b0
.Lml_end:
    eint
    br      .Ldone, always
.Lmono_none:
    bkrep   r2, .Lmn_end
    mov     b0h, r0
    mov     [r0], a0l
    mov     a0l, [r1++]
    mov     a0l, [r1++]
    add     b1, b0
.Lmn_end:
    br      .Ldone, always
.Lstereo:
    // stereo: b0 = i << 16 | frac
    mov     a0, b0
    cmpv    0x2, r5
    br      .Lstereo_none, eq
    dint
    bkrep   r2, .Lsl_end
    mov     b0, a1
    shfi    a1, a1, -1
    and     0x7fff, a1
    mov     a1l, y0
    mov     b0h, a1
    shfi    a1, a1, +1
    add     r3, a1
    mov     a1l, r0                 // r0 = &L[i]
    movs    [r0], a0
    mpy     y0, [r0], a1
    modr    [r0++]
    modr    [r0++]
    msu     [r0], a0
    add     p*, a0
    shfi    a0, a0, +1
    mov     a0h, [r1++]
    modr    [r0--]                  // r0 = &R[i]
    movs    [r0], a0
    mpy     y0, [r0], a1
    modr    [r0++]
    modr    [r0++]
    msu     [r0], a0
    add     p*, a0
    shfi    a0, a0, +1
    mov     a0h, [r1++]
    add     b1, b0
.Lsl_end:
    eint
    br      .Ldone, always
.Lstereo_none:
    bkrep   r2, .Lsn_end
    mov     b0h, a1
    shfi    a1, a1, +1
    add     r3, a1
    mov     a1l, r0
    mov     [r0++], a0l
    mov     a0l, [r1++]
    mov     [r0], a0l
    mov     a0l, [r1++]
    add     b1, b0
.Lsn_end:
    br      .Ldone, always

// polyphase: 4 taps, s[i-2] .. s[i+1]
.Lpoly:
    mov     [r2], r5                // tab
    mov     r0, r2
    modr    [r2--]
    cmpv    0x0, r4
    br      .Lpoly_st, neq
    mov     r3, a1                  // b0 = window - 2, frac
    sub     0x2, a1
    shfi    a1, a1, +16
    add     a1, a0
    mov     a0, b0
    dint
    bkrep   r2, .Lpm_end
    mov     b0, a1
    shfi    a1, a1, -7
    and     0x1fc, a1
    add     r5, a1
    mov     a1l, r4
    mov     b0h, r0
    clr     a0, always
    add     0x2000, a0
    mpy     [r4++], [r0++], a1
    mac     [r4++], [r0++], a0
    mac     [r4++], [r0++], a0
    mac     [r4++], [r0++], a0
    add     p*, a0
    shfi    a0, a0, +2
    lim     a0, a0
    mov     a0h, [r1++]
    mov     a0h, [r1++]
    add     b1, b0
.Lpm_end:
    eint
    br      .Ldone, always
.Lpoly_st:
    dint
    push    cfgi
    mov     0x2, cfgi               // step 2: interleaved L/R
    mov     r3, a1
    sub     0x4, a1
    mov     a1l, r3                 // r3 = &L[-2]
    mov     a0, b0
    dint
    bkrep   r2, .Lps_end
    mov     b0, a1
    shfi    a1, a1, -7
    and     0x1fc, a1
    add     r5, a1
    mov     a1l, r4
    mov     a1l, r6
    mov     b0h, a1
    shfi    a1, a1, +1
    add     r3, a1
    mov     a1l, r0                 // &L[i-2]
    add     0x1, a1
    mov     a1l, r2                 // &R[i-2], r2 free in loop
    clr     a0, always
    add     0x2000, a0
    mpy     [r4++], [r0++s], a1
    mac     [r4++], [r0++s], a0
    mac     [r4++], [r0++s], a0
    mac     [r4++], [r0++s], a0
    add     p*, a0
    shfi    a0, a0, +2
    lim     a0, a0
    mov     a0h, [r1++]
    mov     r6, r4
    mov     r2, r0
    clr     a0, always
    add     0x2000, a0
    mpy     [r4++], [r0++s], a1
    mac     [r4++], [r0++s], a0
    mac     [r4++], [r0++s], a0
    mac     [r4++], [r0++s], a0
    add     p*, a0
    shfi    a0, a0, +2
    lim     a0, a0
    mov     a0h, [r1++]
    add     b1, b0
.Lps_end:
    pop     cfgi
    eint
.Ldone:
    pop     y0
    pop     r5
    pop     r4
    pop     r3
    pop     r2
    pop     r1
    pop     r0
    pop     r6
    ret     always

// void mix_add(const mix_job_t *job)
.global mix_add
mix_add:
    push    r0
    push    r1
    push    r2
    push    r3
    push    y0
    mov     a0l, r0
    mov     [r0++], r2              // acc
    modr    [r0++]
    mov     [r0++], r1              // src
    modr    [r0++]
    mov     [r0++], b0l             // gl
    mov     [r0++], b1l             // gr
    mov     [r0], r3                // n
    cmpv    0x0, r3
    br      .Lmx_done, eq
    modr    [r3--]
    dint
    bkrep   r3, .Lmx_end
    mov     b0l, y0
    mpy     y0, [r1++], a1
    mov     [arrn2+ars0], a0
    add     p*, a0
    mov     a0, [arrn2+ars0]
    modr    [r2++]
    modr    [r2++]
    mov     b1l, y0
    mpy     y0, [r1++], a1
    mov     [arrn2+ars0], a0
    add     p*, a0
    mov     a0, [arrn2+ars0]
    modr    [r2++]
    modr    [r2++]
.Lmx_end:
    eint
.Lmx_done:
    pop     y0
    pop     r3
    pop     r2
    pop     r1
    pop     r0
    ret     always

// void filter_simple(const filter_job_t *job), Q15
.global filter_simple
filter_simple:
    push    r0
    push    r1
    push    r3
    push    r4
    push    y0
    mov     a0l, r3
    mov     [r3++], r1              // buf
    modr    [r3++]
    mov     [r3++], r0              // state
    modr    [r3++]
    mov     [r3], r4                // coef
    mov     0x9f, r3
    dint
    bkrep   r3, .Lfs_end
    mov     [r1], a1l
    mov     a1l, [r0]               // x0 = x
    clr     a0, always
    mpy     [r4++], [r0++], a1
    mac     [r4--], [r0], a0
    add     p*, a0
    shfi    a0, a0, +1
    lim     a0, a0
    mov     a0h, [r0]               // y1 = y
    modr    [r0--]
    mov     a0h, [r1++]
    modr    [r1++]
.Lfs_end:
    eint
    pop     y0
    pop     r4
    pop     r3
    pop     r1
    pop     r0
    ret     always

// void filter_biquad(const filter_job_t *job), Q14
.global filter_biquad
filter_biquad:
    push    r0
    push    r1
    push    r3
    push    r4
    push    r5
    push    y0
    mov     a0l, r3
    mov     [r3++], r1              // buf
    modr    [r3++]
    mov     [r3++], r0              // state
    modr    [r3++]
    mov     [r3], r5                // coef base
    mov     0x9f, r3
    dint
    bkrep   r3, .Lfb_end
    mov     r5, r4
    mov     [r1], a1l
    mov     a1l, [r0]               // x0 = x
    clr     a0, always
    mpy     [r4++], [r0++], a1
    mac     [r4++], [r0++], a0
    mac     [r4++], [r0++], a0
    mac     [r4++], [r0++], a0
    mac     [r4++], [r0], a0        // r0 -> y2
    add     p*, a0
    shfi    a0, a0, +2
    lim     a0, a0
    modr    [r0--]                  // r0 -> y1
    mov     [r0++], a1l             // a1l = y1, r0 -> y2
    mov     a1l, [r0--]             // y2 = y1, r0 -> y1
    mov     a0h, [r0--]             // y1 = y, r0 -> x2
    modr    [r0--]                  // r0 -> x1
    mov     [r0++], a1l             // a1l = x1, r0 -> x2
    mov     a1l, [r0--]             // x2 = x1, r0 -> x1
    modr    [r0--]                  // r0 -> x0
    mov     [r0++], a1l             // a1l = x0, r0 -> x1
    mov     a1l, [r0--]             // x1 = x0, r0 -> x0
    mov     a0h, [r1++]
    modr    [r1++]
.Lfb_end:
    eint
    pop     y0
    pop     r5
    pop     r4
    pop     r3
    pop     r1
    pop     r0
    ret     always

// void pcm8_expand(const u16 *src, s16 *dst, u16 pairs)
.global pcm8_expand
pcm8_expand:
    push    r0
    push    r1
    push    r3
    mov     a0l, r0
    mov     a1l, r1
    mov     b0l, r3
    cmpv    0x0, r3
    br      .Lpx_done, eq
    modr    [r3--]
    bkrep   r3, .Lpx_end
    mov     [r0], a0
    shfi    a0, a0, +8
    mov     a0l, [r1++]
    mov     [r0++], a0
    and     0xff00, a0
    mov     a0l, [r1++]
.Lpx_end:
.Lpx_done:
    pop     r3
    pop     r1
    pop     r0
    ret     always

// void copy_words(const u16 *src, s16 *dst, u16 count)
.global copy_words
copy_words:
    push    r0
    push    r1
    push    r3
    mov     a0l, r0
    mov     a1l, r1
    mov     b0l, r3
    cmpv    0x0, r3
    br      .Lcw_done, eq
    modr    [r3--]
    bkrep   r3, .Lcw_end
    mov     [r0++], a0
    mov     a0l, [r1++]
.Lcw_end:
.Lcw_done:
    pop     r3
    pop     r1
    pop     r0
    ret     always

// void adpcm_run(const adpcm_job_t *job), history in dst[-2..-1]
.global adpcm_run
adpcm_run:
    push    r0
    push    r1
    push    r3
    push    r4
    push    y0                      // mpy loads y0
    mov     a0l, r3
    mov     [r3++], r4              // r4 -> c1, c2
    mov     [r3++], r1              // nibbles
    mov     [r3++], r0
    modr    [r0--]                  // r0 -> dst[-1]
    mov     [r3++], a1l             // n
    mov     [r3], sv
    mov     a1l, r3
    cmpv    0x0, r3
    br      .Lad_done, eq
    modr    [r3--]
    dint
    bkrep   r3, .Lad_end
    movs    [r1++], a0              // nibble << (scale + 11)
    add     0x400, a0
    mpy     [r4++], [r0--], a1      // p = c1 * y1
    mac     [r4--], [r0++], a0      // a0 += c1*y1; p = c2 * y2
    add     p*, a0
    shfi    a0, a0, +5
    lim     a0, a0
    modr    [r0++]
    mov     a0h, [r0]
.Lad_end:
    eint
.Lad_done:
    pop     y0
    pop     r4
    pop     r3
    pop     r1
    pop     r0
    ret     always

// void adpcm_unpack(const u16 *frame, u16 *nibbles)
.global adpcm_unpack
adpcm_unpack:
    push    r0
    push    r1
    push    r3
    mov     a0l, r0
    mov     a1l, r1
    mov     [r0], a0
    and     0xf000, a0
    mov     a0l, [r1++]
    mov     [r0++], a0
    shfi    a0, a0, +4
    and     0xf000, a0
    mov     a0l, [r1++]
    mov     0x2, r3
    bkrep   r3, .Lun_end
    mov     [r0], a0
    shfi    a0, a0, +8
    and     0xf000, a0
    mov     a0l, [r1++]
    mov     [r0], a0
    shfi    a0, a0, +12
    mov     a0l, [r1++]
    mov     [r0], a0
    and     0xf000, a0
    mov     a0l, [r1++]
    mov     [r0++], a0
    shfi    a0, a0, +4
    and     0xf000, a0
    mov     a0l, [r1++]
.Lun_end:
    pop     r3
    pop     r1
    pop     r0
    ret     always

// void adpcm_block(adpcm_block_job_t *job), whole frames
.global adpcm_block
adpcm_block:
    push    r0
    push    r1
    push    r2
    push    r3
    push    r4
    push    r5
    mov     a0l, r5
    mov     r5, r0
    mov     [r0++], r1              // raw
    modr    [r0++]
    mov     [r0++], r2              // dst
    mov     [r0++], r3              // nf
    mov     [r0], r4                // nibs
.Lbk_loop:
    cmpv    0x0, r3
    br      .Lbk_done, eq
    mov     r5, r0
    modr    [r0++]                  // -> coefbase
    mov     [r0], b0
    mov     [r1], a0
    and     0x0070, a0
    shfi    a0, a0, -3              // ((hdr >> 4) & 7) * 2
    add     b0, a0
    mov     r5, a1
    add     0x6, a1
    mov     a1l, r0                 // -> adreq
    mov     a0l, [r0++]             // coef
    modr    [r0++]                  // nib (preset)
    mov     r2, [r0++]              // dst
    modr    [r0++]                  // n (preset)
    mov     [r1], a0
    and     0x000f, a0
    sub     0x1, a0
    mov     a0l, [r0]               // sv = scale - 1
    mov     r1, a0
    mov     r4, a1
    call    adpcm_unpack, always
    mov     r5, a0
    add     0x6, a0
    call    adpcm_run, always
    mov     r1, a0
    add     0x4, a0
    mov     a0l, r1
    mov     r2, a0
    add     0xe, a0
    mov     a0l, r2
    modr    [r3--]
    br      .Lbk_loop, always
.Lbk_done:
    pop     r5
    pop     r4
    pop     r3
    pop     r2
    pop     r1
    pop     r0
    ret     always
