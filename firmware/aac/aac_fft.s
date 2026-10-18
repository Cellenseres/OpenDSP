// SPDX-License-Identifier: BSD-2-Clause
// AAC IMDCT kernels, bit-exact with library/source/aac/filterbank.c.
// s32 values are stored high word first ([arrnX+ars0] order).
// mul_q15(b, w) = floor(b w / 2^15) = 2 bh w + floor(2 bl w / 2^16) with
// product shift 1: mpysu/macsu multiply the unsigned low word, maa adds the
// previous product >> 16, which is that floor.
.text

// frame: [r7+0] z, 1 twiddles, 2 chunk, 3 n, 4 size, 5 half, 6 a, 7 left in stage,
// 8 left in group, 9 stage twiddles, 10 butterflies until yield

// void aac_fft_stages(const u16 *job): z, bits, twiddles, chunk
// Radix-2 stages of size 4 .. 2^bits on z (complex s32: re, im). Twiddles in
// Y, per stage: size/2 pairs (cos, sin) Q15 starting at word size - 4.
// Interrupts are taken between chunks of at most 'chunk' butterflies.
//   t = b w, a' = (a + t) >> 1, b' = (a - t) >> 1
.global aac_fft_stages
aac_fft_stages:
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
    push    cfgi
    push    cfgj
    mov     a0l, r6
    addv    0xfff4, sp
    mov     sp, r7
    nop                             // [r7+k] sees the new r7 one instruction late
    mov     [r6++], a0
    mov     a0l, [r7+0x0000s7]      // z
    mov     [r6++], a0
    mov     a0l, sv                 // bits
    mov     0x1, a0
    shfc    a0, a0, always
    mov     a0l, [r7+0x0003s7]      // n
    mov     [r6++], a0
    mov     a0l, [r7+0x0001s7]      // twiddles
    mov     [r6], a0
    mov     a0l, [r7+0x0002s7]      // chunk
    mov     0x4, a0
    mov     a0l, [r7+0x0004s7]      // size
    mov     0x3, cfgi               // [r0++s]: b re high -> b im low
    mov     0x2, cfgj               // [r4++s]: next twiddle
    set     0x0800, mod0            // product shift 1
// Fixed early stages for the supported 64/512-point transforms.
.Lfs_fixed4:
    mov [r7+0x0003s7], a0
    shfi a0, a0, -2
    mov a0l, [r7+0x0007s7]
    mov [r7+0x0000s7], a0
    mov a0l, r2
    add 0x8, a0
    mov a0l, r5
    add 0x1, a0
    mov a0l, r0
    mov [r7+0x0001s7], a0
    add 0x0, a0
    mov a0l, r4
    mov 0x10, r6
.Lfs_fixed4_group:
    mov 0x1, r1
    dint
    bkrep r1, .Lfs_fixed4_end
    clr     a0, always
    clr     a1, always
    mpysu   [r4], [r0--], a0        // p = wr brl
    maa     [r4++], [r0++s], a0     // a0 = floor(wr brl), p = wr brh
    macsu   [r4], [r0--], a0        // a0 = mul_q15(br, wr), p = wi bil
    maa     [r4], [r0--], a1        // a1 = floor(wi bil), p = wi bih
    macsu   [r4], [r0--], a1        // a1 = mul_q15(bi, wi), p = wi brl
    sub     a1, a0                  // a0 = tr
    clr     a1, always
    maa     [r4--], [r0++s], a1     // a1 = floor(wi brl), p = wi brh
    macsu   [r4], [r0--], a1        // a1 += 2 wi brh, p = wr bil
    maa     [r4++s], [r0++s], a1    // a1 += floor(wr bil), p = wr bih
    add     p*, a1                  // a1 = ti
    mov     [arrn2+ars0], b0        // ar
    add     b0, a0                  // a0 = ar + tr
    shfi    b0, b0, +1
    sub     a0, b0                  // b0 = ar - tr
    shfi    a0, a0, -1
    shfi    b0, b0, -1
    mov     a0, [arrn2+ars0]
    mov     b0, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
    mov     [arrn2+ars0], b1        // ai
    add     b1, a1                  // a1 = ai + ti
    shfi    b1, b1, +1
    sub     a1, b1                  // b1 = ai - ti
    shfi    a1, a1, -1
    shfi    b1, b1, -1
    mov     a1, [arrn2+ars0]
    mov     b1, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
.Lfs_fixed4_end:
    eint
    addv 0x8, r2
    addv 0x8, r5
    addv 0x8, r0
    addv 0xfffc, r4
    modr [r6--]
    mov r6, a0
    cmpv 0x0, a0l
    br .Lfs_fixed4_continue, neq
    call aac_yield, always
    mov 0x10, r6
.Lfs_fixed4_continue:
    mov [r7+0x0007s7], a0
    sub 0x1, a0
    mov a0l, [r7+0x0007s7]
    br .Lfs_fixed4_group, neq
.Lfs_fixed8:
    mov [r7+0x0003s7], a0
    shfi a0, a0, -3
    mov a0l, [r7+0x0007s7]
    mov [r7+0x0000s7], a0
    mov a0l, r2
    add 0x10, a0
    mov a0l, r5
    add 0x1, a0
    mov a0l, r0
    mov [r7+0x0001s7], a0
    add 0x4, a0
    mov a0l, r4
    mov 0x8, r6
.Lfs_fixed8_group:
    mov 0x3, r1
    dint
    bkrep r1, .Lfs_fixed8_end
    clr     a0, always
    clr     a1, always
    mpysu   [r4], [r0--], a0        // p = wr brl
    maa     [r4++], [r0++s], a0     // a0 = floor(wr brl), p = wr brh
    macsu   [r4], [r0--], a0        // a0 = mul_q15(br, wr), p = wi bil
    maa     [r4], [r0--], a1        // a1 = floor(wi bil), p = wi bih
    macsu   [r4], [r0--], a1        // a1 = mul_q15(bi, wi), p = wi brl
    sub     a1, a0                  // a0 = tr
    clr     a1, always
    maa     [r4--], [r0++s], a1     // a1 = floor(wi brl), p = wi brh
    macsu   [r4], [r0--], a1        // a1 += 2 wi brh, p = wr bil
    maa     [r4++s], [r0++s], a1    // a1 += floor(wr bil), p = wr bih
    add     p*, a1                  // a1 = ti
    mov     [arrn2+ars0], b0        // ar
    add     b0, a0                  // a0 = ar + tr
    shfi    b0, b0, +1
    sub     a0, b0                  // b0 = ar - tr
    shfi    a0, a0, -1
    shfi    b0, b0, -1
    mov     a0, [arrn2+ars0]
    mov     b0, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
    mov     [arrn2+ars0], b1        // ai
    add     b1, a1                  // a1 = ai + ti
    shfi    b1, b1, +1
    sub     a1, b1                  // b1 = ai - ti
    shfi    a1, a1, -1
    shfi    b1, b1, -1
    mov     a1, [arrn2+ars0]
    mov     b1, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
.Lfs_fixed8_end:
    eint
    addv 0x10, r2
    addv 0x10, r5
    addv 0x10, r0
    addv 0xfff8, r4
    modr [r6--]
    mov r6, a0
    cmpv 0x0, a0l
    br .Lfs_fixed8_continue, neq
    call aac_yield, always
    mov 0x8, r6
.Lfs_fixed8_continue:
    mov [r7+0x0007s7], a0
    sub 0x1, a0
    mov a0l, [r7+0x0007s7]
    br .Lfs_fixed8_group, neq
.Lfs_fixed16:
    mov [r7+0x0003s7], a0
    shfi a0, a0, -4
    mov a0l, [r7+0x0007s7]
    mov [r7+0x0000s7], a0
    mov a0l, r2
    add 0x20, a0
    mov a0l, r5
    add 0x1, a0
    mov a0l, r0
    mov [r7+0x0001s7], a0
    add 0xc, a0
    mov a0l, r4
    mov 0x4, r6
.Lfs_fixed16_group:
    mov 0x7, r1
    dint
    bkrep r1, .Lfs_fixed16_end
    clr     a0, always
    clr     a1, always
    mpysu   [r4], [r0--], a0        // p = wr brl
    maa     [r4++], [r0++s], a0     // a0 = floor(wr brl), p = wr brh
    macsu   [r4], [r0--], a0        // a0 = mul_q15(br, wr), p = wi bil
    maa     [r4], [r0--], a1        // a1 = floor(wi bil), p = wi bih
    macsu   [r4], [r0--], a1        // a1 = mul_q15(bi, wi), p = wi brl
    sub     a1, a0                  // a0 = tr
    clr     a1, always
    maa     [r4--], [r0++s], a1     // a1 = floor(wi brl), p = wi brh
    macsu   [r4], [r0--], a1        // a1 += 2 wi brh, p = wr bil
    maa     [r4++s], [r0++s], a1    // a1 += floor(wr bil), p = wr bih
    add     p*, a1                  // a1 = ti
    mov     [arrn2+ars0], b0        // ar
    add     b0, a0                  // a0 = ar + tr
    shfi    b0, b0, +1
    sub     a0, b0                  // b0 = ar - tr
    shfi    a0, a0, -1
    shfi    b0, b0, -1
    mov     a0, [arrn2+ars0]
    mov     b0, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
    mov     [arrn2+ars0], b1        // ai
    add     b1, a1                  // a1 = ai + ti
    shfi    b1, b1, +1
    sub     a1, b1                  // b1 = ai - ti
    shfi    a1, a1, -1
    shfi    b1, b1, -1
    mov     a1, [arrn2+ars0]
    mov     b1, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
.Lfs_fixed16_end:
    eint
    addv 0x20, r2
    addv 0x20, r5
    addv 0x20, r0
    addv 0xfff0, r4
    modr [r6--]
    mov r6, a0
    cmpv 0x0, a0l
    br .Lfs_fixed16_continue, neq
    call aac_yield, always
    mov 0x4, r6
.Lfs_fixed16_continue:
    mov [r7+0x0007s7], a0
    sub 0x1, a0
    mov a0l, [r7+0x0007s7]
    br .Lfs_fixed16_group, neq
.Lfs_fixed32:
    mov [r7+0x0003s7], a0
    shfi a0, a0, -5
    mov a0l, [r7+0x0007s7]
    mov [r7+0x0000s7], a0
    mov a0l, r2
    add 0x40, a0
    mov a0l, r5
    add 0x1, a0
    mov a0l, r0
    mov [r7+0x0001s7], a0
    add 0x1c, a0
    mov a0l, r4
    mov 0x2, r6
.Lfs_fixed32_group:
    mov 0xf, r1
    dint
    bkrep r1, .Lfs_fixed32_end
    clr     a0, always
    clr     a1, always
    mpysu   [r4], [r0--], a0        // p = wr brl
    maa     [r4++], [r0++s], a0     // a0 = floor(wr brl), p = wr brh
    macsu   [r4], [r0--], a0        // a0 = mul_q15(br, wr), p = wi bil
    maa     [r4], [r0--], a1        // a1 = floor(wi bil), p = wi bih
    macsu   [r4], [r0--], a1        // a1 = mul_q15(bi, wi), p = wi brl
    sub     a1, a0                  // a0 = tr
    clr     a1, always
    maa     [r4--], [r0++s], a1     // a1 = floor(wi brl), p = wi brh
    macsu   [r4], [r0--], a1        // a1 += 2 wi brh, p = wr bil
    maa     [r4++s], [r0++s], a1    // a1 += floor(wr bil), p = wr bih
    add     p*, a1                  // a1 = ti
    mov     [arrn2+ars0], b0        // ar
    add     b0, a0                  // a0 = ar + tr
    shfi    b0, b0, +1
    sub     a0, b0                  // b0 = ar - tr
    shfi    a0, a0, -1
    shfi    b0, b0, -1
    mov     a0, [arrn2+ars0]
    mov     b0, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
    mov     [arrn2+ars0], b1        // ai
    add     b1, a1                  // a1 = ai + ti
    shfi    b1, b1, +1
    sub     a1, b1                  // b1 = ai - ti
    shfi    a1, a1, -1
    shfi    b1, b1, -1
    mov     a1, [arrn2+ars0]
    mov     b1, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
.Lfs_fixed32_end:
    eint
    addv 0x40, r2
    addv 0x40, r5
    addv 0x40, r0
    addv 0xffe0, r4
    modr [r6--]
    mov r6, a0
    cmpv 0x0, a0l
    br .Lfs_fixed32_continue, neq
    call aac_yield, always
    mov 0x2, r6
.Lfs_fixed32_continue:
    mov [r7+0x0007s7], a0
    sub 0x1, a0
    mov a0l, [r7+0x0007s7]
    br .Lfs_fixed32_group, neq
    mov 0x40, a0
    mov a0l, [r7+0x0004s7]
.Lfs_stage:
    mov     [r7+0x0002s7], a0
    mov     a0l, [r7+0x000as7]      // yield budget spans small FFT groups
    mov     [r7+0x0004s7], a0
    shfi    a0, a0, -1
    mov     a0l, [r7+0x0005s7]      // half
    mov     [r7+0x0004s7], a0
    sub     0x4, a0
    add     [r7+0x0001s7], a0
    mov     a0l, [r7+0x0009s7]      // stage twiddles
    mov     [r7+0x0000s7], a0
    mov     a0l, [r7+0x0006s7]      // a = z
    mov     [r7+0x0003s7], a0
    shfi    a0, a0, -1
    mov     a0l, [r7+0x0007s7]      // butterflies left in the stage
.Lfs_group:
    mov     [r7+0x0006s7], a0
    mov     a0l, r2                 // a, read and written
    mov     [r7+0x0005s7], a1
    shfi    a1, a1, +2
    add     a1, a0
    mov     a0l, r5                 // b, written
    add     0x1, a0
    mov     a0l, r0                 // b re low, read
    mov     [r7+0x0009s7], a0
    mov     a0l, r4                 // twiddle
    mov     [r7+0x0005s7], a0
    mov     a0l, [r7+0x0008s7]      // butterflies left in the group
.Lfs_chunk:
    mov     [r7+0x0008s7], a0
    mov     [r7+0x000as7], a1
    cmp     a1, a0
    br      .Lfs_fits, le
    mov     a1, a0                  // count = min(left, chunk)
.Lfs_fits:
    mov     a0l, r1
    mov     [r7+0x0008s7], a1
    sub     a0, a1
    mov     a1l, [r7+0x0008s7]
    mov     [r7+0x000as7], a1
    sub     a0, a1
    mov     a1l, [r7+0x000as7]
    modr    [r1--]
    dint
    bkrep   r1, .Lfs_bfly_end
    clr     a0, always
    clr     a1, always
    mpysu   [r4], [r0--], a0        // p = wr brl
    maa     [r4++], [r0++s], a0     // a0 = floor(wr brl), p = wr brh
    macsu   [r4], [r0--], a0        // a0 = mul_q15(br, wr), p = wi bil
    maa     [r4], [r0--], a1        // a1 = floor(wi bil), p = wi bih
    macsu   [r4], [r0--], a1        // a1 = mul_q15(bi, wi), p = wi brl
    sub     a1, a0                  // a0 = tr
    clr     a1, always
    maa     [r4--], [r0++s], a1     // a1 = floor(wi brl), p = wi brh
    macsu   [r4], [r0--], a1        // a1 += 2 wi brh, p = wr bil
    maa     [r4++s], [r0++s], a1    // a1 += floor(wr bil), p = wr bih
    add     p*, a1                  // a1 = ti
    mov     [arrn2+ars0], b0        // ar
    add     b0, a0                  // a0 = ar + tr
    shfi    b0, b0, +1
    sub     a0, b0                  // b0 = ar - tr
    shfi    a0, a0, -1
    shfi    b0, b0, -1
    mov     a0, [arrn2+ars0]
    mov     b0, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
    mov     [arrn2+ars0], b1        // ai
    add     b1, a1                  // a1 = ai + ti
    shfi    b1, b1, +1
    sub     a1, b1                  // b1 = ai - ti
    shfi    a1, a1, -1
    shfi    b1, b1, -1
    mov     a1, [arrn2+ars0]
    mov     b1, [arrn3+ars0]
    modr    [r2++]
    modr    [r2++]
    modr    [r5++]
    modr    [r5++]
.Lfs_bfly_end:
    eint
    mov     [r7+0x000as7], a0
    cmpv    0x0, a0l
    br      .Lfs_continue, neq
    call    aac_yield, always
    mov     [r7+0x0002s7], a0
    mov     a0l, [r7+0x000as7]
.Lfs_continue:
    nop
    nop
    mov     [r7+0x0008s7], a0
    cmpv    0x0, a0l
    br      .Lfs_chunk, neq
    mov     [r7+0x0004s7], a1       // next group: a += 4 size
    shfi    a1, a1, +2
    mov     [r7+0x0006s7], a0
    add     a1, a0
    mov     a0l, [r7+0x0006s7]
    mov     [r7+0x0007s7], a0
    sub     [r7+0x0005s7], a0
    mov     a0l, [r7+0x0007s7]
    cmpv    0x0, a0l
    br      .Lfs_group, neq
    mov     [r7+0x0004s7], a0       // next stage
    shfi    a0, a0, +1
    mov     a0l, [r7+0x0004s7]
    mov     [r7+0x0003s7], a1
    cmp     a1, a0
    br      .Lfs_stage, le
    rst     0x0800, mod0
    addv    0x000c, sp
    pop     cfgj
    pop     cfgi
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

// void aac_imdct_norm(u16 *job): x, count, result
// Peak normalization of imdct(): bits = OR |x[k]|, norm = clz32(bits) - 4,
// x[k] = shift_sat(x[k], norm) in place (left shift, or rounding right shift
// for norm < 0). result = norm, or 0x8000 if all zero. No multiplier, so
// interrupts stay on.
.global aac_imdct_norm
aac_imdct_norm:
    push    r2
    push    r3
    push    r5
    push    r6
    push    sv
    push    cfgi
    mov     a0l, r6                 // job
    mov     r6, r5
    mov     [r5++], r2              // x
    mov     [r5++], r3              // count; r5 -> result
    mov     0x2, cfgi
    modr    [r3--]
    clr     a1, always
    bkrep   r3, .Lnm_or_end
    mov     [arrn2+ars0], a0
    neg     a0, lt
    or      a0, a1
    modr    [r2++s]
.Lnm_or_end:
    mov     0x8000, a0
    mov     a1, b0
    br      .Lnm_store, eq          // silence
    exp     b0, a0                  // clz32(bits) - 1
    sub     0x3, a0                 // norm
    br      .Lnm_store, eq          // nothing to shift
    mov     a0l, sv
    clr     b1, always              // rounding term
    mov     a0, a1
    br      .Lnm_shift, gt
    add     0x4, a1                 // norm < 0: 8 >> (norm + 4) = 1 << (-norm - 1)
    neg     a1, always
    mov     a1l, sv
    mov     0x8, b1
    shfc    b1, b1, always
    mov     a0l, sv
.Lnm_shift:
    mov     [r6], r2                // x
    mov     r6, r3
    modr    [r3++]
    mov     [r3], r3                // count
    modr    [r3--]
    bkrep   r3, .Lnm_sh_end
    mov     [arrn2+ars0], a1
    add     b1, a1
    shfc    a1, a1, always
    mov     a1, [arrn2+ars0]
    modr    [r2++s]
.Lnm_sh_end:
.Lnm_store:
    mov     a0l, [r5]
    pop     cfgi
    pop     sv
    pop     r6
    pop     r5
    pop     r3
    pop     r2
    ret     always

// void aac_imdct_head(const u16 *job): x, n, twiddles, z, bitrev, step, chunk
// Pre-twiddle of imdct() fused with fft()'s bit reversal and size-2 stage:
//   Z(k) = (re + i im)(c + i s), re = x[2k], im = x[n/2 - 1 - 2k], (c, s) = tw[k]
//   z[2i] = (Z(k1) + Z(k2)) >> 1, z[2i+1] = (Z(k1) - Z(k2)) >> 1,
//   k1 = bitrev(2i), k2 = k1 + n/8, for i < n/8.
// x is normalized already (aac_imdct_norm). bitrev holds 4 bitrev9(2i),
// i < 256; step 1 for n = 2048, 8 for n = 256. Twiddles in Y.
// frame: [r7+0] x + 1, 1 x + n - 1, 2 n/2, 3 twiddles, 4 n/4, 5 table
// position, 6 table step, 7 iterations left, 8 chunk
.global aac_imdct_head
aac_imdct_head:
    push    r0
    push    r1
    push    r2
    push    r3
    push    r4
    push    r5
    push    r6
    push    r7
    push    y0
    mov     a0l, r6
    addv    0xfff7, sp
    mov     sp, r7
    nop
    mov     [r6++], a0              // x
    mov     [r6++], b0              // n
    add     0x1, a0
    mov     a0l, [r7+0x0000s7]
    add     b0, a0
    sub     0x2, a0
    mov     a0l, [r7+0x0001s7]
    mov     b0, a0
    shfi    a0, a0, -1
    mov     a0l, [r7+0x0002s7]
    shfi    a0, a0, -1
    mov     a0l, [r7+0x0004s7]
    shfi    a0, a0, -1
    mov     a0l, [r7+0x0007s7]
    mov     [r6++], a0
    mov     a0l, [r7+0x0003s7]
    mov     [r6++], a1              // z, the output pointer
    mov     [r6++], a0
    mov     a0l, [r7+0x0005s7]
    mov     [r6++], a0
    mov     a0l, [r7+0x0006s7]
    mov     [r6], a0
    mov     a0l, [r7+0x0008s7]
    mov     a1l, r6
    set     0x0800, mod0            // product shift 1
.Lhd_chunk:
    mov     [r7+0x0007s7], a0
    mov     [r7+0x0008s7], a1
    cmp     a1, a0
    br      .Lhd_fits, le
    mov     a1, a0                  // count = min(left, chunk)
.Lhd_fits:
    mov     [r7+0x0007s7], a1
    sub     a0, a1
    mov     a1l, [r7+0x0007s7]
    sub     0x1, a0
    dint
    bkrep   a0l, .Lhd_end
    mov     [r7+0x0005s7], a0       // t = 4 k1
    mov     a0l, r1
    mov     [r1], b0
    add     [r7+0x0006s7], a0
    mov     a0l, [r7+0x0005s7]
    mov     b0, a0
    add     [r7+0x0000s7], a0
    mov     a0l, r0                 // re(k1) low
    add     [r7+0x0002s7], a0
    mov     a0l, r2                 // re(k2) low
    mov     [r7+0x0001s7], a0
    sub     b0, a0
    mov     a0l, r1                 // im(k1) low
    sub     [r7+0x0002s7], a0
    mov     a0l, r3                 // im(k2) low
    mov     b0, a0
    shfi    a0, a0, -1
    add     [r7+0x0003s7], a0
    mov     a0l, r4                 // tw(k1)
    add     [r7+0x0004s7], a0
    mov     a0l, r5                 // tw(k2)
    clr     a0, always
    clr     a1, always
    mpysu   [r4], [r0--], a0        // c rel
    maa     [r4++], [r0++], a0      // c reh
    macsu   [r4], [r1--], a0        // s iml
    maa     [r4], [r1++], a1        // s imh
    macsu   [r4], [r0--], a1        // s rel
    sub     a1, a0                  // re Z(k1)
    clr     a1, always
    maa     [r4--], [r0], a1        // s reh
    macsu   [r4], [r1--], a1        // c iml
    maa     [r4], [r1], a1          // c imh
    add     p*, a1                  // im Z(k1)
    mov     a0, b0
    mov     a1, b1
    clr     a0, always
    clr     a1, always
    mpysu   [r5], [r2--], a0
    maa     [r5++], [r2++], a0
    macsu   [r5], [r3--], a0
    maa     [r5], [r3++], a1
    macsu   [r5], [r2--], a1
    sub     a1, a0                  // re Z(k2)
    clr     a1, always
    maa     [r5--], [r2], a1
    macsu   [r5], [r3--], a1
    maa     [r5], [r3], a1
    add     p*, a1                  // im Z(k2)
    add     b0, a0
    shfi    b0, b0, +1
    sub     a0, b0
    shfi    a0, a0, -1
    shfi    b0, b0, -1
    add     b1, a1
    shfi    b1, b1, +1
    sub     a1, b1
    shfi    a1, a1, -1
    shfi    b1, b1, -1
    mov     a0h, [r6++]             // z[2i]
    mov     a0l, [r6++]
    mov     a1h, [r6++]
    mov     a1l, [r6++]
    mov     b0h, [r6++]             // z[2i + 1]
    mov     b0l, [r6++]
    mov     b1h, [r6++]
    mov     b1l, [r6++]
.Lhd_end:
    eint
    call    aac_yield, always
    nop
    nop
    mov     [r7+0x0007s7], a0
    cmpv    0x0, a0l
    br      .Lhd_chunk, neq
    rst     0x0800, mod0
    addv    0x0009, sp
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

// void aac_imdct_post(const u16 *job): z, count, twiddles, chunk
// Post-twiddle of imdct(): z[k] = z[k] (c + i s), (c, s) = tw[k], in place.
.global aac_imdct_post
aac_imdct_post:
    push    r0
    push    r1
    push    r2
    push    r3
    push    r4
    push    r6
    push    y0
    push    cfgi
    push    cfgj
    mov     a0l, r6
    mov     [r6++], r2              // z, written
    mov     r2, r0
    modr    [r0++]                  // re low, read
    mov     [r6++], r3              // points left
    mov     [r6++], r4              // twiddles
    mov     [r6], r6                // chunk
    mov     0x3, cfgi
    mov     0x2, cfgj
    set     0x0800, mod0
.Lpo_chunk:
    mov     r3, a0
    mov     r6, a1
    cmp     a1, a0
    br      .Lpo_fits, le
    mov     a1, a0
.Lpo_fits:
    mov     r3, a1
    sub     a0, a1
    mov     a1l, r3
    sub     0x1, a0
    dint
    bkrep   a0l, .Lpo_end
    clr     a0, always
    clr     a1, always
    mpysu   [r4], [r0--], a0        // c rel
    maa     [r4++], [r0++s], a0     // c reh
    macsu   [r4], [r0--], a0        // s iml
    maa     [r4], [r0--], a1        // s imh
    macsu   [r4], [r0--], a1        // s rel
    sub     a1, a0
    clr     a1, always
    maa     [r4--], [r0++s], a1     // s reh
    macsu   [r4], [r0--], a1        // c iml
    maa     [r4++s], [r0++s], a1    // c imh; next point
    add     p*, a1
    mov     a0h, [r2++]
    mov     a0l, [r2++]
    mov     a1h, [r2++]
    mov     a1l, [r2++]
.Lpo_end:
    eint
    call    aac_yield, always
    nop
    nop
    mov     r3, a0
    cmpv    0x0, a0l
    br      .Lpo_chunk, neq
    rst     0x0800, mod0
    pop     cfgj
    pop     cfgi
    pop     y0
    pop     r6
    pop     r4
    pop     r3
    pop     r2
    pop     r1
    pop     r0
    ret     always
