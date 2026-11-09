// SPDX-License-Identifier: BSD-2-Clause
// CELT comb_filter front end for the Teak kernels.
typedef long s32;
typedef short s16;
typedef unsigned short u16;

void opus_comb_const(u16 *block);
void opus_comb_overlap(u16 *block);
void ogg_opus_poll(void);
void ogg_move_words(u16 *destination, const u16 *source, u16 words);

#define COMB_CHUNK 64

// no tables: task page hides foreground data
static s16 comb_gain(int tapset, int tap)
{
    if (tap == 2) return tapset ? 0 : 4248;
    if (tap == 1) return tapset == 0 ? 7112 : tapset == 1 ? 8784 : 3280;
    return tapset == 0 ? 10048 : tapset == 1 ? 15200 : 26208;
}

static s16 comb_taps(s16 g, s16 gain)
{
    return (s16)(((s32)g * gain + 16384) >> 15);
}

static u16 comb_word(const s32 *pointer)
{
    return (u16)(unsigned)pointer;
}

void comb_filter(s32 *y, s32 *x, int T0, int T1, int N, s16 g0, s16 g1,
                 int tapset0, int tapset1, const s16 *window, int overlap)
{
    u16 block[9];
    s16 gains[6];
    int i, count;
    if (g0 == 0 && g1 == 0) {
        if (x != y)
            ogg_move_words((u16 *)y, (const u16 *)x, (u16)(2 * N));
        return;
    }
    if (T0 < 15)
        T0 = 15;
    if (T1 < 15)
        T1 = 15;
    gains[0] = comb_taps(g1, comb_gain(tapset1, 2));
    gains[1] = comb_taps(g1, comb_gain(tapset1, 1));
    gains[2] = comb_taps(g1, comb_gain(tapset1, 0));
    gains[3] = comb_taps(g0, comb_gain(tapset0, 2));
    gains[4] = comb_taps(g0, comb_gain(tapset0, 1));
    gains[5] = comb_taps(g0, comb_gain(tapset0, 0));
    if (g0 == g1 && T0 == T1 && tapset0 == tapset1)
        overlap = 0;
    for (i = 0; i < overlap; i += count) {
        count = overlap - i < COMB_CHUNK ? overlap - i : COMB_CHUNK;
        block[0] = comb_word(y + i);
        block[1] = comb_word(x + i);
        block[2] = comb_word(x + i - T1) + 5;
        block[3] = comb_word(x + i - T1) - 4;
        block[4] = comb_word(x + i - T0) + 5;
        block[5] = comb_word(x + i - T0) - 4;
        block[6] = (u16)(unsigned)(window + i);
        block[7] = (u16)(unsigned)gains;
        block[8] = (u16)count;
        opus_comb_overlap(block);
        ogg_opus_poll();
    }
    if (g1 == 0) {
        if (x != y && N > overlap)
            ogg_move_words((u16 *)(y + overlap), (const u16 *)(x + overlap), (u16)(2 * (N - overlap)));
        return;
    }
    for (; i < N; i += count) {
        count = N - i < COMB_CHUNK ? N - i : COMB_CHUNK;
        block[0] = comb_word(y + i);
        block[1] = comb_word(x + i);
        block[2] = comb_word(x + i - T1) + 5;
        block[3] = comb_word(x + i - T1) - 4;
        block[4] = (u16)(unsigned)gains;
        block[5] = (u16)count;
        opus_comb_const(block);
        ogg_opus_poll();
    }
}
