// SPDX-License-Identifier: BSD-3-Clause
// CELT inverse MDCT on Teak asm kernels.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define SKIP_CONFIG_H
#include "_kiss_fft_guts.h"
#include "kiss_fft.h"
#include "mdct.h"
#include "mathops.h"

typedef unsigned short u16;
void ogg_opus_poll(void);
void opus_fft_bfly4m1(u16 *block);
void opus_fft_bfly2(u16 *block);
void opus_fft_bfly4(u16 *block);
void opus_fft_bfly3(u16 *block);
void opus_fft_bfly5(u16 *block);
void opus_mdct_prescan(u16 *block);
void opus_mdct_pre(u16 *block);
void opus_mdct_post(u16 *block);
void opus_mdct_mirror(u16 *block);
void opus_fft_downshift(u16 *block);
void opus_fft_gather(u16 *block);

static u16 mdct_word(const void *pointer)
{
    return (u16)(unsigned)pointer;
}

// rounding added after the wrap shift
static opus_int32 mdct_round(int shift)
{
    return shift > 0 ? (opus_int32)1 << (shift + 7) : 0;
}

static void fft_downshift(kiss_fft_cpx *x, int N, int *total, int step)
{
    int shift = IMIN(step, *total);
    opus_int32 round;
    u16 block[5];
    *total -= shift;
    if (shift <= 0)
        return;
    // SHR32 for one bit, else PSHR32
    round = shift == 1 ? 0 : mdct_round(shift);
    block[0] = mdct_word(x);
    block[1] = (u16)(2 * N);
    block[2] = (u16)-(8 + shift);
    block[3] = (u16)(round >> 16);
    block[4] = (u16)round;
#ifdef C_DOWN
    { int i; for (i = 0; i < N; i++) {
        if (shift == 1) { x[i].r = SHR32(x[i].r, 1); x[i].i = SHR32(x[i].i, 1); }
        else { x[i].r = PSHR32(x[i].r, shift); x[i].i = PSHR32(x[i].i, shift); } } }
#else
    opus_fft_downshift(block);
#endif
}

// FIFO drains in 32.8k cycles.
// Limits are about 15k cycles.
static void fft_radix(int p, kiss_fft_cpx *fout, int m, int N, int m2, const opus_int16 *table)
{
    int limit = p == 5 ? 36 : p == 4 ? 64 : 48;
    int step = 2 * (p - 1);
    u16 block[6];
    while (N > 0) {
        int groups = 1;
        int j = 0;
        while (groups < N && (groups + 1) * m <= limit)
            groups++;
        do {
            int count = m <= limit ? m : IMIN(m - j, limit);
            block[0] = mdct_word(fout + j);
            block[1] = (u16)count;
            block[2] = (u16)groups;
            block[3] = (u16)(4 * m2);
            block[4] = (u16)(4 * m);
            block[5] = mdct_word(table + j * step);
            if (p == 4)
                opus_fft_bfly4(block);
            else if (p == 3)
                opus_fft_bfly3(block);
            else
                opus_fft_bfly5(block);
            j += count;
            ogg_opus_poll();
        } while (j < m);
        fout += groups * m2;
        N -= groups;
    }
}

static void fft_stages(const kiss_fft_state *st, kiss_fft_cpx *fout, int downshift)
{
    int m2, m, p, L, i, shift;
    int fstride[MAXFACTORS];
    opus_int16 table[768];
    u16 block[6];
    shift = st->shift > 0 ? st->shift : 0;
    fstride[0] = 1;
    L = 0;
    do {
        p = st->factors[2 * L];
        m = st->factors[2 * L + 1];
        fstride[L + 1] = fstride[L] * p;
        L++;
    } while (m != 1);
    m = st->factors[2 * L - 1];
    for (i = L - 1; i >= 0; i--) {
        m2 = i ? st->factors[2 * i - 1] : 1;
        p = st->factors[2 * i];
        ogg_opus_poll();
        fft_downshift(fout, st->nfft, &downshift, p == 2 ? 1 : p == 5 ? 3 : 2);
        block[0] = mdct_word(fout);
        if (p == 2 || (p == 4 && m == 1)) {
            block[1] = (u16)fstride[i];
            if (p == 2)
                opus_fft_bfly2(block);
            else
                opus_fft_bfly4m1(block);
        } else {
            // twiddles k*j*stride, k = 1..p-1, per j
            block[0] = mdct_word(st->twiddles);
            block[1] = (u16)m;
            block[2] = (u16)(p - 1);
            block[3] = (u16)(2 * (fstride[i] << shift));
            block[4] = mdct_word(table);
            block[5] = (u16)(2 * (p - 1) - 1);
#ifdef C_GATHER
            { int j, k; opus_int16 *q = table; for (j = 0; j < m; j++) for (k = 1; k < p; k++) {
                const kiss_twiddle_cpx *tt = st->twiddles + k * j * (fstride[i] << shift); *q++ = tt->r; *q++ = tt->i; } }
#else
            opus_fft_gather(block);
#endif
            fft_radix(p, fout, m, fstride[i], m2, table);
        }
        m = m2;
    }
    fft_downshift(fout, st->nfft, &downshift, downshift);
}

void clt_mdct_backward_c(const mdct_lookup *l, kiss_fft_scalar *in, kiss_fft_scalar * OPUS_RESTRICT out,
                         const celt_coef * OPUS_RESTRICT window, int overlap, int shift, int stride, int arch)
{
    int i;
    int N, N2, N4;
    const kiss_twiddle_scalar *trig;
    int pre_shift, post_shift, fft_shift;
    opus_val32 sumval, maxval;
    opus_int32 round;
    kiss_fft_scalar *yp = out + (overlap >> 1);
    u16 block[10];
    (void)arch;
    N = l->n;
    trig = l->trig;
    for (i = 0; i < shift; i++) {
        N >>= 1;
        trig += N;
    }
    N2 = N >> 1;
    N4 = N >> 2;
    ogg_opus_poll();
    block[0] = mdct_word(in);
    block[1] = (u16)N2;
    block[2] = (u16)((2 * stride - 1) & 0x7f);
    opus_mdct_prescan(block);
    maxval = (opus_val32)block[3] << 16 | block[4];
    sumval = N2 + ((opus_val32)block[5] << 16 | block[6]);
    if (block[3] == 0x8000) {
        // INT_MIN: ABS32 wraps, redo in C
        maxval = 0;
        for (i = 0; i < N2; i++)
            maxval = MAX32(maxval, ABS32(in[i * stride]));
    }
    pre_shift = IMAX(0, 29 - celt_zlog2(1 + maxval));
    post_shift = IMAX(0, 19 - celt_ilog2(ABS32(sumval)));
    post_shift = IMIN(post_shift, pre_shift);
    fft_shift = pre_shift - post_shift;
    ogg_opus_poll();
    block[0] = mdct_word(in);
    block[1] = mdct_word(in + stride * (N2 - 1));
    block[2] = mdct_word(trig);
    block[3] = mdct_word(trig + N4);
    block[4] = mdct_word(l->kfft[shift]->bitrev);
    block[5] = mdct_word(yp);
    block[6] = (u16)N4;
    block[7] = (u16)((4 * stride - 1) & 0x7f);
    block[8] = (u16)((-4 * stride - 1) & 0x7f);
    block[9] = (u16)pre_shift;
#ifdef C_PRE
    { const kiss_fft_scalar *xp1 = in, *xp2 = in + stride * (N2 - 1); const opus_int16 *bitrev = l->kfft[shift]->bitrev; const kiss_twiddle_scalar *t = trig;
      for (i = 0; i < N4; i++) { int rev = *bitrev++; opus_val32 x1 = SHL32_ovflw(*xp1, pre_shift), x2 = SHL32_ovflw(*xp2, pre_shift);
        yp[2 * rev + 1] = ADD32_ovflw(S_MUL(x2, t[i]), S_MUL(x1, t[N4 + i])); yp[2 * rev] = SUB32_ovflw(S_MUL(x1, t[i]), S_MUL(x2, t[N4 + i]));
        xp1 += 2 * stride; xp2 -= 2 * stride; } }
#else
    opus_mdct_pre(block);
#endif
    fft_stages(l->kfft[shift], (kiss_fft_cpx *)yp, fft_shift);
    ogg_opus_poll();
    // N4 is even for every CELT size
    round = mdct_round(post_shift);
    block[0] = mdct_word(yp);
    block[1] = mdct_word(yp + N2 - 2);
    block[2] = mdct_word(trig);
    block[3] = mdct_word(trig + N4);
    block[4] = mdct_word(trig + N4 - 1);
    block[5] = mdct_word(trig + N2 - 1);
    block[6] = (u16)((N4 + 1) >> 1);
    block[7] = (u16)-(8 + post_shift);
    block[8] = (u16)(round >> 16);
    block[9] = (u16)round;
#ifdef C_POST
    { kiss_fft_scalar *yp0 = yp, *yp1 = yp + N2 - 2; const kiss_twiddle_scalar *t = trig;
      for (i = 0; i < (N4 + 1) >> 1; i++) { kiss_fft_scalar re, im, yr, yi; kiss_twiddle_scalar t0, t1;
        re = yp0[1]; im = yp0[0]; t0 = t[i]; t1 = t[N4 + i];
        yr = PSHR32_ovflw(ADD32_ovflw(S_MUL(re, t0), S_MUL(im, t1)), post_shift);
        yi = PSHR32_ovflw(SUB32_ovflw(S_MUL(re, t1), S_MUL(im, t0)), post_shift);
        re = yp1[1]; im = yp1[0]; yp0[0] = yr; yp1[1] = yi; t0 = t[(N4 - i - 1)]; t1 = t[(N2 - i - 1)];
        yr = PSHR32_ovflw(ADD32_ovflw(S_MUL(re, t0), S_MUL(im, t1)), post_shift);
        yi = PSHR32_ovflw(SUB32_ovflw(S_MUL(re, t1), S_MUL(im, t0)), post_shift);
        yp1[0] = yr; yp0[1] = yi; yp0 += 2; yp1 -= 2; } }
#else
    opus_mdct_post(block);
#endif
    block[0] = mdct_word(out + overlap - 1);
    block[1] = mdct_word(out);
    block[2] = mdct_word(window);
    block[3] = mdct_word(window + overlap - 1);
    block[4] = (u16)(overlap / 2);
#ifdef C_MIRROR
    { kiss_fft_scalar *xp1 = out + overlap - 1, *yp1 = out; const celt_coef *wp1 = window, *wp2 = window + overlap - 1;
      for (i = 0; i < overlap / 2; i++) { kiss_fft_scalar x1 = *xp1, x2 = *yp1;
        *yp1++ = SUB32_ovflw(S_MUL(x2, *wp2), S_MUL(x1, *wp1)); *xp1-- = ADD32_ovflw(S_MUL(x2, *wp1), S_MUL(x1, *wp2)); wp1++; wp2--; } }
#else
    opus_mdct_mirror(block);
#endif
}
