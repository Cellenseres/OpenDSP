// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <stdlib.h>
#include <string.h>
#include "aac_backend.h"
#include "internal.h"
#include "aac_protocol.h"
#include "opendsp.h"

#ifndef OPENDSP_AAC_WAIT_MS
#define OPENDSP_AAC_WAIT_MS 250
#endif

struct aac_dsp_backend {
    u8 *memory;
    u16 sequence, session;
    u32 epoch;
    bool connected, changed, faulted, ever_connected, discontinuity, reset_requested;
};
extern const u16 opendsp_aac_tables[AAC_TABLE_WORDS];
static LightLock owner_lock;
static bool setup, sleeping;
static volatile u32 epoch;
static u16 next_session;
static dspHookCookie hook;
static aac_dsp_backend *owner;
static void *quarantined;
static u32 quarantine_epoch;

static void power_hook(DSP_HookType type)
{
    if (type == DSPHOOK_ONSLEEP || type == DSPHOOK_ONCANCEL) {
        sleeping = true;
        epoch++;
    }
    if (type == DSPHOOK_ONWAKEUP) {
        sleeping = false;
        epoch++;
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
    if (quarantined) {
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

static Result connect_stream(aac_dsp_backend *d)
{
    u32 address, physical, caps = 0;
    u16 sum = 0, args[5];
    Result rc;
    if (!opendsp_running || sleeping)
        return OPENDSP_AAC_ERR_BACKEND;
    if (R_FAILED(opendspGetCapabilities(&caps)) || !(caps & AAC_CAP_DSP))
        return OPENDSP_AAC_ERR_BACKEND;
    rc = DSP_ConvertProcessAddressFromDspDram(AAC_TABLE_ADDRESS, &address);
    if (R_FAILED(rc))
        return OPENDSP_AAC_ERR_BACKEND;
    memcpy((void *)address, opendsp_aac_tables, AAC_TABLE_WORDS * 2);
    __dsb();
    for (unsigned i = 0; i < AAC_TABLE_WORDS; i++)
        sum += opendsp_aac_tables[i];
    memset(d->memory, 0, AAC_SHARED_BYTES);
    d->session = new_session();
    d->sequence = 0;
    d->changed = true;
    u16 *control = (u16 *)d->memory;
    control[0] = AAC_MAGIC;
    control[1] = AAC_ABI;
    control[2] = d->session;
    DSP_FlushDataCache(d->memory, AAC_SHARED_BYTES);
    physical = osConvertVirtToPhys(d->memory);
    args[0] = physical >> 16;
    args[1] = physical;
    args[2] = AAC_ABI;
    args[3] = d->session;
    args[4] = sum;
    d->connected = true; // a timed-out request may already have been accepted
    rc = opendsp_request(AAC_CMD_OPEN, args, 5, NULL, 0);
    if (R_FAILED(rc))
        return OPENDSP_AAC_ERR_BACKEND;
    d->epoch = epoch;
    d->faulted = false;
    d->discontinuity = d->ever_connected && !d->reset_requested;
    d->ever_connected = true;
    d->reset_requested = false;
    return 0;
}

Result aac_backend_open(aac_dsp_backend **out)
{
    Result rc;
    aac_dsp_backend *d;
    opendsp_aac_setup();
    LightLock_Lock(&owner_lock);
    if (quarantined && !sleeping && quarantine_epoch != epoch) {
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
    d->memory = linearMemAlign(AAC_SHARED_BYTES, 0x80);
    if (!d->memory) {
        free(d);
        LightLock_Unlock(&owner_lock);
        return OPENDSP_AAC_ERR_NO_MEMORY;
    }
    rc = connect_stream(d);
    if (R_FAILED(rc)) {
        u16 args[1] = {d->session};
        if (!d->connected || R_SUCCEEDED(opendsp_request(AAC_CMD_CLOSE, args, 1, NULL, 0))) {
            linearFree(d->memory);
        } else {
            quarantined = d->memory;
            quarantine_epoch = epoch;
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
    case 0: return 0;
    case 1: return OPENDSP_AAC_ERR_NEED_MORE_DATA;
    case 2: return OPENDSP_AAC_INFO_SKIPPED;
    case 3: return OPENDSP_AAC_ERR_CORRUPT;
    case 4: return OPENDSP_AAC_ERR_UNSUPPORTED;
    case AAC_RESOURCE_BUSY: return OPENDSP_AAC_ERR_BUSY;
    default: return OPENDSP_AAC_ERR_BACKEND;
    }
}

Result aac_backend_decode(aac_dsp_backend *d, const void *input, size_t bytes, size_t *used,
                          s16 *pcm, size_t capacity, size_t *frames, opendspAacInfo *info)
{
    static const u32 rates[12] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};
    const u8 *p = input;
    size_t skip = 0;
    unsigned len, channels, sf, blocks, header, slot;
    u16 *control, *result;
    Result rc;
    if (used)
        *used = 0;
    if (frames)
        *frames = 0;
    if (!d || !used || !frames || !pcm || (!p && bytes))
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
    if (sf < 3 || sf > 11 || channels < 1 || channels > 2 || (p[2]>>6) != 1 || blocks != 1 || len > AAC_INPUT_BYTES) {
        *used = len;
        return OPENDSP_AAC_ERR_UNSUPPORTED;
    }
    if (capacity < 1024)
        return OPENDSP_AAC_ERR_OUTPUT_TOO_SMALL;
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
        svcSleepThread(500000);
    }
    d->sequence++;
    rc = decode_status(result[1]);
    if (result[1] != AAC_RESOURCE_BUSY && result[1] != AAC_BAD_TRANSPORT)
        *used = len;
    if (R_FAILED(rc) || result[1])
        return rc;
    if (result[2] != channels || result[3] != sf)
        return OPENDSP_AAC_ERR_BACKEND;
    DSP_InvalidateDataCache(d->memory + AAC_PCM_OFFSET + slot * AAC_PCM_BYTES, AAC_PCM_BYTES);
    memcpy(pcm, d->memory + AAC_PCM_OFFSET + slot * AAC_PCM_BYTES, channels * 2048);
    *frames = 1024;
    if (info)
        info->flags = ((result[5] || d->changed) ? OPENDSP_AAC_INFO_CHANGED : 0) |
                      (result[4] ? OPENDSP_AAC_INFO_SBR_IGNORED : 0) |
                      (d->discontinuity ? OPENDSP_AAC_INFO_DISCONTINUITY : 0);
    d->discontinuity = false;
    d->changed = false;
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
    u16 session = new_session(), args[2] = {d->session, session};
    if (R_FAILED(opendsp_request(AAC_CMD_RESET, args, 2, NULL, 0))) {
        d->faulted = true;
        return;
    }
    d->session = session;
    d->sequence = 0;
    d->changed = true;
    d->discontinuity = false;
    d->reset_requested = false;
    d->faulted = false;
    memset(d->memory, 0, AAC_CONTROL_BYTES);
    u16 *control = (u16 *)d->memory;
    control[0] = AAC_MAGIC;
    control[1] = AAC_ABI;
    control[2] = session;
    DSP_FlushDataCache(d->memory, AAC_CONTROL_BYTES);
}

void aac_backend_close(aac_dsp_backend *d)
{
    if (!d)
        return;
    LightLock_Lock(&owner_lock);
    bool safe = !opendsp_running || (!sleeping && d->epoch != epoch);
    u16 args[1] = {d->session};
    if (!safe && !sleeping)
        safe = R_SUCCEEDED(opendsp_request(AAC_CMD_CLOSE, args, 1, NULL, 0));
    if (safe) {
        linearFree(d->memory);
    } else {
        // Keep the arena alive until DSP shutdown confirms DMA has stopped.
        quarantined = d->memory;
        quarantine_epoch = epoch;
    }
    if (owner == d)
        owner = NULL;
    free(d);
    LightLock_Unlock(&owner_lock);
}

Result aac_backend_stats(aac_dsp_backend *d, opendspAacStats *stats)
{
    u16 args[2] = {d->session, 0}, r[16], slice[2];
    Result rc = opendsp_request(AAC_CMD_STATS, args, 2, r, 16);
    if (R_FAILED(rc))
        return rc;
    stats->frames = (u32)r[0] << 16 | r[1];
    stats->errors = (u32)r[2] << 16 | r[3];
    stats->yields = (u32)r[4] << 16 | r[5];
    stats->stack_words = r[6];
    stats->last_ticks = (u32)r[8] << 16 | r[9];
    stats->worst_ticks = (u32)r[10] << 16 | r[11];
    stats->last_latency_ticks = (u32)r[12] << 16 | r[13];
    stats->worst_latency_ticks = (u32)r[14] << 16 | r[15];
    args[1] = 1;
    rc = opendsp_request(AAC_CMD_STATS, args, 2, slice, 2);
    if (R_FAILED(rc))
        return rc;
    stats->worst_slice_ticks = (u32)slice[0] << 16 | slice[1];
    return 0;
}
