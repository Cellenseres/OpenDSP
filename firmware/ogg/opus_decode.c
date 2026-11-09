// SPDX-License-Identifier: BSD-2-Clause
#include "dma.h"
#include "ogg/decoder.h"
#include "ogg/task.h"
#include "ogg/opus_packet.h"
#include "ogg_protocol.h"
#ifdef OPENDSP_VORBIS_DSP
#include "ogg/vorbis_decode.h"
#endif
// Decoder IR uses 16-bit int.
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#define int short
#include "opus.h"

static ogg_opus_packet packet_input __attribute__((section(".opus_packet_state")));
static s16 pcm[11524] __attribute__((section(".decoder_pcm"), aligned(4)));
int opus_decoder_set_packet_duration(OpusDecoder *decoder, int duration);
void opus_decoder_get_packet_state(const OpusDecoder *decoder, opus_int32 *duration, opus_uint32 *range);
#undef int

static u32 read_u32(const volatile u16 *words)
{
    return ((u32)words[0] << 16) | words[1];
}

void ogg_opus_pause(void)
{
    ((vu16 *)OGG_DECODE_WORD)[OGG_TASK_ERROR] = dma_error();
    ogg_task_yield();
    ogg_opus_resume();
}

void ogg_code_yield(void)
{
    ogg_opus_pause();
}

static int decode_packet(OpusDecoder *decoder, vu16 *command, u16 channels)
{
    unsigned char *packet = (void *)OGG_PACKET_WORD;
    u16 bytes = command[OGG_TASK_BYTES], frames = command[OGG_TASK_FRAMES], fec = command[OGG_TASK_FEC];
    opus_int32 previous_duration;
    u16 index, decoded = 0;
    int result;
    if (!bytes) return opus_decode(decoder, 0, 0, pcm + 2, frames, fec);
    if (ogg_opus_packet_open(&packet_input, read_u32(command + OGG_TASK_PACKET), bytes,
                             read_u32(command + OGG_TASK_INPUT_BEGIN), read_u32(command + OGG_TASK_INPUT_END)))
        return OPUS_INVALID_PACKET;
    if (!fec && (u32)packet_input.samples * packet_input.frames > frames) return OPUS_BUFFER_TOO_SMALL;
    opus_decoder_get_packet_state(decoder, &previous_duration, 0);
    for (index = 0; index < packet_input.frames; ++index) {
        int length = ogg_opus_packet_frame(&packet_input, index, packet, OGG_PACKET_BYTES);
        if (length < 0) {
            opus_decoder_set_packet_duration(decoder, previous_duration);
            return OPUS_INTERNAL_ERROR;
        }
        result = opus_decode(decoder, packet, length, pcm + 2 + (u32)decoded * channels,
                             fec ? frames : packet_input.samples, fec);
        if (result < 0) {
            opus_decoder_set_packet_duration(decoder, previous_duration);
            return result;
        }
        decoded += (u16)result;
        if (fec) break;
    }
    if (opus_decoder_set_packet_duration(decoder, decoded)) return OPUS_INTERNAL_ERROR;
    return decoded;
}

static int write_pcm(vu16 *command, u16 frames, u16 channels)
{
    u32 address = read_u32(command + OGG_TASK_PCM), end = read_u32(command + OGG_TASK_PCM_END);
    u16 words = frames * channels, copied = 0;
#ifdef OPENDSP_VORBIS_DSP
    if (words & 1) pcm[2 + words++] = 0;
#endif
    if (address >= end || ((u32)words << 1) > end - address || (address & 31)) return -1;
    while (copied < words) {
        u16 count = words - copied;
        if (count > 128) count = 128;
        dma_write_buffer((const u16 *)pcm + 2 + copied, count, address + ((u32)copied << 1));
        if (dma_error()) return -1;
        copied += count;
        ogg_opus_pause();
    }
    return 0;
}

#ifdef OPENDSP_VORBIS_DSP
static int decode_vorbis(vu16 *command, u16 channels)
{
    u16 samples = (command[OGG_TASK_FRAMES] * channels + 1u) & ~1u;
    int result;
    pcm[0] = 0x1234;
    pcm[1] = 0x5678;
    pcm[samples + 2] = 0x2345;
    pcm[samples + 3] = 0x3456;
    result = vorbis_dsp_packet_decode(command, pcm + 2);
    if (result > command[OGG_TASK_FRAMES]) return -1;
    if (result > 0 && write_pcm(command, result, channels)) return -1;
    if (pcm[0] != 0x1234 || pcm[1] != 0x5678 || pcm[samples + 2] != 0x2345 || pcm[samples + 3] != 0x3456)
        return -1;
    return result;
}
#endif

void ogg_task_main(void)
{
    vu16 *command = (void *)OGG_DECODE_WORD;
    vu16 *state = (void *)0xc000;
    OpusDecoder *decoder = (void *)0xc002;
    u16 channels = 0, state_bytes = 0, index;
    for (index = 0; index < 32; ++index) {
        ((vu16 *)OGG_TASK_GUARD)[index] = 0xa55a;
        ((vu16 *)OGG_PCM_GUARD)[index] = 0xa55a;
    }
    irq_enable();
    ogg_opus_resume();
    for (;;) {
        int result = OPUS_BAD_ARG;
        u16 decoded = 0;
        opus_int32 duration = 0;
        opus_uint32 range = 0;
        while (command[OGG_TASK_STATUS] != OGG_TASK_RUNNING) ogg_opus_pause();
#ifdef OPENDSP_VORBIS_DSP
        if (command[OGG_TASK_CODEC] == OGG_CODEC_VORBIS) {
            channels = command[OGG_TASK_CHANNELS];
            if (command[OGG_TASK_OPERATION] == OGG_CMD_INIT)
                result = vorbis_dsp_session_init(channels);
            else if (command[OGG_TASK_OPERATION] == OGG_CMD_RESET)
                result = vorbis_dsp_session_restart();
            else if (command[OGG_TASK_OPERATION] == OGG_CMD_DECODE &&
                     command[OGG_TASK_FRAMES] && command[OGG_TASK_FRAMES] <= 4096 &&
                     channels >= 1 && channels <= 2 && command[OGG_TASK_FEC] <= 1) {
                result = decode_vorbis(command, channels);
                if (result >= 0) decoded = result;
            }
        } else
#endif
        if (command[OGG_TASK_OPERATION] == OGG_CMD_INIT) {
            channels = command[OGG_TASK_CHANNELS];
            state_bytes = opus_decoder_get_size(channels);
            if (state_bytes && !(state_bytes & 1) && state_bytes < (0xf600u - 0xc002u - 1) * 2u) {
                state[0] = 0x4567;
                state[2 + state_bytes / 2] = 0x7654;
                result = opus_decoder_init(decoder, 48000, channels);
            }
        } else if (command[OGG_TASK_OPERATION] == OGG_CMD_DECODE && state_bytes &&
                   command[OGG_TASK_FRAMES] && command[OGG_TASK_FRAMES] <= OGG_PCM_FRAMES &&
                   channels >= 1 && channels <= 2 && command[OGG_TASK_FEC] <= 1) {
            u16 samples = command[OGG_TASK_FRAMES] * channels;
            pcm[0] = 0x1234;
            pcm[1] = 0x5678;
            pcm[samples + 2] = 0x2345;
            pcm[samples + 3] = 0x3456;
            result = decode_packet(decoder, command, channels);
            if (result > 0) {
                decoded = (u16)result;
                if (decoded > command[OGG_TASK_FRAMES]) result = OPUS_INTERNAL_ERROR;
                else {
                    ogg_opus_gain(pcm + 2, decoded * channels, (s16)command[OGG_TASK_GAIN]);
                    if (write_pcm(command, decoded, channels)) result = OPUS_INTERNAL_ERROR;
                }
            }
            if (pcm[0] != 0x1234 || pcm[1] != 0x5678 || pcm[samples + 2] != 0x2345 || pcm[samples + 3] != 0x3456)
                result = OPUS_INTERNAL_ERROR;
        }
        if (state_bytes) opus_decoder_get_packet_state(decoder, &duration, &range);
        command[OGG_TASK_RESULT] = (u16)result;
        command[OGG_TASK_DECODED] = result > 0 ? decoded : 0;
        command[OGG_TASK_DURATION] = (u16)duration;
        command[OGG_TASK_RANGE] = (u16)(range >> 16);
        command[OGG_TASK_RANGE + 1] = (u16)range;
        command[OGG_TASK_ERROR] = dma_error();
#ifdef OPENDSP_VORBIS_DSP
        if (command[OGG_TASK_CODEC] == OGG_CODEC_VORBIS && vorbis_dsp_session_status())
            command[OGG_TASK_ERROR] = 1;
#endif
        if (state_bytes && (state[0] != 0x4567 || state[2 + state_bytes / 2] != 0x7654))
            command[OGG_TASK_ERROR] = 1;
        command[OGG_TASK_STATUS] = OGG_TASK_COMPLETE;
        ogg_opus_pause();
    }
}
