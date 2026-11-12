// SPDX-License-Identifier: BSD-2-Clause
#include "code.h"
#include "decoder.h"
#include "task.h"

static u16 current_bank = 0xffff, required_bank = 0xffff;
static ogg_code_loader loader;
// Spare command word; open_stream clears it.
#define OVERLAY_SESSION ((vu16 *)(OGG_DECODE_WORD + 31))
#ifdef OPENDSP_CELT_SHARED
extern const u16 ogg_celt_prefix_bytes;
#endif

static u32 read_u32(const volatile u16 *words)
{
    return ((u32)words[0] << 16) | words[1];
}

u16 overlay_enter(u16 wanted)
{
    vu16 *status = (void *)OGG_OVERLAY_WORD;
    vu16 *config = (void *)OGG_OVERLAY_CONFIG;
    const vu16 *descriptor = (void *)(OGG_OVERLAY_DIRECTORY + wanted * 4u);
    u16 previous = required_bank;
    u32 source, bytes;
    // New session: a cancelled task may have stopped mid-load.
    if (!*OVERLAY_SESSION) {
        current_bank = 0xffff;
        loader.arena_end = 0;
        *OVERLAY_SESSION = 1;
    }
    required_bank = wanted;
    if (wanted == 0xffff || wanted == current_bank) return previous;
    if (wanted >= config[4]) goto failed;
    if (!loader.arena_end && ogg_code_open(&loader, read_u32(config), read_u32(config + 2),
                                          CODEC_PROGRAM_WINDOW, 0x20000UL)) goto failed;
    source = read_u32(descriptor);
    bytes = read_u32(descriptor + 2);
#ifdef OPENDSP_CELT_SHARED
    {
        u16 prefix = (current_bank == 2 || current_bank == 3) && (wanted == 2 || wanted == 3)
                         ? ogg_celt_prefix_bytes : 0;
        if (prefix >= bytes || (prefix & 31)) goto failed;
        loader.window_begin = CODEC_PROGRAM_WINDOW + (prefix >> 1);
        source += prefix;
        bytes -= prefix;
    }
#endif
    if (ogg_code_load(&loader, wanted, source, bytes)) goto failed;
    current_bank = wanted;
    status[0] = 0;
    status[1] = wanted;
    return previous;
failed:
    status[0] = 1;
    for (;;) ogg_opus_pause();
}
