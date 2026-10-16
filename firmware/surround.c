// SPDX-License-Identifier: BSD-2-Clause
// Surround output mode: rear channels folded into stereo.
#include "surround.h"
#include "voice.h"

#define F FRAME_SAMPLES

static u16 depth, position, rear_ratio, headphones;

void surround_init(void)
{
    depth = 0x7FFF;
    position = 0;
    rear_ratio = 0x8000;
    headphones = 0;
}

// dirty bits 28..31: headphones, depth, position, rear ratio
u16 surround_parse(u16 hi, vu16 *d)
{
    u16 changed = 0;
    if (hi & (1u << 12))
        headphones = d[13];
    if (hi & (1u << 13))
        depth = d[14];
    if (hi & (1u << 14))
        position = d[15];
    if ((hi & (1u << 15)) && rear_ratio != d[17]) {
        rear_ratio = d[17];
        changed = 1;
    }
    return changed;
}

u16 surround_rear_ratio(void) { return rear_ratio; }

static s16 sat16(s32 v) { return v > 32767 ? 32767 : v < -32768 ? -32768 : (s16)v; }

void surround_run(s32 *main, const s32 *rear, const s32 *post, u16 mode)
{
    u16 k;
    if (mode == 1) {                    // signed Q15 ratio, 0x8000 = 1.0
        for (k = 0; k < 2 * F; k++)
            main[k] += mul16(sat16(rear[k] >> 14), (s16)rear_ratio) >> 1;
        return;
    }
    for (k = 0; k < 2 * F; k++)
        main[k] += rear[k] + post[k];
}
