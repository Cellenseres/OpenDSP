// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds/types.h>
#include <3ds/result.h>
#include <opendsp/aac.h>
#include <opendsp/mp3.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPENDSP_AUDIO_CHANNEL_AUTO (-1)
#define OPENDSP_AUDIO_LOOP (1u << 0)
#define OPENDSP_AUDIO_PROFILE (1u << 1) // Optional MP3 stage timers; off for normal playback
#define OPENDSP_AUDIO_ERR_ARGUMENT MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_DSP, 0x2B0)
#define OPENDSP_AUDIO_ERR_STATE MAKERESULT(RL_USAGE, RS_INVALIDSTATE, RM_DSP, 0x2B1)
#define OPENDSP_AUDIO_ERR_BUSY MAKERESULT(RL_STATUS, RS_WOULDBLOCK, RM_DSP, 0x2B2)
#define OPENDSP_AUDIO_ERR_IO MAKERESULT(RL_PERMANENT, RS_NOTFOUND, RM_DSP, 0x2B3)
#define OPENDSP_AUDIO_ERR_FORMAT MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_DSP, 0x2B4)
#define OPENDSP_AUDIO_ERR_UNSUPPORTED MAKERESULT(RL_PERMANENT, RS_NOTSUPPORTED, RM_DSP, 0x2B5)
#define OPENDSP_AUDIO_ERR_MEMORY MAKERESULT(RL_PERMANENT, RS_OUTOFRESOURCE, RM_DSP, 0x2B6)
#define OPENDSP_AUDIO_ERR_TRUNCATED MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_DSP, 0x2B7)
#define OPENDSP_AUDIO_ERR_CONTEXT MAKERESULT(RL_TEMPORARY, RS_INVALIDSTATE, RM_DSP, 0x2B8)

typedef struct opendspAudioStream opendspAudioStream;
typedef opendspMp3Stats opendspAudioDecoderStats;
typedef enum {
    OPENDSP_AUDIO_STOPPED,
    OPENDSP_AUDIO_BUFFERING,
    OPENDSP_AUDIO_PLAYING,
    OPENDSP_AUDIO_PAUSED,
    OPENDSP_AUDIO_DRAINING,
    OPENDSP_AUDIO_FINISHED,
    OPENDSP_AUDIO_STOPPING,
    OPENDSP_AUDIO_ERROR
} opendspAudioState;
typedef enum { OPENDSP_AUDIO_WAV, OPENDSP_AUDIO_AAC, OPENDSP_AUDIO_MP3 } opendspAudioFormat;

typedef struct {
    u32 struct_size;
    s32 channel;
    u32 buffer_count;  // 0 = 4; otherwise 2..8 (MP3: 3..8)
    u32 buffer_frames; // 0 = WAV 2048 / AAC 1024 / MP3 1152; otherwise 256..16384
    u32 aac_backend;   // AAC/MP3 backend; MP3 AUTO requires DSP, ARM11 is explicit
    u32 flags;
} opendspAudioConfig;
#define OPENDSP_AUDIO_CONFIG_INIT                                                                            \
    {sizeof(opendspAudioConfig), OPENDSP_AUDIO_CHANNEL_AUTO, 0, 0, OPENDSP_AAC_BACKEND_AUTO, 0}

typedef struct {
    u32 struct_size;
    opendspAudioFormat format;
    u32 sample_rate;
    u16 channels, bits;
    u64 total_frames; // Compressed: known after a complete pass; includes encoder padding
    bool duration_known;
    u32 aac_flags; // Accumulated codec INFO_* flags (MP3 uses CHANGED/DISCONTINUITY)
} opendspAudioInfo;

typedef struct {
    u32 struct_size;
    u32 queued_buffers, queued_frames, capacity_frames;
    u32 min_queued_frames, peak_queued_frames;
    u64 queue_observations, queue_frames_sum;
    u32 underruns, rebuffers, loops, busy_retries;
    u32 linear_bytes, codec_bytes; // PCM allocation / approximate codec storage
    u64 input_bytes, decoded_frames, skipped_bytes;
    u64 read_ticks, decode_ticks, worker_ticks, peak_worker_ticks; // Elapsed ticks, including waits
} opendspAudioStats;

// Init/Open/Close/Exit run on the main thread, outside ndsp/DSP hooks.
// Commands and getters allow app threads; do not access a handle during Close.
// The Init mask reserves channels for the library.
Result opendspAudioInit(u32 channel_mask);
Result opendspAudioExit(void); // BUSY while handles remain open
// Open probes synchronously; failure sets *out = NULL. Close before replacing a stream.
Result opendspAudioOpen(opendspAudioStream **out, const char *path, const opendspAudioConfig *config);
// Playback commands are asynchronous. GetState/GetLastError report stream status.
Result opendspAudioPlay(opendspAudioStream *stream);
Result opendspAudioPause(opendspAudioStream *stream);
Result opendspAudioResume(opendspAudioStream *stream);
Result opendspAudioStop(opendspAudioStream *stream);
Result opendspAudioSeek(opendspAudioStream *stream, u64 frame); // AAC/MP3 support only 0
// On BUSY the handle and channel remain owned; retry. Success sets *stream = NULL.
Result opendspAudioClose(opendspAudioStream **stream);
Result opendspAudioSetLoop(opendspAudioStream *stream, bool loop);
Result opendspAudioSetVolume(opendspAudioStream *stream, float volume);     // 0..1, scales all mix gains
Result opendspAudioSetMix(opendspAudioStream *stream, const float mix[12]); // Each gain: -1..1
Result opendspAudioSetRate(opendspAudioStream *stream, float sample_rate);  // Playback Hz: 1..130912
// GetInfo/GetStats require output.struct_size = sizeof(output).
Result opendspAudioGetInfo(opendspAudioStream *stream, opendspAudioInfo *info);
Result opendspAudioGetStats(opendspAudioStream *stream, opendspAudioStats *stats);
// DSP-only decoder counters. MP3 stage timings require AUDIO_PROFILE; AAC leaves
// those six fields zero. This performs mailbox queries; sample at a low rate.
Result opendspAudioGetDecoderStats(opendspAudioStream *stream, opendspAudioDecoderStats *stats);
opendspAudioState opendspAudioGetState(opendspAudioStream *stream);
u64 opendspAudioGetPosition(opendspAudioStream *stream); // audible PCM frames, not file read-ahead
u32 opendspAudioGetBackend(opendspAudioStream *stream);  // AUTO for WAV
s32 opendspAudioGetChannel(opendspAudioStream *stream);
Result opendspAudioGetLastError(opendspAudioStream *stream);
// Optional NDSP callback forwarding for WAV/AAC. MP3 uses its own DSP pipe event.
void opendspAudioNotifyFrame(void);

#ifdef __cplusplus
}
#endif
