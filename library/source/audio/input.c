// SPDX-License-Identifier: BSD-2-Clause
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "codec.h"

bool opendsp_audio_input_cancelled(const audio_input *in)
{
    return in->generation && __atomic_load_n(in->generation, __ATOMIC_ACQUIRE) != in->expected;
}

Result opendsp_audio_input_open(audio_input *in, const char *path)
{
    in->file = fopen(path, "rb");
    if (!in->file) return OPENDSP_AUDIO_ERR_IO;
    in->block = malloc(AUDIO_INPUT_BLOCKS * AUDIO_INPUT_BLOCK);
    if (!in->block) return OPENDSP_AUDIO_ERR_MEMORY;
    memset(in->block_bytes, 0, sizeof(in->block_bytes));
    // Reads are whole blocks; stdio buffering only copies.
    setvbuf(in->file, NULL, _IONBF, 0);
    if (fseek(in->file, 0, SEEK_END)) return OPENDSP_AUDIO_ERR_IO;
    long size = ftell(in->file);
    if (size < 0) return OPENDSP_AUDIO_ERR_IO;
    // ARM stdio uses signed 32-bit offsets.
    if ((u64)size > INT32_MAX) return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    in->size = in->file_at = (u64)size;
    return opendsp_audio_input_seek(in, 0);
}

void opendsp_audio_input_close(audio_input *in)
{
    if (in->file) fclose(in->file);
    free(in->block);
    in->file = NULL;
    in->block = NULL;
    memset(in->block_bytes, 0, sizeof(in->block_bytes));
}

Result opendsp_audio_input_seek(audio_input *in, u64 offset)
{
    if (offset > in->size || offset > INT32_MAX) return OPENDSP_AUDIO_ERR_ARGUMENT;
    if (opendsp_audio_input_cancelled(in)) return OPENDSP_AUDIO_ERR_BUSY;
    in->at = offset;
    return 0;
}

// Each SD read costs ~2.4 ms on HW.
static Result fill(audio_input *in, void *data, u64 offset, size_t size, size_t *got)
{
    *got = 0;
    if (in->file_at != offset) {
        if (fseek(in->file, (long)offset, SEEK_SET)) return OPENDSP_AUDIO_ERR_IO;
        clearerr(in->file);
        in->file_at = offset;
    }
    u64 start = svcGetSystemTick();
    size_t n = fread(data, 1, size, in->file);
    if (in->stats) {
        in->stats->read_ticks += svcGetSystemTick() - start;
        in->stats->input_bytes += n;
    }
    in->file_at += n;
    *got = n;
    return ferror(in->file) ? OPENDSP_AUDIO_ERR_IO : 0;
}

// Cached block holding the read position.
static int find_block(const audio_input *in)
{
    for (u32 i = 0; i < AUDIO_INPUT_BLOCKS; ++i)
        if (in->at >= in->block_at[i] && in->at < in->block_at[i] + in->block_bytes[i])
            return (int)i;
    return -1;
}

// Least recently used block.
static int oldest_block(const audio_input *in)
{
    int slot = 0;
    for (u32 i = 1; i < AUDIO_INPUT_BLOCKS; ++i)
        if (in->block_used[i] < in->block_used[slot]) slot = (int)i;
    return slot;
}

Result opendsp_audio_input_read(audio_input *in, void *data, size_t size, size_t *got)
{
    *got = 0;
    while (*got < size && in->at < in->size) {
        if (opendsp_audio_input_cancelled(in)) return OPENDSP_AUDIO_ERR_BUSY;
        size_t count = size - *got, n;
        int slot = find_block(in);
        if (slot < 0) {
            if (count >= AUDIO_INPUT_BLOCK && !(in->at % AUDIO_INPUT_BLOCK)) {
                // Large aligned reads bypass the block.
                count -= count % AUDIO_INPUT_BLOCK;
                Result r = fill(in, (u8 *)data + *got, in->at, count, &n);
                in->at += n;
                *got += n;
                if (R_FAILED(r)) return r;
                if (!n) break;
                // Keep the tail; Ogg seeks re-read it.
                if (n >= AUDIO_INPUT_BLOCK) {
                    slot = oldest_block(in);
                    memcpy(in->block + slot * AUDIO_INPUT_BLOCK, (u8 *)data + *got - AUDIO_INPUT_BLOCK,
                           AUDIO_INPUT_BLOCK);
                    in->block_at[slot] = in->at - AUDIO_INPUT_BLOCK;
                    in->block_bytes[slot] = AUDIO_INPUT_BLOCK;
                    in->block_used[slot] = ++in->clock;
                }
                continue;
            }
            slot = oldest_block(in);
            u64 base = in->at - in->at % AUDIO_INPUT_BLOCK;
            in->block_bytes[slot] = 0;
            Result r = fill(in, in->block + slot * AUDIO_INPUT_BLOCK, base, AUDIO_INPUT_BLOCK, &n);
            in->block_at[slot] = base;
            in->block_bytes[slot] = (u32)n;
            if (R_FAILED(r)) return r;
            if (in->at >= base + n) break;
        }
        in->block_used[slot] = ++in->clock;
        u32 skip = (u32)(in->at - in->block_at[slot]);
        n = in->block_bytes[slot] - skip;
        if (n > count) n = count;
        memcpy((u8 *)data + *got, in->block + slot * AUDIO_INPUT_BLOCK + skip, n);
        in->at += n;
        *got += n;
    }
    return 0;
}
