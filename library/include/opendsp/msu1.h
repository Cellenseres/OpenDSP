// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <opendsp/audio.h>

#ifdef __cplusplus
extern "C" {
#endif

// MSU-1 audio and data device for emulators and recompilations.
// Tracks are <pack>-<N>.pcm, .opuz, .opus or .ogg; data is <pack>.msu.
typedef struct opendspMsu1 opendspMsu1;

typedef struct {
    u32 struct_size;
    s32 channel; // Reserved by opendspAudioInit, or AUTO
    u32 backend; // OPENDSP_AAC_BACKEND_*, for compressed tracks
} opendspMsu1Config;
#define OPENDSP_MSU1_CONFIG_INIT {sizeof(opendspMsu1Config), OPENDSP_AUDIO_CHANNEL_AUTO, OPENDSP_AAC_BACKEND_AUTO}

// Call these on the main thread, like opendspAudioOpen.
// pack is a path prefix or a directory with one pack.
Result opendspMsu1Open(opendspMsu1 **out, const char *pack, const opendspMsu1Config *config);
// BUSY keeps the handle; retry later.
Result opendspMsu1Close(opendspMsu1 **msu);
// Console reset: stops audio, volume 0.
void opendspMsu1Reset(opendspMsu1 *msu);
// $2000-$2007; only the low three address bits count.
// Selecting a track opens its file before returning.
u8 opendspMsu1Read(opendspMsu1 *msu, u32 address);
void opendspMsu1Write(opendspMsu1 *msu, u32 address, u8 value);
// Kept across tracks; the volume register scales it.
Result opendspMsu1SetMix(opendspMsu1 *msu, const float mix[12]);
opendspAudioStream *opendspMsu1GetStream(opendspMsu1 *msu); // NULL without a track
const char *opendspMsu1GetPack(const opendspMsu1 *msu);

#ifdef __cplusplus
}
#endif
