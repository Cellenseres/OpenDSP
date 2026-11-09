// SPDX-License-Identifier: BSD-3-Clause
// CELT deemphasis and band denormalisation on Teak asm loops.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "celt.h"
#include "modes.h"
#include "mathops.h"
#include "quant_bands.h"

typedef unsigned short u16;
void ogg_opus_poll(void);
// native compiler has no division
static int small_div(int n, int d)
{
   int q = 0;
   while ((q + 1) * d <= n)
      q++;
   return q;
}
void opus_deemph_run(u16 *block);
void opus_deemph_acc(u16 *block);
void opus_denorm_band(u16 *block);
void opus_k_denorm_bands(u16 *block);
void opus_k_zero(void *dst, opus_int16 words);
// IR table, one word per entry
#ifndef SYNTH_EMEANS
#define SYNTH_EMEANS opus_synth_emeans
#endif
extern const opus_int16 SYNTH_EMEANS[25];

static u16 synth_word(const void *pointer)
{
    return (u16)(unsigned)pointer;
}

// one channel, no downsampling
static celt_sig deemph_channel(const celt_sig *x, opus_res *y, int N, int C, opus_val16 coef0, celt_sig m, int accum)
{
    u16 block[7];
    block[3] = (u16)C;
    block[4] = (u16)coef0;
    block[5] = (u16)((opus_uint32)m >> 16);
    block[6] = (u16)m;
    // about 10k cycles per section
    while (N > 0) {
       int n = N > 320 ? 320 : N;
       block[0] = synth_word(x);
       block[1] = synth_word(y);
       block[2] = (u16)n;
       ogg_opus_poll();
       if (accum)
          opus_deemph_acc(block);
       else
          opus_deemph_run(block);
       x += n;
       y += n * C;
       N -= n;
    }
    return (celt_sig)((opus_uint32)block[5] << 16 | block[6]);
}

void deemphasis(celt_sig *in[], opus_res *pcm, int N, int C, int downsample, const opus_val16 *coef,
      celt_sig *mem, int accum)
{
   int c;
   opus_val16 coef0;
   if (downsample == 1)
   {
      for (c = 0; c < C; c++)
         mem[c] = deemph_channel(in[c], pcm + c, N, C, coef[0], mem[c], accum);
      return;
   }
   coef0 = coef[0];
   c=0; do {
      int j, k, phase;
      celt_sig * OPUS_RESTRICT x;
      opus_res  * OPUS_RESTRICT y;
      celt_sig m = mem[c];
      x =in[c];
      y = pcm+c;
      ogg_opus_poll();
      // keep every downsample-th sample, no scratch
      k = 0;
      phase = 0;
      for (j=0;j<N;j++)
      {
         celt_sig tmp = SATURATE(x[j] + VERY_SMALL + m, SIG_SAT);
         m = MULT16_32_Q15(coef0, tmp);
         if (phase == 0 && j + downsample <= N)
         {
            if (accum) {
               y[k*C] = ADD_RES(y[k*C], SIG2RES(tmp));
            } else {
               y[k*C] = SIG2RES(tmp);
            }
            k++;
         }
         if (++phase == downsample)
            phase = 0;
      }
      mem[c] = m;
   } while (++c<C);
}

void denormalise_bands(const CELTMode *m, const celt_norm * OPUS_RESTRICT X,
      celt_sig * OPUS_RESTRICT freq, const celt_glog *bandLogE, int start,
      int end, int M, int downsample, int silence)
{
   int i, N;
   int bound;
   celt_sig * OPUS_RESTRICT f;
   const celt_norm * OPUS_RESTRICT x;
   const opus_int16 *eBands = m->eBands;
   u16 block[10];
   N = M*m->shortMdctSize;
   bound = M*eBands[end];
   if (downsample!=1)
      bound = IMIN(bound, small_div(N, downsample));
   if (silence)
   {
      bound = 0;
      start = end = 0;
   }
   f = freq;
   x = X+M*eBands[start];
   if (start != 0)
   {
      for (i=0;i<M*eBands[start];i++)
         *f++ = 0;
   } else {
      f += M*eBands[start];
   }
   ogg_opus_poll();
   for (i=start;i<end;i++)
   {
      int j, band_end;
      opus_val32 g, round;
      {
         u16 fast[7];
         fast[0] = synth_word(&eBands[i]);
         fast[1] = synth_word(&SYNTH_EMEANS[i]);
         fast[2] = synth_word(&bandLogE[i]);
         fast[3] = synth_word(x);
         fast[4] = synth_word(f);
         fast[5] = (u16)M;
         fast[6] = (u16)(end-i);
         opus_k_denorm_bands(fast);
         i = end-(int)fast[6];
         x = X+M*eBands[i];
         f = freq+M*eBands[i];
         if (i >= end)
            break;
      }
      celt_glog lg;
      int shift;
      j=M*eBands[i];
      band_end = M*eBands[i+1];
      lg = ADD32(bandLogE[i], SHL32((opus_val32)SYNTH_EMEANS[i],DB_SHIFT-4));
      shift = 17-(lg>>DB_SHIFT);
      if (shift>=31)
      {
         shift=0;
         g=0;
      } else {
         g = SHL32(celt_exp2_db_frac((lg&((1<<DB_SHIFT)-1))), 2);
      }
      if (shift<0)
      {
         g = 2147483647;
         shift = 0;
      }
      round = shift > 0 ? (opus_val32)1 << (shift - 1) : 0;
      // libopus do-while runs at least once
      if (band_end <= j)
         band_end = j + 1;
      block[0] = synth_word(x);
      block[1] = synth_word(f);
      block[2] = (u16)(band_end - j);
      block[3] = (u16)((opus_uint32)g >> 16);
      block[4] = (u16)g;
      block[5] = (u16)-(8 + shift);
      block[6] = (u16)((opus_uint32)round >> 16);
      block[7] = (u16)round;
      block[8] = synth_word(&block[3]);
      block[9] = synth_word(&block[4]);
      opus_denorm_band(block);
      x += band_end - j;
      f += band_end - j;
   }
   celt_assert(start <= end);
   opus_k_zero(&freq[bound], (opus_int16)(2*(N-bound)));
}
