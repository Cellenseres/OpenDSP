// SPDX-License-Identifier: BSD-3-Clause
#include "decoder.h"
#include "mathops.h"

void ogg_opus_gain(s16 *pcm, u16 samples, s16 gain_q8)
{
    if (!gain_q8) return;
    opus_val32 gain = celt_exp2(MULT16_16_P15(QCONST16(6.48814081e-4f, 25), gain_q8));
    for (u16 index = 0; index < samples; ++index) {
        opus_val32 value = MULT16_32_P16(pcm[index], gain);
        pcm[index] = SATURATE(value, 32767);
        if (!(index & 127)) ogg_opus_pause();
    }
}
