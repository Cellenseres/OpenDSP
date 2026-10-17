// SPDX-License-Identifier: BSD-2-Clause
// IMDCT via N/4-point complex FFT, windowing, overlap-add.
#include <string.h>
#include "internal.h"

// in-place radix-2 FFT, scaled by 1/n
static void fft(int32_t *z, unsigned bits)
{
    unsigned n = 1u << bits, stride = 1u << (9 - bits), size, step, i, j;
    for (i = 0; i < n; i++) {
        j = aac_bitrev512[i * stride];
        if (j > i) {
            int32_t re = z[2 * i], im = z[2 * i + 1];
            z[2 * i] = z[2 * j];
            z[2 * i + 1] = z[2 * j + 1];
            z[2 * j] = re;
            z[2 * j + 1] = im;
        }
    }
    for (i = 0; i < n; i += 2) {
        int32_t ar = z[2 * i], ai = z[2 * i + 1], br = z[2 * i + 2], bi = z[2 * i + 3];
        z[2 * i] = (ar + br) >> 1;
        z[2 * i + 1] = (ai + bi) >> 1;
        z[2 * i + 2] = (ar - br) >> 1;
        z[2 * i + 3] = (ai - bi) >> 1;
    }
    for (size = 4, step = 128; size <= n; size <<= 1, step >>= 1) {
        unsigned half = size >> 1;
        for (j = 0; j < half; j++) {
            int16_t wr = aac_fft_twiddle[2 * j * step], wi = aac_fft_twiddle[2 * j * step + 1];
            for (i = j; i < n; i += size) {
                int32_t *a = z + 2 * i, *b = z + 2 * (i + half);
                int32_t tr = mul_q15(b[0], wr) - mul_q15(b[1], wi);
                int32_t ti = mul_q15(b[0], wi) + mul_q15(b[1], wr);
                int32_t ar = a[0], ai = a[1];
                a[0] = (ar + tr) >> 1;
                a[1] = (ai + ti) >> 1;
                b[0] = (ar - tr) >> 1;
                b[1] = (ai - ti) >> 1;
            }
        }
    }
}

// y = IMDCT(x) without 2/N; returns output shift
// INT32_MIN means silence
static int imdct(const int32_t *x, unsigned n, int32_t *z, int32_t *y)
{
    unsigned quarter = n / 4, half8 = n / 8, k;
    const int16_t *tw = n == 2048 ? aac_imdct_twiddle_long : aac_imdct_twiddle_short;
    uint32_t bits = 0;
    int norm;
    for (k = 0; k < n / 2; k++)
        bits |= (uint32_t)(x[k] < 0 ? -x[k] : x[k]);
    if (!bits)
        return INT32_MIN;
    // peak below 2^28 keeps FFT in range
    norm = (int)clz32(bits) - 4;
    for (k = 0; k < quarter; k++) {
        int32_t re = shift_sat(x[2 * k], norm), im = shift_sat(x[n / 2 - 1 - 2 * k], norm);
        int16_t c = tw[2 * k], s = tw[2 * k + 1];
        z[2 * k] = mul_q15(re, c) - mul_q15(im, s);
        z[2 * k + 1] = mul_q15(re, s) + mul_q15(im, c);
    }
    fft(z, n == 2048 ? 9 : 6);
    for (k = 0; k < quarter; k++) {
        int32_t re = z[2 * k], im = z[2 * k + 1];
        int16_t c = tw[2 * k], s = tw[2 * k + 1];
        z[2 * k] = mul_q15(re, c) - mul_q15(im, s);
        z[2 * k + 1] = mul_q15(re, s) + mul_q15(im, c);
    }
#define RE(i) z[2 * (i)]
#define IM(i) z[2 * (i) + 1]
    for (k = 0; k < half8; k++) {
        y[2 * k] = RE(half8 + k);
        y[2 * k + 1] = -IM(half8 - 1 - k);
        y[quarter + 2 * k] = IM(k);
        y[quarter + 2 * k + 1] = -RE(quarter - 1 - k);
        y[2 * quarter + 2 * k] = IM(half8 + k);
        y[2 * quarter + 2 * k + 1] = -RE(half8 - 1 - k);
        y[3 * quarter + 2 * k] = -RE(k);
        y[3 * quarter + 2 * k + 1] = IM(quarter - 1 - k);
    }
#undef RE
#undef IM
    // PCM = y >> (SPEC_FRAC+norm+1)
    return TIME_FRAC - SPEC_FRAC - norm - 1;
}

static void window_rising(int32_t *y, const int16_t *w, unsigned count, int shift)
{
    unsigned i;
    for (i = 0; i < count; i++)
        y[i] = shift_sat(mul_q15(y[i], w[i]), shift);
}

static void window_falling(int32_t *y, const int16_t *w, unsigned count, int shift)
{
    unsigned i;
    for (i = 0; i < count; i++)
        y[i] = shift_sat(mul_q15(y[i], w[count - 1 - i]), shift);
}

static void window_flat(int32_t *y, unsigned count, int shift)
{
    unsigned i;
    for (i = 0; i < count; i++)
        y[i] = shift_sat(y[i], shift);
}

void aac_filterbank(aac_decoder_t *d, unsigned ch, int16_t *pcm, unsigned stride)
{
    const aac_channel_t *c = &d->ch[ch];
    unsigned seq = c->ics.window_sequence, shape = c->ics.window_shape, prev = d->prev_shape[ch], i;
    const int16_t *long_prev = prev ? aac_kbd_long : aac_sine_long;
    const int16_t *long_cur = shape ? aac_kbd_long : aac_sine_long;
    const int16_t *short_prev = prev ? aac_kbd_short : aac_sine_short;
    const int16_t *short_cur = shape ? aac_kbd_short : aac_sine_short;
    int32_t *t = d->time, *overlap = d->overlap[ch];

    if (seq == EIGHT_SHORT_SEQUENCE) {
        int32_t *y = d->fft + 256;
        unsigned w;
        memset(t, 0, sizeof(d->time));
        for (w = 0; w < 8; w++) {
            int shift = imdct(c->spec + w * 128, 256, d->fft, y);
            int32_t *dst = t + 448 + w * 128;
            if (shift == INT32_MIN)
                continue;
            window_rising(y, w ? short_cur : short_prev, 128, shift);
            window_falling(y + 128, short_cur, 128, shift);
            for (i = 0; i < 256; i++)
                dst[i] = add_sat(dst[i], y[i]);
        }
    } else {
        int shift = imdct(c->spec, 2048, d->fft, t);
        if (shift == INT32_MIN) {
            memset(t, 0, sizeof(d->time));
        } else {
            if (seq == LONG_STOP_SEQUENCE) {
                memset(t, 0, 448 * sizeof(*t));
                window_rising(t + 448, short_prev, 128, shift);
                window_flat(t + 576, 448, shift);
            } else {
                window_rising(t, long_prev, 1024, shift);
            }
            if (seq == LONG_START_SEQUENCE) {
                window_flat(t + 1024, 448, shift);
                window_falling(t + 1472, short_cur, 128, shift);
                memset(t + 1600, 0, 448 * sizeof(*t));
            } else {
                window_falling(t + 1024, long_cur, 1024, shift);
            }
        }
    }

    for (i = 0; i < AAC_FRAME_SAMPLES; i++) {
        int32_t v = add_sat(overlap[i], t[i]);
        v = (int32_t)(((int64_t)v + (1 << (TIME_FRAC - 1))) >> TIME_FRAC);
        pcm[i * stride] = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
        overlap[i] = t[AAC_FRAME_SAMPLES + i];
    }
    d->prev_shape[ch] = (uint8_t)shape;
}
