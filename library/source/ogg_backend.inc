// SPDX-License-Identifier: BSD-2-Clause
#ifndef OPENDSP_OGG_WAIT_MS
#define OPENDSP_OGG_WAIT_MS 2000
#endif

bool ogg_backend_context_valid(const aac_dsp_backend *decoder)
{
    return decoder && decoder->codec >= 3 && decoder->codec <= 4 && opendsp_running && !sleeping_now() && dspIsComponentLoaded() &&
           decoder->epoch == power_epoch() && decoder->generation == current_generation() &&
           decoder->wakeups == __atomic_load_n(&wakeups, __ATOMIC_ACQUIRE);
}

// A wakeup ended the session; reset reopens it.
bool ogg_backend_recoverable(const aac_dsp_backend *decoder)
{
    return decoder && decoder->codec >= 3 && decoder->codec <= 4 && opendsp_running && !sleeping_now() &&
           dspIsComponentLoaded() && !ogg_backend_context_valid(decoder);
}

#define OGG_EVENT_WAIT_NS 20000000LL

static bool ogg_cancelled(const u32 *generation, u32 expected)
{
    return generation && __atomic_load_n(generation, __ATOMIC_ACQUIRE) != expected;
}

static Result ogg_wait_result(aac_dsp_backend *decoder, const u32 *generation, u32 expected, u16 reply[8])
{
    u64 deadline = svcGetSystemTick() + (u64)SYSCLOCK_ARM11 * OPENDSP_OGG_WAIT_MS / 1000;
    u16 args[1] = {decoder->session};
    bool events = decoder->ogg_events && opendsp_codec_events_active();
    for (;;) {
        if (!ogg_backend_context_valid(decoder)) return OPENDSP_AUDIO_ERR_CONTEXT;
        bool done = events && opendsp_mailbox_peek(OGG_DONE_WORD) == decoder->sequence;
        if (done) {
            // Published before the ticket.
            reply[0] = decoder->sequence;
            for (u16 i = 0; i < 4; ++i)
                reply[1 + i] = opendsp_mailbox_peek(OGG_DONE_STATUS + i);
        }
        if (!events || done) {
            Result result = done ? 0 : opendsp_request(OGG_CMD_STATUS, args, 1, reply, 8);
            if (R_FAILED(result)) return result;
            if (reply[0] != decoder->sequence || reply[1] == OGG_STATE_FAILED) return OPENDSP_ERR_ADDRESS;
            // A cancelled job still runs to the end.
            if (reply[1] == OGG_STATE_DONE)
                return ogg_cancelled(generation, expected) ? OPENDSP_AUDIO_ERR_STATE : 0;
            if (reply[1] != OGG_STATE_BUSY) return OPENDSP_ERR_ARGUMENT;
        }
        u64 now = svcGetSystemTick();
        if (now >= deadline) return OPENDSP_ERR_TIMEOUT;
        if (events)
            // Bounded, in case an event is lost.
            opendsp_codec_wait(OGG_EVENT_WAIT_NS);
        else
            svcSleepThread(500000);
    }
}

static Result ogg_submit(aac_dsp_backend *decoder, u16 command, u16 bytes, u16 frames, s16 gain,
                         const u32 *generation, u32 expected)
{
    u16 reply[8];
    if (!ogg_backend_context_valid(decoder)) return OPENDSP_AUDIO_ERR_CONTEXT;
    if (!++decoder->sequence) ++decoder->sequence;
    u16 args[6] = {decoder->session, decoder->sequence, bytes, frames, 0, (u16)gain};
    Result result = opendsp_request(command, args, 6, NULL, 0);
    if (R_SUCCEEDED(result)) result = ogg_wait_result(decoder, generation, expected, reply);
    if (result == OPENDSP_AUDIO_ERR_STATE) {
        // Vorbis reconnects; Opus takes the INIT reset.
        if (decoder->codec == 4) decoder->faulted = true;
        return result;
    }
    if (R_FAILED(result)) {
        opendsp_codec_error(decoder->codec == 4 ? "Vorbis control" :
                            command == OGG_CMD_INIT ? "Opus init" : "Opus decode", result);
        decoder->faulted = true;
        return result;
    }
    if ((s16)reply[2] < 0 || reply[3] != frames || reply[4] != decoder->ogg_channels)
        return OPENDSP_AUDIO_ERR_FORMAT;
    return 0;
}

static Result ogg_connect_stream(aac_dsp_backend *decoder, u32 capabilities)
{
    const u32 required = CODEC_CAP_BANKS | (decoder->codec == 4 ? OGG_CAP_VORBIS_DSP : OGG_CAP_OPUS_DSP);
    if ((capabilities & required) != required) return OPENDSP_AAC_ERR_BACKEND;
    decoder->epoch = power_epoch();
    decoder->generation = current_generation();
    decoder->wakeups = __atomic_load_n(&wakeups, __ATOMIC_ACQUIRE);
    decoder->ogg_events = false;
    if ((capabilities & (MP3_CAP_STREAM | OGG_CAP_DONE_WORD)) == (MP3_CAP_STREAM | OGG_CAP_DONE_WORD)) {
        Result events = opendsp_codec_events_start();
        opendsp_codec_error("DSP notification setup", events);
        decoder->ogg_events = R_SUCCEEDED(events);
    }
    if (!decoder->banks.token) decoder->banks.token = new_session();
    Result result = codec_banks_register(&decoder->banks, decoder->memory, capabilities);
    if (R_FAILED(result)) return result;
    u32 begin = osConvertVirtToPhys(decoder->memory), end = begin + shared_bytes(decoder), output = begin + OGG_INPUT_BYTES;
    decoder->session = new_session();
    decoder->sequence = 0;
    u16 args[12] = {OGG_ABI, decoder->session, decoder->banks.token,
                    decoder->ogg_channels | (decoder->codec == 4 ? OGG_CODEC_VORBIS : 0),
                    begin >> 16, begin, end >> 16, end, begin >> 16, begin, output >> 16, output};
    decoder->connected = true;
    result = opendsp_request(OGG_CMD_OPEN, args, 12, NULL, 0);
    opendsp_codec_error(decoder->codec == 4 ? "Vorbis session open" : "Opus session open", result);
    if (R_FAILED(result)) {
        if (result == OPENDSP_ERR_ARGUMENT || result == OPENDSP_ERR_ADDRESS || result == OPENDSP_ERR_UNSUPPORTED)
            decoder->connected = false;
        return result;
    }
    if (!ogg_backend_context_valid(decoder)) return OPENDSP_AUDIO_ERR_CONTEXT;
    result = codec_banks_upload_profile(decoder->codec == 4 ? 2 : 1);
    if (R_FAILED(result)) return result;
    result = ogg_submit(decoder, OGG_CMD_INIT, 0, 0, 0, NULL, 0);
    if (R_SUCCEEDED(result)) decoder->faulted = false;
    return result;
}

Result ogg_backend_open(aac_dsp_backend **out, u16 channels)
{
    if (!out || channels < 1 || channels > 2) return OPENDSP_AUDIO_ERR_ARGUMENT;
    return codec_backend_allocate(out, 3, channels, 0, false);
}

/* One healthy Opus session outlives its stream. */
static aac_dsp_backend *idle_opus;

static aac_dsp_backend *take_idle(void)
{
    LightLock_Lock(&owner_lock);
    aac_dsp_backend *decoder = idle_opus;
    idle_opus = NULL;
    LightLock_Unlock(&owner_lock);
    return decoder;
}

static void release_idle(void)
{
    aac_backend_close(take_idle());
}

void aac_backend_release_idle(void)
{
    if (setup) release_idle();
}

void ogg_backend_park(aac_dsp_backend *decoder)
{
    if (!decoder) return;
    LightLock_Lock(&owner_lock);
    bool keep = !idle_opus && decoder->codec == 3 && !decoder->faulted && ogg_backend_context_valid(decoder);
    if (keep) idle_opus = decoder;
    LightLock_Unlock(&owner_lock);
    if (!keep) aac_backend_close(decoder);
}

Result ogg_backend_reuse(aac_dsp_backend **out, u16 channels)
{
    if (!out || channels < 1 || channels > 2) return OPENDSP_AUDIO_ERR_ARGUMENT;
    aac_dsp_backend *decoder = setup ? take_idle() : NULL;
    if (decoder && decoder->ogg_channels == channels && !decoder->faulted && ogg_backend_context_valid(decoder) &&
        R_SUCCEEDED(ogg_backend_reset(decoder, NULL, 0))) {
        *out = decoder;
        return 0;
    }
    aac_backend_close(decoder);
    return ogg_backend_open(out, channels);
}

Result vorbis_backend_open(aac_dsp_backend **out, u16 channels)
{
    if (!out || channels < 1 || channels > 2) return OPENDSP_AUDIO_ERR_ARGUMENT;
    return codec_backend_allocate(out, 4, channels, 0, false);
}

Result ogg_backend_reset(aac_dsp_backend *decoder, const u32 *generation, u32 expected)
{
    if (!decoder || decoder->codec < 3 || decoder->codec > 4) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&owner_lock);
    Result result;
    bool lost = ogg_backend_recoverable(decoder);
    if (!lost && !ogg_backend_context_valid(decoder)) result = OPENDSP_AUDIO_ERR_CONTEXT;
    else if (decoder->faulted || lost) {
        u16 args[1] = {decoder->session};
        result = opendsp_request(OGG_CMD_CLOSE, args, 1, NULL, 0);
        // A reloaded DSP no longer knows the session.
        if (lost) result = 0;
        if (R_SUCCEEDED(result)) {
            decoder->connected = false;
            u32 capabilities = 0;
            result = opendspGetCapabilities(&capabilities);
            if (R_SUCCEEDED(result)) result = ogg_connect_stream(decoder, capabilities);
        }
    } else result = ogg_submit(decoder, OGG_CMD_INIT, 0, 0, 0, generation, expected);
    LightLock_Unlock(&owner_lock);
    return result;
}

Result vorbis_backend_restart(aac_dsp_backend *decoder, const u32 *generation, u32 expected)
{
    if (!decoder || decoder->codec != 4) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&owner_lock);
    Result result = OPENDSP_AUDIO_ERR_CONTEXT;
    if (ogg_backend_context_valid(decoder) && !decoder->faulted)
        result = ogg_submit(decoder, OGG_CMD_RESET, 0, 0, 0, generation, expected);
    LightLock_Unlock(&owner_lock);
    return result;
}

static Result vorbis_send(aac_dsp_backend *decoder, const void *packet, size_t bytes, s64 granule, bool eos,
                          u16 capacity)
{
    memcpy(decoder->memory, packet, bytes);
    DSP_FlushDataCache(decoder->memory, (bytes + 31) & ~31u);
    __dsb();
    if (!++decoder->sequence) ++decoder->sequence;
    u64 raw_granule = (u64)granule;
    u16 args[10] = {decoder->session, decoder->sequence, bytes, capacity, eos, 0,
                    raw_granule >> 48, raw_granule >> 32, raw_granule >> 16, raw_granule};
    return opendsp_request(OGG_CMD_DECODE, args, 10, NULL, 0);
}

static Result vorbis_receive(aac_dsp_backend *decoder, Result result, s16 *pcm, u16 capacity, u16 *frames,
                             const u32 *generation, u32 expected)
{
    u16 reply[8];
    if (R_SUCCEEDED(result)) result = ogg_wait_result(decoder, generation, expected, reply);
    if (R_FAILED(result)) {
        if (result != OPENDSP_AUDIO_ERR_STATE) opendsp_codec_error("Vorbis decode", result);
        decoder->faulted = true;
        return result;
    }
    if ((s16)reply[2] < 0 || reply[3] != reply[2] || reply[3] > capacity || reply[4] != decoder->ogg_channels) {
        decoder->faulted = true;
        return OPENDSP_AUDIO_ERR_FORMAT;
    }
    if (reply[3]) {
        u32 count = reply[3] * decoder->ogg_channels * 2u;
        DSP_InvalidateDataCache(decoder->memory + OGG_INPUT_BYTES, (count + 31) & ~31u);
        __dsb();
        memcpy(pcm, decoder->memory + OGG_INPUT_BYTES, count);
    }
    *frames = reply[3];
    return 0;
}

Result vorbis_backend_packet(aac_dsp_backend *decoder, const void *packet, size_t bytes, s64 granule, bool eos,
                            s16 *pcm, u16 capacity, u16 *frames, const u32 *generation, u32 expected)
{
    if (frames) *frames = 0;
    if (!decoder || decoder->codec != 4 || !packet || !bytes || bytes >= OGG_INPUT_BYTES ||
        !pcm || !frames || !capacity || capacity > 4096) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&owner_lock);
    Result result = OPENDSP_AUDIO_ERR_CONTEXT;
    if (ogg_backend_context_valid(decoder) && !decoder->faulted)
        result = vorbis_receive(decoder, vorbis_send(decoder, packet, bytes, granule, eos, capacity),
                                pcm, capacity, frames, generation, expected);
    if (R_SUCCEEDED(result))
        stats_serve(decoder);
    LightLock_Unlock(&owner_lock);
    return result;
}

Result vorbis_backend_send(aac_dsp_backend *decoder, const void *packet, size_t bytes)
{
    if (!decoder || decoder->codec != 4 || !packet || !bytes || bytes >= OGG_INPUT_BYTES)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&owner_lock);
    Result result = OPENDSP_AUDIO_ERR_CONTEXT;
    if (ogg_backend_context_valid(decoder) && !decoder->faulted) {
        result = vorbis_send(decoder, packet, bytes, -1, false, 1);
        if (R_FAILED(result)) {
            opendsp_codec_error("Vorbis decode", result);
            decoder->faulted = true;
        }
    }
    LightLock_Unlock(&owner_lock);
    return result;
}

Result vorbis_backend_receive(aac_dsp_backend *decoder, s16 *pcm, u16 capacity, u16 *frames,
                              const u32 *generation, u32 expected)
{
    if (frames) *frames = 0;
    if (!decoder || decoder->codec != 4 || !pcm || !frames || !capacity || capacity > 4096)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&owner_lock);
    Result result = OPENDSP_AUDIO_ERR_CONTEXT;
    if (ogg_backend_context_valid(decoder) && !decoder->faulted)
        result = vorbis_receive(decoder, 0, pcm, capacity, frames, generation, expected);
    if (R_SUCCEEDED(result))
        stats_serve(decoder);
    LightLock_Unlock(&owner_lock);
    return result;
}

Result ogg_backend_decode(aac_dsp_backend *decoder, const void *packet, size_t bytes, s16 *pcm,
                          u16 frames, s16 gain, const u32 *generation, u32 expected)
{
    if (!decoder || decoder->codec != 3 || !pcm || !frames || frames > OGG_PCM_FRAMES ||
        bytes >= OGG_INPUT_BYTES || (!packet && bytes)) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&owner_lock);
    Result result = OPENDSP_AUDIO_ERR_CONTEXT;
    if (!ogg_backend_context_valid(decoder) || decoder->faulted) goto done;
    if (bytes) {
        memcpy(decoder->memory, packet, bytes);
        DSP_FlushDataCache(decoder->memory, (bytes + 31) & ~31u);
        __dsb();
    }
    result = ogg_submit(decoder, OGG_CMD_DECODE, (u16)bytes, frames, gain, generation, expected);
    if (R_SUCCEEDED(result)) {
        u32 count = frames * decoder->ogg_channels * 2u;
        DSP_InvalidateDataCache(decoder->memory + OGG_INPUT_BYTES, (count + 31) & ~31u);
        __dsb();
        memcpy(pcm, decoder->memory + OGG_INPUT_BYTES, count);
        stats_serve(decoder);
    }
done:
    LightLock_Unlock(&owner_lock);
    return result;
}
