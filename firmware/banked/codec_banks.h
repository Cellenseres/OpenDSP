// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <stdint.h>
typedef struct { uint32_t offset, bytes; } codec_bank_source;
typedef struct { uint32_t word_address, offset, bytes; } codec_data_source;
#define CODEC_PROGRAM_WINDOW 0x11000u
#define CODEC_BANK_COUNT 0x8u
#define CODEC_DATA_COUNT 0x6u
#define CODEC_AAC_BANK 0x4u
#define CODEC_MP3_BANK 0x5u
#define CODEC_PAYLOAD_BYTES 0x70b00u
#define CODEC_IMAGE_TAG0 0x474f53a7u
#define CODEC_IMAGE_TAG1 0xaab6807cu
#define CODEC_IMAGE_SHA256 "44d6c331b1fa391613dcfe4f8b748b2fc9681a712dcb77e22568f0d3814da5c4"
static const codec_bank_source codec_banks[CODEC_BANK_COUNT] = {
    {0x0u, 0x35cu},
    {0x380u, 0x7290u},
    {0x7680u, 0x1a07cu},
    {0x21700u, 0x1e8u},
    {0x21900u, 0x7fe4u},
    {0x29900u, 0x112a0u},
    {0x3ac00u, 0x17072u},
    {0x51c80u, 0x177b4u},
};
static const codec_data_source codec_data[CODEC_DATA_COUNT] = {
    {0x12d80u, 0x69480u, 0xd78u},
    {0x12d80u, 0x6a200u, 0x3094u},
    {0x1f600u, 0x6d300u, 0x708u},
    {0x18000u, 0x6da80u, 0xffcu},
    {0x1b080u, 0x6ea80u, 0x1f00u},
    {0x1ff00u, 0x70980u, 0x160u},
};
#define CODEC_DATA_PROFILES 1
static const uint16_t codec_data_profiles[CODEC_DATA_COUNT] = {
    2u,
    0u,
    0u,
    0u,
    0u,
    0u,
};
