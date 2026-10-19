// SPDX-License-Identifier: BSD-2-Clause
#include <limits.h>
#include "codec.h"

bool opendsp_audio_input_cancelled(const audio_input *in)
{
    return in->generation && __atomic_load_n(in->generation, __ATOMIC_ACQUIRE) != in->expected;
}

Result opendsp_audio_input_open(audio_input *in, const char *path)
{
    in->file = fopen(path, "rb");
    if (!in->file) return OPENDSP_AUDIO_ERR_IO;
    if (fseek(in->file, 0, SEEK_END)) return OPENDSP_AUDIO_ERR_IO;
    long size = ftell(in->file);
    if (size < 0) return OPENDSP_AUDIO_ERR_IO;
    // ARM stdio uses signed 32-bit offsets.
    if ((u64)size > INT32_MAX) return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    in->size = (u64)size;
    return opendsp_audio_input_seek(in, 0);
}

Result opendsp_audio_input_seek(audio_input *in, u64 offset)
{
    if (offset > in->size || offset > INT32_MAX) return OPENDSP_AUDIO_ERR_ARGUMENT;
    if (opendsp_audio_input_cancelled(in)) return OPENDSP_AUDIO_ERR_BUSY;
    if (fseek(in->file, (long)offset, SEEK_SET)) return OPENDSP_AUDIO_ERR_IO;
    clearerr(in->file);
    return 0;
}

Result opendsp_audio_input_read(audio_input *in, void *data, size_t size, size_t *got)
{
    *got = 0;
    while (*got < size) {
        if (opendsp_audio_input_cancelled(in)) return OPENDSP_AUDIO_ERR_BUSY;
        size_t count = size - *got;
        if (count > 4096) count = 4096;
        u64 start = svcGetSystemTick();
        size_t n = fread((u8 *)data + *got, 1, count, in->file);
        if (in->stats) {
            in->stats->read_ticks += svcGetSystemTick() - start;
            in->stats->input_bytes += n;
        }
        *got += n;
        if (ferror(in->file)) return OPENDSP_AUDIO_ERR_IO;
        if (!n) break;
    }
    return 0;
}
