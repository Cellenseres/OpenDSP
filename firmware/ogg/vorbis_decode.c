// SPDX-License-Identifier: BSD-2-Clause
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "ogg.h"
#include "ivorbiscodec.h"
#include "codec_internal.h"
#include "decoder.h"
#include "packet.h"
#include "task.h"
#include "vorbis_decode.h"

static vorbis_info stream_info;
static vorbis_comment stream_comments;
static vorbis_dsp_state *decoder;
static ogg_input input;
static u16 heap_next, headers, channels, failed;
static ogg_int64_t packet_number;

u16 overlay_enter(u16 bank);
void ogg_opus_poll(void);

static u32 read_u32(const volatile u16 *words)
{
    return ((u32)words[0] << 16) | words[1];
}

static void memory_failure(void) __attribute__((noreturn));
static void memory_failure(void)
{
    failed = 1;
    for (;;) {
        ((vu16 *)OGG_DECODE_WORD)[OGG_TASK_ERROR] = 2;
        ogg_task_yield();
    }
}

void *tremor_malloc(size_t bytes)
{
    u32 words, address = heap_next;
    if (bytes > 30712) memory_failure();
    words = ((bytes + 3) & ~(size_t)3) / 2;
    if (address < 0xc002u || address + words > 0xfbffu) memory_failure();
    heap_next = address + words;
    return (void *)(uintptr_t)address;
}

void *tremor_calloc(size_t count, size_t bytes)
{
    void *memory;
    if (bytes && count > 30712u / bytes) memory_failure();
    memory = tremor_malloc(count * bytes);
    memset(memory, 0, count * bytes);
    return memory;
}

// Opus stack bottom and PCM tail, idle under Vorbis.
#define WORK_BASE 0x4620u
#define WORK_GUARD 0x5620u
#define RIGHT_BASE 0x2050u
#define WORK_WORDS 0x800u
#define RIGHT_WORDS 0x400u

static int static_work(uintptr_t address)
{
    return (address >= WORK_BASE && address < WORK_GUARD) ||
           (address >= RIGHT_BASE && address < RIGHT_BASE + 2 * RIGHT_WORDS);
}

void *tremor_work_buffer(int kind, int channel, size_t bytes)
{
    u16 words = kind ? RIGHT_WORDS : WORK_WORDS;
    u16 index;
    void *memory;
    // Long blocks keep the heap path.
    if (bytes > words * 2u || channel > 1) return tremor_calloc(1, bytes);
    memory = (void *)(uintptr_t)((kind ? RIGHT_BASE : WORK_BASE) + channel * words);
    memset(memory, 0, bytes);
    for (index = 0; index < 16; ++index)
        ((vu16 *)WORK_GUARD)[index] = 0x5aa5;
    return memory;
}

void tremor_free(void *memory)
{
    uintptr_t address = (uintptr_t)memory;
    if (static_work(address)) return;
    if (address && (address < 0xc002u || address > heap_next)) memory_failure();
}

void *tremor_realloc(void *memory, size_t bytes)
{
    (void)memory;
    (void)bytes;
    memory_failure();
}

void tremor_decode_yield(void)
{
    ogg_opus_poll();
}

int vorbis_dsp_session_init(u16 requested_channels)
{
    if (requested_channels < 1 || requested_channels > 2) return OV_EINVAL;
    overlay_enter(CODEC_VORBIS_HEADER_BANK);
    memset(&stream_info, 0, sizeof(stream_info));
    memset(&stream_comments, 0, sizeof(stream_comments));
    decoder = 0;
    channels = requested_channels;
    heap_next = 0xc002;
    headers = failed = 0;
    packet_number = 0;
    *(vu16 *)0xc000 = 0x4567;
    *(vu16 *)0xfbff = 0x7654;
    vorbis_info_init(&stream_info);
    vorbis_comment_init(&stream_comments);
    return 0;
}

int vorbis_dsp_session_restart(void)
{
    if (headers != 3 || !decoder || failed) return OV_EINVAL;
    overlay_enter(CODEC_VORBIS_DECODE_BANK);
    packet_number = 3;
    return vorbis_dsp_restart(decoder);
}

int vorbis_dsp_packet_decode(vu16 *command, s16 *pcm)
{
    ogg_buffer buffer;
    ogg_reference reference;
    ogg_packet packet;
    u16 bytes = command[OGG_TASK_BYTES], capacity = command[OGG_TASK_FRAMES];
    int result, available;
    if (!bytes || !channels || failed || capacity > 4096) return OV_EBADPACKET;
    overlay_enter(headers < 3 ? CODEC_VORBIS_HEADER_BANK : CODEC_VORBIS_DECODE_BANK);
    if (ogg_input_open(&input, read_u32(command + OGG_TASK_PACKET), bytes,
                       read_u32(command + OGG_TASK_INPUT_BEGIN), read_u32(command + OGG_TASK_INPUT_END)))
        return OV_EBADPACKET;
    memset(&buffer, 0, sizeof(buffer));
    memset(&reference, 0, sizeof(reference));
    memset(&packet, 0, sizeof(packet));
    buffer.size = bytes;
    buffer.refcount = 1;
    buffer.read_byte = ogg_input_byte;
    buffer.context = &input;
    reference.buffer = &buffer;
    reference.length = bytes;
    packet.packet = &reference;
    packet.bytes = bytes;
    packet.b_o_s = packet_number == 0;
    packet.packetno = packet_number++;
    packet.e_o_s = command[OGG_TASK_FEC];
    packet.granulepos = (ogg_int64_t)(((unsigned long long)read_u32(command + OGG_TASK_GRANULE) << 32) |
                                    read_u32(command + OGG_TASK_GRANULE + 2));
    if (headers < 3) {
        result = vorbis_dsp_headerin(&stream_info, &stream_comments, &packet);
        if (result) return result;
        if (stream_info.channels != channels) return OV_EBADHEADER;
        if (++headers == 3) {
            decoder = vorbis_dsp_create(&stream_info);
            if (!decoder) return OV_EFAULT;
        }
        return 0;
    }
    if (!decoder || !capacity) return OV_EINVAL;
    result = vorbis_dsp_synthesis(decoder, &packet, 1);
    if (result || input.failed) return result ? result : OV_EBADPACKET;
    available = vorbis_dsp_pcmout(decoder, 0, 8192);
    if (available < 0 || available > capacity) return OV_EINVAL;
    result = vorbis_dsp_pcmout(decoder, pcm, available);
    if (result != available || vorbis_dsp_read(decoder, available)) return OV_EFAULT;
    return result;
}

u16 vorbis_dsp_session_status(void)
{
    return failed || *(vu16 *)0xc000 != 0x4567 || *(vu16 *)0xfbff != 0x7654 ||
           (decoder && ((vu16 *)WORK_GUARD)[0] != 0x5aa5);
}
