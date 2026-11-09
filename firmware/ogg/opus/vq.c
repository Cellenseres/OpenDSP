// SPDX-License-Identifier: BSD-3-Clause
// CELT inner product on a Teak asm loop.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "arch.h"
#include "vq.h"

typedef unsigned short u16;
void opus_inner_prod_shift(u16 *block);

// 64-bit sum from 32-bit parts, no i64 on native-cc
static opus_val32 inner_prod_wide(const celt_norm *x, const celt_norm *y, int len)
{
   int i;
   opus_uint32 shi = 0, slo = 0;
   for (i=0;i<len;i++)
   {
      opus_uint32 a = x[i] < 0 ? 0u - (opus_uint32)x[i] : (opus_uint32)x[i];
      opus_uint32 b = y[i] < 0 ? 0u - (opus_uint32)y[i] : (opus_uint32)y[i];
      opus_uint32 ah = a >> 16, al = a & 0xffff, bh = b >> 16, bl = b & 0xffff;
      opus_uint32 hi = ah * bh, lo = al * bl, m, t;
      m = ah * bl;
      t = lo + (m << 16);
      hi += (m >> 16) + (t < lo);
      lo = t;
      m = al * bh;
      t = lo + (m << 16);
      hi += (m >> 16) + (t < lo);
      lo = t;
      if ((x[i] < 0) != (y[i] < 0))
      {
         lo = ~lo + 1;
         hi = ~hi + (lo == 0);
      }
      t = slo + lo;
      shi += hi + (t < slo);
      slo = t;
   }
   return (opus_val32)(shi << 12 | slo >> 20);
}

static u16 vq_word(const void *pointer)
{
    return (u16)(unsigned)pointer;
}

opus_val32 celt_inner_prod_norm_shift(const celt_norm *x, const celt_norm *y, int len, int arch)
{
   u16 block[8];
   int n1;
   (void)arch;
   // low-word chunks must stay below 2^39
   if (len <= 0 || len > 226)
      return inner_prod_wide(x, y, len);
   n1 = len > 100 ? 100 : len;
   block[0] = vq_word(x);
   block[1] = vq_word(y);
   block[2] = (u16)len;
   block[3] = (u16)n1;
   block[4] = (u16)(len - n1);
   block[7] = 2;
   opus_inner_prod_shift(block);
   return (opus_val32)((opus_uint32)block[5] << 16 | block[6]);
}
