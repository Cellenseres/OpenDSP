// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "aac_backend.h"

Result ogg_backend_open(aac_dsp_backend **out, u16 channels);
/* Opus sessions: park on close, reuse on the next open. */
Result ogg_backend_reuse(aac_dsp_backend **out, u16 channels);
void ogg_backend_park(aac_dsp_backend *decoder);
Result vorbis_backend_open(aac_dsp_backend **out, u16 channels);
Result vorbis_backend_restart(aac_dsp_backend *decoder, const u32 *generation, u32 expected);
Result vorbis_backend_packet(aac_dsp_backend *decoder, const void *packet, size_t bytes, s64 granule, bool eos,
                            s16 *pcm, u16 capacity, u16 *frames, const u32 *generation, u32 expected);
/* Send now, collect the reply later. */
Result vorbis_backend_send(aac_dsp_backend *decoder, const void *packet, size_t bytes);
Result vorbis_backend_receive(aac_dsp_backend *decoder, s16 *pcm, u16 capacity, u16 *frames,
                              const u32 *generation, u32 expected);
bool ogg_backend_context_valid(const aac_dsp_backend *decoder);
/* True after a wakeup; ogg_backend_reset reconnects. */
bool ogg_backend_recoverable(const aac_dsp_backend *decoder);
Result ogg_backend_reset(aac_dsp_backend *decoder, const u32 *generation, u32 expected);
Result ogg_backend_decode(aac_dsp_backend *decoder, const void *packet, size_t bytes, s16 *pcm,
                          u16 frames, s16 gain, const u32 *generation, u32 expected);
