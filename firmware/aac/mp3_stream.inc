/* SPDX-License-Identifier: BSD-2-Clause */
/* Framing and PCM output share the codec bank-1 workspace. */
typedef struct {
    u32 skip, pcm_address, skipped;
    u16 read, write, producer, completed, flags, error, eof, head_ready;
    u16 output_active, output_used, output_slot, notice_read;
    u16 status_sequence, resync;
} mp3_byte_stream;
#define BS ((mp3_byte_stream *)(time + 3072))
#define STREAM_META ((vu16 *)MP3_STREAM_META_WORD)
typedef char mp3_byte_stream_fits[(sizeof(mp3_byte_stream) <= (4084 - 3072) * sizeof(u16)) ? 1 : -1];

void aac_stream_kick(void)
{
    stream_waiting = 0;
}
void aac_stream_played(u32 address)
{
    if (arena && stream_mode && address >= stream_pcm_begin && address < stream_pcm_end)
        stream_play_pending++;
}
void aac_stream_frame(void)
{
    /* Audio's pipe-2 frame notification is queued first. Retry on subsequent
     * frames until ARM acknowledges the number of reaped NDSP buffers. */
    if (arena && stream_mode && stream_play_pending != stream_play_ack) {
        if (STREAM_META[2] == session && !(STREAM_META[31] & 0x8000))
            stream_play_ack = STREAM_META[6];
        if (stream_play_pending != stream_play_ack)
            codec_notify();
    }
}
static void byte_stream_status(void)
{
    u16 i;
    for (i = 0; i < 16; ++i)
        result[i] = 0;
    u16 sequence = (BS->status_sequence + 1) & 0x7fff;
    BS->status_sequence = sequence;
    result[0] = sequence ^ 0x8000;
    result[1] = session;
    result[2] = BS->flags;
    result[3] = BS->error;
    result[4] = BS->completed;
    result[5] = codec_rate;
    result[6] = codec_channels;
    result[7] = BS->read;
    result[10] = (u16)(BS->skipped >> 16);
    result[11] = (u16)BS->skipped;
    result[15] = sequence;
    for (i = 0; i < 16; ++i)
        STREAM_META[MP3_STREAM_STATUS / 2 + i] = result[i];
    STREAM_META[MP3_STREAM_STATUS / 2] = sequence;
    codec_notify();
}
static void byte_stream_control(void)
{
    u16 i, before;
    for (;;) {
        before = STREAM_META[31];
        for (i = 0; i < 32; ++i)
            control[i] = STREAM_META[i];
        if (!(before & 0x8000) && before == STREAM_META[31])
            break;
        stream_waiting = 1;
        aac_yield();
    }
    if (dma_error() || control[0] != MP3_MAGIC || control[1] != MP3_STREAM_ABI || control[2] != session ||
        (u16)(control[3] - BS->read) > MP3_STREAM_INPUT_BYTES ||
        (u16)(control[4] - BS->completed) > MP3_STREAM_SLOTS || control[5] > 1) {
        BS->flags |= MP3_STREAM_ERROR;
        BS->error = AAC_BAD_TRANSPORT;
        return;
    }
    BS->write = control[3];
    BS->producer = control[4];
    BS->eof = control[5];
}
static void byte_stream_fail(u16 status)
{
    BS->flags |= MP3_STREAM_ERROR;
    BS->error = status;
    errors++;
    byte_stream_status();
    for (;;) {
        stream_waiting = 1;
        aac_yield();
    }
}
static void byte_stream_wait(void)
{
    if (dma_error() || (BS->flags & MP3_STREAM_ERROR))
        byte_stream_fail(AAC_BAD_TRANSPORT);
    stream_waiting = 1;
    aac_yield();
    byte_stream_control();
}
static void byte_stream_init(void)
{
    u16 i;
    u16 *p = (u16 *)BS;
    for (i = 0; i < sizeof(*BS) / sizeof(u16); ++i)
        p[i] = 0;
    /* The last complete stage snapshot survives decoder-history loop resets.
     * It occupies the unused end of the existing bank-1 reservation. */
    if (!jobs)
        for (i = 0; i < 6; ++i)
            MP3_STREAM_LAST_TICKS[i] = 0;
}
static u16 byte_stream_available(void)
{
    return BS->write - BS->read;
}
static void byte_stream_need(u16 n)
{
    byte_stream_control();
    while (byte_stream_available() < n) {
        if (BS->flags & MP3_STREAM_ERROR)
            byte_stream_fail(BS->error);
        if (BS->eof)
            byte_stream_fail(MP3_TRUNCATED);
        if (!(BS->flags & MP3_STREAM_NEED_INPUT)) {
            BS->flags |= MP3_STREAM_NEED_INPUT;
            byte_stream_status();
        }
        byte_stream_wait();
    }
    BS->flags &= ~MP3_STREAM_NEED_INPUT;
}
static void byte_stream_load(u16 n)
{
    u16 offset = BS->read & (MP3_STREAM_INPUT_BYTES - 1), odd = offset & 1;
    u16 words = (n + odd + 1) >> 1, first = (MP3_STREAM_INPUT_BYTES - (offset & ~1u)) >> 1;
    if (first > words)
        first = words;
    dma_read(arena + MP3_STREAM_INPUT + (offset & ~1u), first, (u16)(unsigned)aac_bs_data);
    if (words > first)
        dma_read(arena + MP3_STREAM_INPUT, words - first, (u16)(unsigned)(aac_bs_data + first));
    if (dma_error())
        byte_stream_fail(AAC_BAD_TRANSPORT);
    if (odd)
        mp3_align_bytes(aac_bs_data, (n + 1) >> 1);
}
static u16 byte_stream_byte(u16 i)
{
    u16 v = ((volatile u16 *)aac_bs_data)[i >> 1];
    return (v >> ((i & 1) << 3)) & 255;
}
static void byte_stream_advance(u16 n)
{
    BS->read += n;
    if ((u16)(BS->read - BS->notice_read) >= MP3_STREAM_REFILL) {
        BS->notice_read = BS->read;
        byte_stream_status();
    }
}
static void byte_stream_complete(void)
{
    u16 i, seq = BS->completed + 1;
    for (i = 0; i < 16; ++i)
        result[i] = 0;
    result[0] = seq ^ 0x8000;
    result[2] = codec_channels;
    result[3] = codec_rate;
    result[8] = session;
    result[9] = BS->output_used;
    for (i = 0; i < 16; ++i)
        STREAM_META[(MP3_STREAM_RESULTS >> 1) + ((BS->completed & 7) << 5) + i] = result[i];
    STREAM_META[(MP3_STREAM_RESULTS >> 1) + ((BS->completed & 7) << 5)] = seq;
    BS->completed = seq;
    BS->output_active = BS->output_used = 0;
    byte_stream_status();
}
static void byte_stream_output(void)
{
    byte_stream_control();
    while (BS->producer == BS->completed)
        byte_stream_wait();
    if (BS->flags & MP3_STREAM_ERROR)
        byte_stream_fail(BS->error);
    result[0] = STREAM_META[(MP3_STREAM_DESCRIPTORS >> 1) + ((BS->completed & 7) << 5)];
    result[1] = STREAM_META[(MP3_STREAM_DESCRIPTORS >> 1) + ((BS->completed & 7) << 5) + 1];
    if (dma_error() || result[0] >= stream_count || result[1] != session)
        byte_stream_fail(AAC_BAD_TRANSPORT);
    BS->output_slot = result[0];
    BS->pcm_address = stream_pcm_begin + mp3_slot_bytes(result[0], stream_frames);
    BS->output_active = 1;
    BS->output_used = 0;
    stream_started = 1;
}
static void byte_stream_emit(const u16 *samples, u16 frames)
{
    u16 at = 0;
    if (frames != 64)
        byte_stream_fail(AAC_BAD_TRANSPORT);
    while (at < frames) {
        u16 n = frames - at, channels = codec_channels, count;
        u16 *stage = codec_work + 4480;
        if (!BS->output_active)
            byte_stream_output();
        if (n > stream_frames - BS->output_used)
            n = stream_frames - BS->output_used;
        count = channels == 2 ? n << 1 : n;
        (void)samples;
        dma_write((u16)(unsigned)(stage + (channels == 2 ? at << 1 : at)), count,
                  BS->pcm_address + ((u32)BS->output_used << (channels == 2 ? 2 : 1)));
        if (dma_error())
            byte_stream_fail(AAC_BAD_TRANSPORT);
        BS->output_used += n;
        at += n;
        if (BS->output_used == stream_frames)
            byte_stream_complete();
    }
}
static u16 byte_stream_header(void)
{
    /* Legal MPEG-1 sizes; power-of-two row stride avoids compiler products. */
    static const u16 lengths[3][16] = {
        {0, 104, 130, 156, 182, 208, 261, 313, 365, 417, 522, 626, 731, 835, 1044, 0},
        {0, 96, 120, 144, 168, 192, 240, 288, 336, 384, 480, 576, 672, 768, 960, 0},
        {0, 144, 180, 216, 252, 288, 360, 432, 504, 576, 720, 864, 1008, 1152, 1440, 0}};
    u16 h1 = byte_stream_byte(1), h2 = byte_stream_byte(2), h3 = byte_stream_byte(3);
    if (byte_stream_byte(0) != 255 || (h1 & 0xe0) != 0xe0 || (h1 & 6) != 2 || (h1 & 0x18) == 8 ||
        (h2 & 12) == 12 || (h2 >> 4) == 15 || (h3 & 3) == 2)
        return 0;
    if ((h1 & 0x18) != 0x18 || !(h2 >> 4) || (h3 & 3) || (h3 & 0xd0) == 0x50)
        byte_stream_fail(MP3_UNSUPPORTED);
    return lengths[(h2 >> 2) & 3][h2 >> 4] + ((h2 >> 1) & 1);
}
static void byte_stream_tags(void)
{
    byte_stream_need(4);
    byte_stream_load(4);
    if (byte_stream_byte(0) == 73 && byte_stream_byte(1) == 68 && byte_stream_byte(2) == 51) {
        u16 i, v, version, footer;
        byte_stream_need(10);
        byte_stream_load(10);
        version = byte_stream_byte(3);
        footer = byte_stream_byte(5) & 16;
        if (version < 2 || version > 4 || byte_stream_byte(4) == 255)
            byte_stream_fail(MP3_CORRUPT);
        BS->skip = 0;
        for (i = 6; i < 10; ++i) {
            v = byte_stream_byte(i);
            if (v & 128)
                byte_stream_fail(MP3_CORRUPT);
            BS->skip = (BS->skip << 7) | v;
        }
        if (BS->skip > 16777216UL)
            byte_stream_fail(MP3_UNSUPPORTED);
        BS->skip += 10 + (version == 4 && footer ? 10 : 0);
        while (BS->skip) {
            u16 n;
            byte_stream_need(1);
            n = byte_stream_available();
            if ((u32)n > BS->skip)
                n = (u16)BS->skip;
            BS->skipped += n;
            byte_stream_advance(n);
            BS->skip -= n;
            aac_yield();
        }
    }
}
static void byte_stream_task(void)
{
    if (!BS->head_ready) {
        byte_stream_tags();
        BS->head_ready = 1;
    }
    for (;;) {
        u16 n, status, i;
        byte_stream_control();
        if (BS->flags & MP3_STREAM_ERROR)
            byte_stream_fail(BS->error);
        n = byte_stream_available();
        if (BS->eof && (!n || n == 128)) {
            if (n) {
                byte_stream_load(4);
                if (byte_stream_byte(0) != 84 || byte_stream_byte(1) != 65 || byte_stream_byte(2) != 71)
                    byte_stream_fail(MP3_TRUNCATED);
                BS->skipped += n;
                byte_stream_advance(n);
            }
            if (!(BS->flags & MP3_STREAM_HEADER))
                byte_stream_fail(MP3_TRUNCATED);
            if (BS->output_active)
                byte_stream_complete();
            while (BS->completed != BS->producer) {
                byte_stream_output();
                byte_stream_complete();
            }
            BS->flags |= MP3_STREAM_DRAINED;
            byte_stream_status();
            for (;;) {
                stream_waiting = 1;
                aac_yield();
            }
        }
        byte_stream_need(4);
        byte_stream_load(4);
        n = byte_stream_header();
        if (!n) {
            if (++BS->resync > MP3_STREAM_SYNC_LIMIT)
                byte_stream_fail(MP3_CORRUPT);
            BS->skipped++;
            byte_stream_advance(1);
            aac_yield();
            continue;
        }
        BS->resync = 0;
        if (!(BS->flags & MP3_STREAM_HEADER)) {
            codec_channels = (byte_stream_byte(3) >> 6) == 3 ? 1 : 2;
            codec_rate = (byte_stream_byte(2) >> 2) & 3;
            BS->flags |= MP3_STREAM_HEADER;
            byte_stream_status();
        }
        if (!stream_started)
            byte_stream_output();
        byte_stream_need(n);
        byte_stream_load(n);
        byte_stream_advance(n);
        if (stats.period_last && stats.render_raw_last > stats.period_last / 2)
            byte_stream_fail(AAC_RESOURCE_BUSY);
        pcm_frames = 0;
        frame_ticks = 0;
        slice_start = job_start = aac_timer_now();
        if (stream_profile) {
            for (i = 0; i < 6; ++i)
                MP3_STATE->stage_ticks[i] = 0;
            MP3_STATE->stage = 0;
            mp3_slice_start = aac_timer_now();
        }
        status = mp3_frame(&mp3, n);
        if (stream_profile) {
            mp3_charge_stage();
            for (i = 0; i < 6; ++i)
                MP3_STREAM_LAST_TICKS[i] = MP3_STATE->stage_ticks[i];
        }
        if (status != MP3_HISTORY && status)
            byte_stream_fail(status);
        if (!status)
            jobs++;
        completed_ticks = frame_ticks + aac_timer_elapsed(slice_start);
        latency_last = aac_timer_elapsed(job_start);
        if (latency_last > latency_max)
            latency_max = latency_last;
        aac_yield();
    }
}
