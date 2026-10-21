// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <stdlib.h>
#include <string.h>
#include "aac_backend.h"
#include "internal.h"
#include "aac_protocol.h"
#include "mp3_protocol.h"
#include "opendsp.h"
#include "codec_events.h"

#ifndef OPENDSP_AAC_WAIT_MS
#define OPENDSP_AAC_WAIT_MS 250
#endif

struct aac_dsp_backend {
    u8 *memory;
    volatile u16 *metadata;
    u16 sequence, session;
    u16 codec;
    u16 stream_count, stream_frames, input_write, stream_completed;
    u16 played_ack, control_sequence;
    u32 epoch, generation;
    bool connected, changed, faulted, ever_connected, discontinuity, reset_requested, stopped,
        preserve_output, input_eof, profile;
};
extern const u16 opendsp_aac_tables[AAC_TABLE_WORDS];
extern const u16 opendsp_mp3_tables[MP3_TABLE_WORDS];
static LightLock owner_lock;
static bool setup, sleeping;
static volatile u32 epoch;
static u32 dsp_generation;
static u32 current_generation(void)
{
    return __atomic_load_n(&dsp_generation, __ATOMIC_ACQUIRE);
}
static u16 next_session;
static dspHookCookie hook;
static aac_dsp_backend *owner;
static void *quarantined;
static u32 quarantine_epoch;
static Result codec_backend_allocate(aac_dsp_backend **out, u16 codec, u16 count, u16 frames, bool profile);

static unsigned shared_bytes(const aac_dsp_backend *d)
{
    if (d->codec == 2)
        return MP3_STREAM_PCM + d->stream_count * ((d->stream_frames * 4u + 31) & ~31u);
    return d->codec ? MP3_SHARED_BYTES : AAC_SHARED_BYTES;
}
static unsigned pcm_bytes(const aac_dsp_backend *d)
{
    return d->codec ? MP3_PCM_BYTES : AAC_PCM_BYTES;
}
static u16 codec_command(const aac_dsp_backend *d, u16 command)
{
    return command + (d->codec ? MP3_CMD_OPEN - AAC_CMD_OPEN : 0);
}

static void power_hook(DSP_HookType type)
{
    if (type == DSPHOOK_ONSLEEP || type == DSPHOOK_ONCANCEL) {
        sleeping = true;
        epoch++;
    }
    if (type == DSPHOOK_ONWAKEUP) {
        sleeping = false;
        epoch++;
        __atomic_add_fetch(&dsp_generation, 1, __ATOMIC_RELEASE);
    }
}

void opendsp_aac_setup(void)
{
    if (!setup) {
        LightLock_Init(&owner_lock);
        dspHook(&hook, power_hook);
        setup = true;
    }
}

void opendsp_aac_release(void)
{
    if (!setup)
        return;
    LightLock_Lock(&owner_lock);
    if (owner) {
        owner->connected = false;
        owner->epoch = epoch - 1;
    }
    if (quarantined && !dspIsComponentLoaded()) {
        linearFree(quarantined);
        quarantined = NULL;
    }
    epoch++;
    sleeping = false;
    LightLock_Unlock(&owner_lock);
}

static u16 new_session(void)
{
    if (!++next_session)
        ++next_session;
    return next_session;
}

static void stream_metadata_init(aac_dsp_backend *d)
{
    for (unsigned i = 0; i < MP3_STREAM_META_BYTES / 2; ++i)
        d->metadata[i] = 0;
    d->metadata[0] = MP3_MAGIC;
    d->metadata[1] = MP3_STREAM_ABI;
    d->metadata[2] = d->session;
    d->played_ack = d->control_sequence = 0;
    d->input_eof = false;
    __dsb();
}

static Result connect_stream(aac_dsp_backend *d)
{
    u32 address, physical, caps = 0;
    u16 sum = 0, args[8];
    Result rc;
    unsigned words = d->codec ? MP3_TABLE_WORDS : AAC_TABLE_WORDS;
    const u16 *tables = d->codec ? opendsp_mp3_tables : opendsp_aac_tables;
    if (!opendsp_running || sleeping)
        return OPENDSP_AAC_ERR_BACKEND;
    if (R_FAILED(opendspGetCapabilities(&caps)) || !(caps & (d->codec ? MP3_CAP_DSP : AAC_CAP_DSP)))
        return OPENDSP_AAC_ERR_BACKEND;
    if (d->codec && !(caps & MP3_CAP_STREAM))
        return OPENDSP_MP3_ERR_BACKEND;
    if ((caps & MP3_CAP_STREAM) && R_FAILED(opendsp_codec_events_start()))
        return OPENDSP_AAC_ERR_BACKEND;
    if (d->codec == 2) {
        u32 metadata_address;
        if (R_FAILED(DSP_ConvertProcessAddressFromDspDram(MP3_STREAM_META_WORD, &metadata_address)))
            return OPENDSP_MP3_ERR_BACKEND;
        d->metadata = (volatile u16 *)metadata_address;
    }
    rc = DSP_ConvertProcessAddressFromDspDram(AAC_TABLE_ADDRESS, &address);
    if (R_FAILED(rc))
        return OPENDSP_AAC_ERR_BACKEND;
    for (unsigned i = 0; i < words; i++)
        sum += tables[i];
    unsigned cleared = d->codec == 2 && d->ever_connected ? MP3_STREAM_INPUT : shared_bytes(d);
    memset(d->memory, 0, cleared);
    d->session = new_session();
    d->sequence = 0;
    d->input_write = 0;
    d->stream_completed = 0;
    d->changed = true;
    u16 abi = d->codec == 2 ? MP3_STREAM_ABI : d->codec ? MP3_ABI : AAC_ABI;
    if (d->codec != 2) {
        u16 *control = (u16 *)d->memory;
        control[0] = d->codec ? MP3_MAGIC : AAC_MAGIC;
        control[1] = abi;
        control[2] = d->session;
    }
    if (cleared)
        DSP_FlushDataCache(d->memory, cleared);
    physical = osConvertVirtToPhys(d->memory);
    args[0] = physical >> 16;
    args[1] = physical;
    args[2] = abi;
    args[3] = d->session;
    args[4] = sum;
    d->connected = true; // a timed-out request may already have been accepted
    args[5] = d->stream_count;
    args[6] = d->stream_frames;
    args[7] = d->profile;
    rc = opendsp_request(d->codec == 2 ? MP3_CMD_STREAM_OPEN : codec_command(d, AAC_CMD_OPEN), args,
                         d->codec == 2 ? 8 : 5, NULL, 0);
    if (R_FAILED(rc)) {
        if (rc == OPENDSP_ERR_ARGUMENT || rc == OPENDSP_ERR_ADDRESS || rc == OPENDSP_ERR_UNSUPPORTED)
            d->connected = false;
        return OPENDSP_AAC_ERR_BACKEND;
    }
    if (d->codec == 2)
        stream_metadata_init(d);
    /* Acquire the codec lease before writing its shared tables. */
    memcpy((void *)address, tables, words * 2);
    __dsb();
    d->epoch = epoch;
    d->generation = current_generation();
    d->stopped = false;
    d->faulted = false;
    d->discontinuity = d->ever_connected && !d->reset_requested;
    d->ever_connected = true;
    d->reset_requested = false;
    return 0;
}

Result aac_backend_open(aac_dsp_backend **out)
{
    return codec_backend_open(out, 0);
}

Result codec_backend_open(aac_dsp_backend **out, u16 codec)
{
    return codec_backend_allocate(out, codec, 0, 0, false);
}

Result mp3_backend_stream_open(aac_dsp_backend **out, u16 count, u16 frames, bool profile)
{
    if (out)
        *out = NULL;
    if (count < 3 || count > 8 || frames < 256 || frames > 16384 ||
        (u32)(count - 1) * frames < MP3_STREAM_MIN_FRAMES)
        return OPENDSP_MP3_ERR_INVALID_ARG;
    return codec_backend_allocate(out, 2, count, frames, profile);
}

static Result codec_backend_allocate(aac_dsp_backend **out, u16 codec, u16 count, u16 frames, bool profile)
{
    Result rc;
    aac_dsp_backend *d;
    if (!out || codec > 2)
        return OPENDSP_AAC_ERR_INVALID_ARG;
    *out = NULL;
    opendsp_aac_setup();
    LightLock_Lock(&owner_lock);
    if (quarantined && !sleeping && (!dspIsComponentLoaded() || quarantine_epoch != current_generation())) {
        linearFree(quarantined);
        quarantined = NULL;
    }
    if (owner || quarantined) {
        LightLock_Unlock(&owner_lock);
        return OPENDSP_AAC_ERR_BUSY;
    }
    d = calloc(1, sizeof(*d));
    if (!d) {
        LightLock_Unlock(&owner_lock);
        return OPENDSP_AAC_ERR_NO_MEMORY;
    }
    d->codec = codec;
    d->profile = profile;
    d->stream_count = count;
    d->stream_frames = frames;
    d->memory = linearMemAlign(shared_bytes(d), 0x80);
    if (!d->memory) {
        free(d);
        LightLock_Unlock(&owner_lock);
        return OPENDSP_AAC_ERR_NO_MEMORY;
    }
    rc = connect_stream(d);
    if (R_FAILED(rc)) {
        u16 args[1] = {d->session};
        if (!d->connected ||
            R_SUCCEEDED(opendsp_request(codec_command(d, AAC_CMD_CLOSE), args, 1, NULL, 0))) {
            linearFree(d->memory);
        } else {
            quarantined = d->memory;
            quarantine_epoch = current_generation();
        }
        free(d);
    } else {
        owner = d;
        *out = d;
    }
    LightLock_Unlock(&owner_lock);
    return rc;
}

static Result decode_status(u16 status)
{
    switch (status) {
    case 0:
        return 0;
    case 1:
        return OPENDSP_AAC_ERR_NEED_MORE_DATA;
    case 2:
        return OPENDSP_AAC_INFO_SKIPPED;
    case 3:
        return OPENDSP_AAC_ERR_CORRUPT;
    case 4:
        return OPENDSP_AAC_ERR_UNSUPPORTED;
    case AAC_RESOURCE_BUSY:
        return OPENDSP_AAC_ERR_BUSY;
    default:
        return OPENDSP_AAC_ERR_BACKEND;
    }
}

static Result submit_frame(aac_dsp_backend *d, const u8 *p, unsigned len, u16 **out)
{
    unsigned slot;
    u16 *control, *result;
    Result rc;
    if (!opendsp_running || sleeping)
        return OPENDSP_AAC_ERR_BACKEND;
    if (d->faulted)
        return OPENDSP_AAC_ERR_TIMEOUT;
    if (d->epoch != epoch || !d->connected) {
        rc = connect_stream(d);
        if (R_FAILED(rc))
            return rc;
    }
    slot = d->sequence & 3;
    control = (u16 *)d->memory;
    result = control + 64 + slot * 16;
    memcpy(d->memory + AAC_INPUT_OFFSET + slot * AAC_INPUT_BYTES, p, len);
    if (len & 1)
        d->memory[AAC_INPUT_OFFSET + slot * AAC_INPUT_BYTES + len] = 0;
    DSP_FlushDataCache(d->memory + AAC_INPUT_OFFSET + slot * AAC_INPUT_BYTES, (len + 31) & ~31u);
    control[16 + slot * 4] = len;
    __dsb();
    control[3] = d->sequence + 1;
    DSP_FlushDataCache(d->memory, 64);
    u64 deadline = svcGetSystemTick() + (u64)SYSCLOCK_ARM11 * OPENDSP_AAC_WAIT_MS / 1000;
    for (;;) {
        DSP_InvalidateDataCache(result, 32);
        __dsb();
        if (result[0] == (u16)(d->sequence + 1) && result[8] == d->session)
            break;
        if (d->epoch != epoch || sleeping || !opendsp_running) {
            d->connected = false;
            return OPENDSP_AAC_ERR_BACKEND;
        }
        if (svcGetSystemTick() > deadline) {
            d->faulted = true;
            return OPENDSP_AAC_ERR_TIMEOUT;
        }
        if (opendsp_codec_events_active()) {
            u64 now = svcGetSystemTick();
            if (now < deadline)
                opendsp_codec_wait((s64)((deadline - now) * 1000000000ull / SYSCLOCK_ARM11));
        } else
            svcSleepThread(500000);
    }
    d->sequence++;
    *out = result;
    return 0;
}

static void read_pcm(aac_dsp_backend *d, unsigned slot, s16 *pcm, unsigned frames, unsigned channels)
{
    unsigned count = frames * channels * 2;
    u8 *source = d->memory + AAC_PCM_OFFSET + slot * pcm_bytes(d);
    DSP_InvalidateDataCache(source, (count + 31) & ~31u);
    __dsb();
    memcpy(pcm, source, count);
}

Result aac_backend_decode(aac_dsp_backend *d, const void *input, size_t bytes, size_t *used, s16 *pcm,
                          size_t capacity, size_t *frames, opendspAacInfo *info)
{
    static const u32 rates[12] = {96000, 88200, 64000, 48000, 44100, 32000,
                                  24000, 22050, 16000, 12000, 11025, 8000};
    const u8 *p = input;
    size_t skip = 0;
    unsigned len, channels, sf, blocks, header, slot;
    u16 *result;
    Result rc;
    if (used)
        *used = 0;
    if (frames)
        *frames = 0;
    if (!d || d->codec || !used || !frames || !pcm || (!p && bytes))
        return OPENDSP_AAC_ERR_INVALID_ARG;
    if (!bytes)
        return OPENDSP_AAC_ERR_NEED_MORE_DATA;
    while (skip + 1 < bytes && (p[skip] != 255 || (p[skip + 1] & 0xf6) != 0xf0))
        skip++;
    if (skip + 1 >= bytes)
        skip = p[bytes - 1] == 255 ? bytes - 1 : bytes;
    if (skip) {
        *used = skip;
        return OPENDSP_AAC_INFO_SKIPPED;
    }
    if (bytes < 7)
        return OPENDSP_AAC_ERR_NEED_MORE_DATA;
    sf = (p[2] >> 2) & 15;
    channels = (p[2] & 1) << 2 | p[3] >> 6;
    len = (p[3] & 3) << 11 | p[4] << 3 | p[5] >> 5;
    blocks = (p[6] & 3) + 1;
    header = 7 + ((p[1] & 1) ? 0 : 2 * blocks);
    if (sf >= 13 || len <= header) {
        *used = 1;
        return OPENDSP_AAC_INFO_SKIPPED;
    }
    if (bytes < len)
        return OPENDSP_AAC_ERR_NEED_MORE_DATA;
    if (info) {
        memset(info, 0, sizeof(*info));
        info->sample_rate = sf < 12 ? rates[sf] : 7350;
        info->channels = channels;
        info->frame_samples = 1024;
        info->blocks = blocks;
        info->object_type = (p[2] >> 6) + 1;
    }
    if (sf < 3 || sf > 11 || channels < 1 || channels > 2 || (p[2] >> 6) != 1 || blocks != 1 ||
        len > AAC_INPUT_BYTES) {
        *used = len;
        return OPENDSP_AAC_ERR_UNSUPPORTED;
    }
    if (capacity < 1024)
        return OPENDSP_AAC_ERR_OUTPUT_TOO_SMALL;
    rc = submit_frame(d, p, len, &result);
    if (R_FAILED(rc))
        return rc;
    slot = (d->sequence - 1) & 3;
    rc = decode_status(result[1]);
    if (result[1] != AAC_RESOURCE_BUSY && result[1] != AAC_BAD_TRANSPORT)
        *used = len;
    if (R_FAILED(rc) || result[1])
        return rc;
    if (result[2] != channels || result[3] != sf)
        return OPENDSP_AAC_ERR_BACKEND;
    read_pcm(d, slot, pcm, 1024, channels);
    *frames = 1024;
    if (info)
        info->flags = ((result[5] || d->changed) ? OPENDSP_AAC_INFO_CHANGED : 0) |
                      (result[4] ? OPENDSP_AAC_INFO_SBR_IGNORED : 0) |
                      (d->discontinuity ? OPENDSP_AAC_INFO_DISCONTINUITY : 0);
    d->discontinuity = false;
    d->changed = false;
    return 0;
}

Result mp3_backend_decode(aac_dsp_backend *d, const void *input, size_t bytes, size_t *used, s16 *pcm,
                          size_t capacity, size_t *frames, opendspMp3Info *info)
{
    static const u16 bitrates[16] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
    static const u32 rates[3] = {44100, 48000, 32000};
    const u8 *p = input;
    size_t skip = 0;
    unsigned sf, channels, len, kbps, slot;
    u16 *result;
    Result rc;
    if (used)
        *used = 0;
    if (frames)
        *frames = 0;
    if (info)
        memset(info, 0, sizeof(*info));
    if (!d || !d->codec || !used || !frames || !pcm || (!p && bytes))
        return OPENDSP_MP3_ERR_INVALID_ARG;
    if (!bytes)
        return OPENDSP_MP3_ERR_NEED_MORE_DATA;
    while (skip + 1 < bytes && (p[skip] != 255 || (p[skip + 1] & 0xe6) != 0xe2))
        ++skip;
    if (skip + 1 >= bytes)
        skip = p[bytes - 1] == 255 ? bytes - 1 : bytes;
    if (skip) {
        *used = skip;
        return OPENDSP_MP3_INFO_SKIPPED;
    }
    if (bytes < 4)
        return OPENDSP_MP3_ERR_NEED_MORE_DATA;
    if ((p[1] & 0x18) != 0x18 || !(p[2] >> 4)) {
        *used = 4;
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    }
    sf = p[2] >> 2 & 3;
    kbps = bitrates[p[2] >> 4];
    if (sf == 3 || !kbps || (p[3] & 3) == 2) {
        *used = 1;
        return OPENDSP_MP3_INFO_SKIPPED;
    }
    channels = (p[3] >> 6) == 3 ? 1 : 2;
    len = 144000u * kbps / rates[sf] + (p[2] >> 1 & 1);
    if (bytes < len)
        return OPENDSP_MP3_ERR_NEED_MORE_DATA;
    if (info) {
        info->sample_rate = rates[sf];
        info->channels = channels;
        info->frame_samples = 1152;
        info->bitrate_kbps = kbps;
    }
    if ((p[3] & 0xd0) == 0x50 || (p[3] & 3) || len > AAC_INPUT_BYTES) {
        *used = len;
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    }
    if (capacity < 1152)
        return OPENDSP_MP3_ERR_OUTPUT_TOO_SMALL;
    rc = submit_frame(d, p, len, &result);
    if (R_FAILED(rc))
        return rc;
    slot = (d->sequence - 1) & 3;
    if (result[1] != AAC_RESOURCE_BUSY && result[1] != AAC_BAD_TRANSPORT)
        *used = len;
    switch (result[1]) {
    case 0:
        break;
    case 1:
        return OPENDSP_MP3_INFO_SKIPPED; /* missing bit-reservoir history */
    case 2:
        return OPENDSP_MP3_ERR_NEED_MORE_DATA;
    case 3:
        return OPENDSP_MP3_ERR_CORRUPT;
    case 4:
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    case 6:
        return OPENDSP_MP3_ERR_CRC;
    case AAC_RESOURCE_BUSY:
        return OPENDSP_MP3_ERR_BUSY;
    default:
        return OPENDSP_MP3_ERR_BACKEND;
    }
    if (result[2] != channels || result[3] != sf || result[9] != 1152)
        return OPENDSP_MP3_ERR_BACKEND;
    read_pcm(d, slot, pcm, 1152, channels);
    *frames = 1152;
    if (info)
        info->flags = ((result[5] || d->changed) ? OPENDSP_MP3_INFO_CHANGED : 0) |
                      (d->discontinuity ? OPENDSP_MP3_INFO_DISCONTINUITY : 0);
    d->changed = d->discontinuity = false;
    return 0;
}

void aac_backend_reset(aac_dsp_backend *d)
{
    if (!d)
        return;
    d->reset_requested = true;
    if (!opendsp_running || sleeping || d->epoch != epoch) {
        d->connected = false;
        return;
    }
    u16 session = new_session(), args[3] = {d->session, session, d->preserve_output};
    if (R_FAILED(opendsp_request(codec_command(d, AAC_CMD_RESET), args, d->codec == 2 ? 3 : 2, NULL, 0))) {
        d->faulted = true;
        return;
    }
    d->session = session;
    d->sequence = 0;
    d->changed = true;
    d->discontinuity = false;
    d->reset_requested = false;
    d->faulted = false;
    d->input_write = d->stream_completed = 0;
    if (d->codec == 2)
        stream_metadata_init(d);
    else {
        memset(d->memory, 0, AAC_CONTROL_BYTES);
        u16 *control = (u16 *)d->memory;
        control[0] = d->codec ? MP3_MAGIC : AAC_MAGIC;
        control[1] = d->codec ? MP3_ABI : AAC_ABI;
        control[2] = session;
        DSP_FlushDataCache(d->memory, AAC_CONTROL_BYTES);
    }
}

static Result stream_valid(aac_dsp_backend *d)
{
    if (!d || d->codec != 2)
        return OPENDSP_MP3_ERR_INVALID_ARG;
    if (!opendsp_running || sleeping || !d->connected || d->epoch != epoch)
        return OPENDSP_MP3_ERR_BACKEND;
    if (d->faulted)
        return OPENDSP_MP3_ERR_TIMEOUT;
    return 0;
}
Result mp3_backend_stream_poll(aac_dsp_backend *d, mp3_stream_progress *p)
{
    Result r = stream_valid(d);
    if (R_FAILED(r))
        return r;
    volatile u16 *status = d->metadata + MP3_STREAM_STATUS / 2;
    __dsb();
    memset(p, 0, sizeof(*p));
    if (status[1] != d->session)
        return 0; /* new session has not published yet */
    u16 sequence = status[0];
    p->read = status[7];
    p->flags = status[2];
    p->error = status[3];
    p->completed = status[4];
    p->rate = status[5];
    p->channels = status[6];
    p->skipped = (u32)status[10] << 16 | status[11];
    __dsb();
    if (sequence != status[15] || sequence != status[0])
        return OPENDSP_MP3_ERR_BUSY;
    if ((u16)(d->input_write - p->read) > MP3_STREAM_INPUT_BYTES ||
        (u16)(d->sequence - p->completed) > MP3_STREAM_SLOTS ||
        ((p->flags & MP3_STREAM_HEADER) && (p->rate > 2 || p->channels < 1 || p->channels > 2)))
        return OPENDSP_MP3_ERR_BACKEND;
    d->stream_completed = p->completed;
    return 0;
}
u8 *mp3_backend_stream_input(aac_dsp_backend *d, u16 read, size_t *available)
{
    unsigned used = (u16)(d->input_write - read), at = d->input_write & (MP3_STREAM_INPUT_BYTES - 1);
    if (used > MP3_STREAM_INPUT_BYTES) {
        *available = 0;
        return NULL;
    }
    *available = MP3_STREAM_INPUT_BYTES - used;
    if (*available > MP3_STREAM_INPUT_BYTES - at)
        *available = MP3_STREAM_INPUT_BYTES - at;
    return d->memory + MP3_STREAM_INPUT + at;
}
void mp3_backend_stream_written(aac_dsp_backend *d, size_t bytes, bool eof)
{
    unsigned at = d->input_write & (MP3_STREAM_INPUT_BYTES - 1), begin = at & ~31u;
    if (bytes)
        DSP_FlushDataCache(d->memory + MP3_STREAM_INPUT + begin, ((at + bytes + 31) & ~31u) - begin);
    d->input_write += (u16)bytes;
    if (eof)
        d->input_eof = true;
}
Result mp3_backend_stream_submit(aac_dsp_backend *d, u16 slot, u16 *ticket)
{
    Result r = stream_valid(d);
    if (R_FAILED(r))
        return r;
    if (slot >= d->stream_count || (u16)(d->sequence - d->stream_completed) >= MP3_STREAM_SLOTS)
        return OPENDSP_MP3_ERR_BUSY;
    volatile u16 *descriptor = d->metadata + MP3_STREAM_DESCRIPTORS / 2 + (d->sequence & 7) * 32;
    descriptor[0] = slot;
    descriptor[1] = d->session;
    *ticket = ++d->sequence;
    return 0;
}
Result mp3_backend_stream_result(aac_dsp_backend *d, u16 ticket, u16 *frames)
{
    Result r = stream_valid(d);
    if (R_FAILED(r))
        return r;
    if ((u16)(d->stream_completed - ticket) >= MP3_STREAM_SLOTS)
        return OPENDSP_MP3_ERR_BUSY;
    volatile u16 *record = d->metadata + MP3_STREAM_RESULTS / 2 + ((ticket - 1) & 7) * 32;
    __dsb();
    if (record[0] != ticket || record[8] != d->session || record[9] > d->stream_frames)
        return OPENDSP_MP3_ERR_BACKEND;
    *frames = record[9];
    return 0;
}
void mp3_backend_stream_commit(aac_dsp_backend *d)
{
    u16 sequence = (d->control_sequence + 1) & 0x7fff;
    d->control_sequence = sequence;
    d->metadata[31] = sequence ^ 0x8000;
    __dsb();
    d->metadata[3] = d->input_write;
    d->metadata[4] = d->sequence;
    d->metadata[5] = d->input_eof;
    d->metadata[6] = d->played_ack;
    __dsb();
    d->metadata[31] = sequence;
    __dsb();
    DSP_SetSemaphore(MP3_STREAM_DOORBELL);
}
void mp3_backend_stream_played(aac_dsp_backend *d, u16 count)
{
    d->played_ack = count;
}
void *mp3_backend_stream_pcm(aac_dsp_backend *d, u32 *stride)
{
    *stride = (d->stream_frames * 4u + 31) & ~31u;
    return d->memory + MP3_STREAM_PCM;
}
u32 mp3_backend_stream_overhead(void)
{
    return MP3_STREAM_PCM + sizeof(aac_dsp_backend);
}
Result mp3_backend_stream_reset(aac_dsp_backend *d, bool preserve_output)
{
    if (!d || d->codec != 2 || !opendsp_running || sleeping)
        return OPENDSP_MP3_ERR_BACKEND;
    LightLock_Lock(&owner_lock);
    d->preserve_output = preserve_output;
    Result r;
    if (!d->connected || d->epoch != epoch) {
        d->reset_requested = true;
        r = connect_stream(d);
    } else {
        aac_backend_reset(d);
        r = d->faulted ? OPENDSP_MP3_ERR_BACKEND : 0;
    }
    LightLock_Unlock(&owner_lock);
    return r;
}
void mp3_backend_stream_stop(aac_dsp_backend *d)
{
    if (!d || d->codec != 2 || d->stopped)
        return;
    u16 args[1] = {d->session};
    if (!dspIsComponentLoaded() || d->generation != current_generation() ||
        (!sleeping && R_SUCCEEDED(opendsp_request(MP3_CMD_CLOSE, args, 1, NULL, 0))))
        d->stopped = true;
}

void aac_backend_close(aac_dsp_backend *d)
{
    if (!d)
        return;
    LightLock_Lock(&owner_lock);
    bool safe = !dspIsComponentLoaded() || (!sleeping && d->generation != current_generation()) || d->stopped;
    u16 args[1] = {d->session};
    if (!safe && !sleeping)
        safe = R_SUCCEEDED(opendsp_request(codec_command(d, AAC_CMD_CLOSE), args, 1, NULL, 0));
    if (safe) {
        linearFree(d->memory);
    } else {
        // Keep the arena alive until DSP shutdown confirms DMA has stopped.
        quarantined = d->memory;
        quarantine_epoch = current_generation();
    }
    if (owner == d)
        owner = NULL;
    free(d);
    LightLock_Unlock(&owner_lock);
}

Result aac_backend_stats(aac_dsp_backend *d, opendspAacStats *stats)
{
    LightLock_Lock(&owner_lock);
    u16 args[2] = {d->session, 0}, r[16], slice[2];
    Result rc = opendsp_request(codec_command(d, AAC_CMD_STATS), args, 2, r, 16);
    if (R_FAILED(rc))
        goto done;
    stats->frames = (u32)r[0] << 16 | r[1];
    stats->errors = (u32)r[2] << 16 | r[3];
    stats->yields = (u32)r[4] << 16 | r[5];
    stats->stack_words = r[6];
    stats->last_ticks = (u32)r[8] << 16 | r[9];
    stats->worst_ticks = (u32)r[10] << 16 | r[11];
    stats->last_latency_ticks = (u32)r[12] << 16 | r[13];
    stats->worst_latency_ticks = (u32)r[14] << 16 | r[15];
    args[1] = 1;
    rc = opendsp_request(codec_command(d, AAC_CMD_STATS), args, 2, slice, 2);
    if (R_FAILED(rc))
        goto done;
    stats->worst_slice_ticks = (u32)slice[0] << 16 | slice[1];
done:
    LightLock_Unlock(&owner_lock);
    return rc;
}

Result mp3_backend_stages(aac_dsp_backend *d, u32 ticks[6])
{
    LightLock_Lock(&owner_lock);
    u16 args[2] = {d->session, 2}, reply[12];
    Result rc = opendsp_request(MP3_CMD_STATS, args, 2, reply, 12);
    if (R_SUCCEEDED(rc))
        for (unsigned i = 0; i < 6; ++i)
            ticks[i] = (u32)reply[2 * i] << 16 | reply[2 * i + 1];
    LightLock_Unlock(&owner_lock);
    return rc;
}
