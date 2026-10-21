/* SPDX-License-Identifier: BSD-2-Clause */
static mp3_decoder *current;
static s32 mul30(s32 a, s32 b)
{
#ifdef __teak__
    return mp3_mul30(a, b);
#else
    return mp3_product(a, b, 30);
#endif
}
static __attribute__((noinline)) void copy_bytes(void *to, const void *from, u16 bytes)
{
#ifdef __teak__
    void mp3_copy_words(const u16 *, u16 *, u16);
    mp3_copy_words((const u16 *)from, (u16 *)to, bytes / sizeof(u16));
#else
    u16 *p = (u16 *)to;
    const u16 *q = (const u16 *)from;
    u16 n = bytes / sizeof(u16);
    while (n--)
        *p++ = *q++;
#endif
}
static s32 shift_value(s32 x, s16 shift)
{
    if (shift >= 0)
        return shift > 30 ? 0 : (s32)((u32)x << shift);
    return shift < -31 ? (x < 0 ? -1 : 0) : x >> -shift;
}
static __attribute__((noinline)) s16 bits32(s32 value)
{
    u32 x = value < 0 ? (u32)(-value) : (u32)value;
    s16 n = 0;
    if (x >= 65536UL) {
        x >>= 16;
        n += 16;
    }
    if (x >= 256) {
        x >>= 8;
        n += 8;
    }
    if (x >= 16) {
        x >>= 4;
        n += 4;
    }
    if (x >= 4) {
        x >>= 2;
        n += 2;
    }
    if (x >= 2) {
        x >>= 1;
        ++n;
    }
    return n + (x != 0);
}
static s16 maxbits(const void *data, u16 n)
{
#ifdef __teak__
    return mp3_maxbits(data, n);
#else
    const s32 *p = data;
    s16 bits = 0, b;
    while (n--) {
        b = bits32(*p++);
        if (b > bits)
            bits = b;
    }
    return bits;
#endif
}
static void rescale(void *data, u16 n, s16 shift)
{
#ifdef __teak__
    mp3_rescale(data, n, shift);
#else
    s32 *p = data;
    if (shift)
        while (n--) {
            *p = shift_value(*p, shift);
            ++p;
        }
#endif
}

/* Mantissa in Q20, with an explicit extra binary exponent. The quartic
 * approximation has |fraction| <= 1/32; no 8207-entry inverse-power table. */
static s32 power43(u16 q, s16 *exponent)
{
    s32 f, p;
    u16 x = q, sign, index;
    *exponent = 0;
    if (x < 129)
        return mp3_pow43[x];
    *exponent = 8;
    if (x < 1024) {
        x <<= 3;
        *exponent = 4;
    }
    sign = (2 * x) & 64;
    index = (x + sign) >> 6;
    f = mp3_product(((s32)(x & 63) - sign) * 65536L, mp3_reciprocal[index], 16);
    p = 22093454L;                 /* 5/243 */
    p = -53024288L + mul30(f, p);  /* -4/81 */
    p = 238609294L + mul30(f, p);  /* 2/9 */
    p = 1431655765L + mul30(f, p); /* 4/3 */
    p = 1073741824L + mul30(f, p);
    return mul30(mp3_pow43[index], p);
}
static s16 band_gain(mp3_state *s, mp3_granule *g, u16 ch, u16 band)
{
    return (s16)g->gain - 214 - (s->mode ? 2 : 0) - (scale_row(s->effective[0], ch)[band] << (g->scale + 1));
}

static __attribute__((noinline)) void requantize(mp3_decoder *d, mp3_granule *gr)
{
    static const u16 power_bound[15] = {0, 2, 3, 4, 6, 7, 8, 10, 11, 12, 14, 15, 16, 18, 19};
    mp3_state *s = d->s;
    u16 ch, band, i, at;
    s16 bound = -100, gain, b, e, base_shift;
    MP3_PHASE(d, 2);
    /* A conservative exponent bound is computed on integer Huffman values,
     * before materializing any potentially huge requantized spectrum. */
    for (ch = 0; ch < s->channels; ++ch) {
        mp3_granule *g = granule_at(gr, ch);
        s32 *row = d->spectrum + mp3_index(ch, 576);
        at = 0;
        for (band = 0; band < g->long_bands + g->short_bands; ++band) {
            b = maxbits(row + at, g->bands[band]);
            gain = band_gain(s, g, ch, band);
            if (b) {
                e = power_bound[b] + (gain >> 2) + ((gain & 3) != 0);
                if (e > bound)
                    bound = e;
            }
            at += g->bands[band];
        }
    }
    s->exponent = bound - 20;
    for (ch = 0; ch < s->channels; ++ch) {
        mp3_granule *g = granule_at(gr, ch);
        s32 *row = d->spectrum + mp3_index(ch, 576);
        at = 0;
        for (band = 0; band < g->long_bands + g->short_bands; ++band) {
            gain = band_gain(s, g, ch, band);
            base_shift = (gain >> 2) + 1 - 20 - s->exponent;
            for (i = 0; i < g->bands[band];) {
#ifdef __teak__
                u16 limit = g->bands[band] - i, n;
                if (limit > 64 - (at & 63))
                    limit = 64 - (at & 63);
                /* Reuse the stage-disjoint eight-word synthesis job. Stop
                 * before a large value so the polynomial path stays exact. */
                s->window_job[0] = (u16)(unsigned)(row + at);
                s->window_job[1] = limit;
                s->window_job[2] = base_shift;
                s->window_job[3] = gain & 3;
                s->window_job[4] = (u16)(unsigned)mp3_pow43;
                s->window_job[5] = (u16)(unsigned)(mp3_quarter + (gain & 3));
                n = mp3_requant_small(s->window_job);
                i += n;
                at += n;
                if (n && !(at & 63))
                    MP3_YIELD(d);
                if (i == g->bands[band])
                    break;
                if (n == limit)
                    continue;
#endif
                s32 x = row[at], value;
                /* Huffman zero runs need no inverse power, product or shift.
                 * Retain a checkpoint at every 64 spectral values. */
                if (x) {
                    u16 magnitude = (u16)(x < 0 ? -x : x);
                    if (magnitude < 129) {
                        value = mp3_pow43[magnitude];
                        e = 0;
                    } else {
                        value = power43(magnitude, &s->power_exp);
                        e = s->power_exp;
                    }
                    if (gain & 3)
                        value = mul30(value, mp3_quarter[gain & 3]);
                    else
                        value = (value >> 1) + (value & 1); /* Q30 coefficient 0.5 */
                    value = shift_value(value, e + base_shift);
                    row[at] = x < 0 ? -value : value;
                }
                ++i;
                ++at;
                if (!(at & 63))
                    MP3_YIELD(d);
            }
        }
    }
    if (s->mode)
        for (i = 0; i < 576; ++i) {
            s32 l = d->spectrum[i], r = d->spectrum[576 + i];
            d->spectrum[i] = l + r;
            d->spectrum[576 + i] = l - r;
        }
    MP3_YIELD(d);
}

static __attribute__((noinline)) s16 mp3d_scale_pcm(s32 sample)
{
    s16 shift = current->s->exponent + 17;
    s32 value;
    if (shift >= 0) {
        if (shift > 15)
            return sample ? (sample < 0 ? -32768 : 32767) : 0;
        if (sample > (32767L >> shift))
            return 32767;
        if (sample < (-32768L >> shift))
            return -32768;
        value = shift_value(sample, shift);
    } else {
        s16 right = -shift;
        if (right > 31)
            return 0;
        value = sample >> right;
        if (((u32)sample >> (right - 1)) & 1)
            ++value;
    }
    if (value > 32767)
        return 32767;
    if (value < -32768L)
        return -32768;
    return (s16)value;
}
#define SHORT_BLOCK_TYPE 2
#define STOP_BLOCK_TYPE 3
#ifdef __teak__
#define mp3d_scale_pcm(value) mp3_scale_pcm(value, current->s->exponent + 17)
#endif
#include "hybrid.inc"

static __attribute__((noinline)) void hybrid(mp3_decoder *d, mp3_granule *gr)
{
    static const s32 cs[8] = {920726018L,  946763264L,  1019651018L, 1055811010L,
                              1068910418L, 1072820496L, 1073633590L, 1073734480L};
    static const s32 ca[8] = {552435611L, 506518839L, 336486475L, 195327790L,
                              101548264L, 43986469L,  15245688L,  3972820L};
    mp3_state *s = d->s;
    u16 ch, i, band, long_count;
    s16 exponent = s->exponent > s->overlap_exp ? s->exponent : s->overlap_exp;
    current = d;
    MP3_PHASE(d, 3);
    rescale(d->spectrum, mp3_index(s->channels, 576), s->exponent - exponent);
    for (ch = 0; ch < s->channels; ++ch)
        rescale(d->overlap[ch], 288, s->overlap_exp - exponent);
    s->exponent = exponent;
    for (ch = 0; ch < s->channels; ++ch) {
        mp3_granule *g = granule_at(gr, ch);
        s32 *x = d->spectrum + mp3_index(ch, 576);
        long_count = g->short_bands ? (g->long_bands ? 2 : 0) : 32;
        if (g->short_bands) {
            u16 base = mp3_index(long_count, 18), out = base, len;
            const u16 *width = g->bands + g->long_bands;
            while ((len = *width) != 0) {
                for (i = 0; i < len; ++i) {
                    d->work[out++] = x[base + i];
                    d->work[out++] = x[base + len + i];
                    d->work[out++] = x[base + 2 * len + i];
                }
                base += mp3_index(len, 3);
                width += 3;
            }
            for (i = mp3_index(long_count, 18); i < 576; ++i)
                x[i] = d->work[i];
        }
        for (band = 1; band < long_count; ++band) {
#ifdef __teak__
            mp3_alias(x + mp3_index(band, 18) - 8, s->math);
#else
            for (i = 0; i < 8; ++i) {
                s32 u = x[band * 18 + i], v = x[band * 18 - 1 - i];
                x[band * 18 + i] = mul30(u, cs[i]) - mul30(v, ca[i]);
                x[band * 18 - 1 - i] = mul30(u, ca[i]) + mul30(v, cs[i]);
            }
#endif
            if (!(band & 3) || band + 1 == long_count)
                MP3_YIELD(d);
        }
#if defined(__teak__) && !defined(MP3_C_KERNELS)
        for (band = 0; band < 32; ++band) {
            /* Zero spectrum and overlap stay zero for every window type. */
            if (mp3_hybrid_empty(x + mp3_index(band, 18), d->overlap[ch] + mp3_index(band, 18))) {
                if ((band & 3) == 3)
                    MP3_YIELD(d);
                continue;
            }
            if (g->block == 2 && band >= long_count)
                mp3_imdct_short(x + mp3_index(band, 18), d->overlap[ch] + mp3_index(band, 18), 0, s->math);
            else
                mp3_imdct_long(
                    x + mp3_index(band, 18), d->overlap[ch] + mp3_index(band, 18),
                    g_mdct_window[0] + mp3_index(band < (g->mixed ? 2 : 0) ? 0 : g->block == 3, 18), s->math);
            if ((band & 3) == 3)
                MP3_YIELD(d);
        }
#else
        L3_imdct_gr(x, (s32 *)d->overlap[ch], g->block, g->mixed ? 2 : 0);
#endif
        L3_change_sign(x);
        MP3_YIELD(d);
    }
    s->overlap_exp = s->exponent;
    /* Rescale history separately to retain quiet tails. */
    {
        s16 bits = maxbits(d->overlap[0], 288), b = maxbits(d->overlap[1], 288), sh;
        if (b > bits)
            bits = b;
        sh = 25 - bits;
        if (!bits)
            s->overlap_exp = -80;
        else if (sh > 0) {
            rescale(d->overlap[0], 288, sh);
            rescale(d->overlap[1], 288, sh);
            s->overlap_exp -= sh;
        }
    }
}

/* Lee DCT32 with Q31 secants. */
static __attribute__((noinline)) void dct32(mp3_decoder *d, s32 *src)
{
    s32 *x = d->s->math, *tmp = x + 32;
    u16 n, base, i, half;
    for (i = 0; i < 32; ++i)
        x[i] = src[18 * i];
    for (n = 32; n > 1; n >>= 1) {
        half = n >> 1;
        for (base = 0; base < 32; base += n) {
            for (i = 0; i < half; ++i) {
                tmp[base + i] = x[base + i] + x[base + n - 1 - i];
                tmp[base + half + i] = mp3_product(x[base + i] - x[base + n - 1 - i],
                                                   mp3_secants[half - 1 + i], mp3_secant_shift[half - 1 + i]);
            }
        }
        for (i = 0; i < 32; ++i)
            x[i] = tmp[i];
    }
    for (n = 2; n <= 32; n <<= 1) {
        half = n >> 1;
        for (base = 0; base < 32; base += n)
            for (i = 0; i < half; ++i) {
                tmp[base + 2 * i] = x[base + i];
                tmp[base + 2 * i + 1] = x[base + half + i] + (i + 1 < half ? x[base + half + i + 1] : 0);
            }
        for (i = 0; i < 32; ++i)
            x[i] = tmp[i];
    }
    for (i = 0; i < 32; ++i)
        src[18 * i] = x[i];
}
static __attribute__((noinline)) void synthesis(mp3_decoder *d)
{
    mp3_state *s = d->s;
    u16 i, ch;
    u16 *pcm = MP3_PCM(d);
    u16 count = mp3_index(s->channels, 576);
    s16 sh = 23 - maxbits(d->spectrum, count), exponent;
    MP3_PHASE(d, 4);
    rescale(d->spectrum, count, sh);
    s->exponent -= sh;
    for (ch = 0; ch < s->channels; ++ch)
        for (i = 0; i < 18; ++i) {
#if defined(__teak__) && !defined(MP3_C_KERNELS)
            mp3_dct32(d->spectrum + mp3_index(ch, 576) + i, s->math);
#else
            dct32(d, d->spectrum + mp3_index(ch, 576) + i);
#endif
            if ((i & 3) == 3 || i == 17)
                MP3_YIELD(d);
        }
    exponent = s->exponent > s->qmf_exp ? s->exponent : s->qmf_exp;
    rescale(d->spectrum, count, s->exponent - exponent);
    rescale(s->qmf, 960, s->qmf_exp - exponent);
    s->exponent = exponent;
    MP3_PHASE(d, 5);
    copy_bytes(d->work, s->qmf, 960 * sizeof(s32));
    for (i = 0; i < 18; i += 2) {
        mp3d_synth(d->spectrum + i, (s16 *)pcm, s->channels, d->work + 64 * i);
        MP3_EMIT(d, pcm, 64);
        MP3_YIELD(d);
    }
    copy_bytes(s->qmf, d->work + 18 * 64, 960 * sizeof(s32));
    s->qmf_exp = s->exponent;
    sh = maxbits(s->qmf, 960);
    if (!sh)
        s->qmf_exp = -80;
    else if (sh < 25) {
        rescale(s->qmf, 960, 25 - sh);
        s->qmf_exp -= 25 - sh;
    }
}
