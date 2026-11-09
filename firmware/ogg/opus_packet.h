// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "packet.h"

typedef struct {
    ogg_input input;
    u16 offset[48], bytes[48];
    u16 frames, samples, toc, failed;
} ogg_opus_packet;

int ogg_opus_packet_open(ogg_opus_packet *packet, u32 address, u32 bytes, u32 arena_begin, u32 arena_end);
int ogg_opus_packet_frame(ogg_opus_packet *packet, u16 index, unsigned char *output, u16 capacity);
