// SPDX-License-Identifier: BSD-2-Clause
// AAC filterbank on the DSP, bit-exact with library/source/aac/filterbank.c.
#include "aac_fb.h"

void aac_imdct_norm(u16 *job);
void aac_imdct_head(const u16 *job);
void aac_fft_stages(const u16 *job);
void aac_imdct_post(const u16 *job);
void aac_window_emit(const u16 *job);
void aac_pcm_out(const u16 *job);
void aac_add_sat(const u16 *job);
void clear_words(void *dst, u16 words);
void copy_words(const u16 *src, s16 *dst, u16 count);

#define CHUNK 32                // verified below the 2500-instruction IRQ bound
#define PAIRS 32                // window pairs per span

aac_fb_t aac_fb;

static u16 job[10];
static u16 fz, fn, fshift;      // current transform: z, n, shift
static u16 pcm_target, pcm_overlap;
static u16 wstep, wvariant;     // window direction (1 or 0xFFFF), flat bit

// imdct() of x into fz; 0 when silent
static u16 imdct(u16 x, u16 n)
{
    u16 longw = n == 2048, tw = longw ? aac_fb.imdct_long : aac_fb.imdct_short;
    s16 norm, shift;
    job[0] = x;
    job[1] = n >> 1;
    aac_imdct_norm(job);
    if (job[2] == 0x8000)
        return 0;
    norm = (s16)job[2];
    shift = 4 - norm;
    fshift = (u16)shift;
    job[0] = x;
    job[1] = n;
    job[2] = tw;
    job[3] = fz;
    job[4] = aac_fb.bitrev;
    job[5] = longw ? 1 : 8;
    job[6] = CHUNK;
    aac_yield();
    aac_imdct_head(job);
    job[0] = fz;
    job[1] = longw ? 9 : 6;
    job[2] = aac_fb.stage_tw;
    job[3] = CHUNK;
    aac_yield();
    aac_fft_stages(job);
    job[0] = fz;
    job[1] = n >> 2;
    job[2] = tw;
    job[3] = CHUNK;
    aac_yield();
    aac_imdct_post(job);
    return 1;
}

// Keep this setup out of window_range's compiler-limited stack frame.
__attribute__((noinline)) static void emit_window(u16 out)
{
    job[6] = 0;
    if (pcm_target) {
        job[6] = pcm_overlap + out - aac_fb.t;
        job[9] = pcm_target + ((out - aac_fb.t) >> 1);
    }
    aac_window_emit(job);
}

static void window_range(u16 y, u16 end, u16 win, u16 out)
{
    u16 n4 = fn >> 2, h = fn >> 3, qbits = fn == 2048 ? 9 : 6;
    while (y < end) {
        u16 q = y >> qbits, base = q << qbits;
        u16 stop = base + n4 < end ? base + n4 : end;
        u16 k0 = (y - base) >> 1, pairs = (stop - y) >> 1;
        u16 fwd, bwd, variant = 0;
        if (q == 0) {
            fwd = fz + ((h + k0) << 2);
            bwd = fz + ((h - 1 - k0) << 2) + 2;
        } else if (q == 1) {
            fwd = fz + (k0 << 2) + 2;
            bwd = fz + ((n4 - 1 - k0) << 2);
        } else if (q == 2) {
            fwd = fz + ((h + k0) << 2) + 2;
            bwd = fz + ((h - 1 - k0) << 2);
        } else {
            fwd = fz + (k0 << 2);
            bwd = fz + ((n4 - 1 - k0) << 2) + 2;
            variant = 1;
        }
        while (pairs) {
            u16 c = pairs > PAIRS ? PAIRS : pairs;
            job[0] = fwd;
            job[1] = bwd;
            job[2] = win;
            job[3] = wstep;
            job[4] = c;
            job[5] = fshift;
            job[7] = out;
            job[8] = variant | wvariant;
            aac_yield();
            emit_window(out);
            fwd += c << 2;
            bwd -= c << 2;
            if (wstep == 1)
                win += c << 1;
            else
                win -= c << 1;
            out += c << 2;
            pairs -= c;
        }
        y = stop;
    }
}

static void pcm_out(u16 t, u16 overlap, u16 pcm, u16 count)
{
    job[0] = t;
    job[1] = overlap;
    job[2] = pcm;
    job[3] = count;
    aac_pcm_out(job);
}

static void short_blocks(u16 x, u16 overlap, u16 pcm, u16 mode)
{
    u16 shape = (mode >> 2) & 1, prev = (mode >> 3) & 1, w, t = aac_fb.t;
    u16 sc = shape ? aac_fb.kbd_short : aac_fb.sine_short;
    u16 sp = prev ? aac_fb.kbd_short : aac_fb.sine_short;
    clear_words((void *)(unsigned)t, 4096);
    fn = 256;
    for (w = 0; w < 8; w++) {
        if (!imdct(x + (w << 8), 256))
            continue;
        wvariant = 0;
        wstep = 1;
        window_range(0, 128, w ? sc : sp, aac_fb.tmp);
        wstep = 0xFFFF;
        window_range(128, 256, sc + 127, aac_fb.tmp + 256);
        job[0] = t + ((448 + (w << 7)) << 1);
        job[1] = aac_fb.tmp;
        job[2] = 256;
        aac_add_sat(job);
    }
    pcm_out(t, overlap, pcm, 1024);
    copy_words((const u16 *)(unsigned)(t + 2048), (s16 *)(unsigned)overlap, 2048);
}

void aac_fb_channel(u16 x, u16 overlap, u16 pcm, u16 mode)
{
    u16 seq = mode & 3, shape = (mode >> 2) & 1, prev = (mode >> 3) & 1, t = aac_fb.t;
    u16 lp = prev ? aac_fb.kbd_long : aac_fb.sine_long, lc = shape ? aac_fb.kbd_long : aac_fb.sine_long;
    u16 sp = prev ? aac_fb.kbd_short : aac_fb.sine_short, sc = shape ? aac_fb.kbd_short : aac_fb.sine_short;
    pcm_target = pcm_overlap = 0;
    fz = aac_fb.z;
    if (seq == 2) {
        short_blocks(x, overlap, pcm, mode);
        return;
    }
    fn = 2048;
    if (!imdct(x, 2048)) {
        clear_words((void *)(unsigned)t, 2048);
        pcm_out(t, overlap, pcm, 1024);
        clear_words((void *)(unsigned)overlap, 2048);
        return;
    }
    // Long windows combine windowing and old overlap directly into PCM.
    pcm_target = pcm;
    pcm_overlap = overlap;
    wstep = 1;
    wvariant = 0;
    if (seq == 3) {
        clear_words((void *)(unsigned)t, 896);
        pcm_out(t, overlap, pcm, 448);
        window_range(448, 576, sp, t + 896);
        wvariant = 2;
        window_range(576, 1024, 0, t + 1152);
    } else {
        window_range(0, 1024, lp, t);
    }
    pcm_target = pcm_overlap = 0;
    // second half is the new overlap
    wstep = 0xFFFF;
    if (seq == 1) {
        wvariant = 2;
        window_range(1024, 1472, 0, overlap);
        wvariant = 0;
        window_range(1472, 1600, sc + 127, overlap + 896);
        clear_words((void *)(unsigned)(overlap + 1152), 896);
    } else {
        wvariant = 0;
        window_range(1024, 2048, lc + 1023, overlap);
    }
}
