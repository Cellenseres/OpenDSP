// SPDX-License-Identifier: BSD-2-Clause
// Shared memory structures (word addresses, both banks).
#pragma once

#define SHM_DSP_STATUS        0x8800
#define SHM_DSP_DEBUG         0x8810
#define SHM_FINAL_SAMPLES     0x8940
#define SHM_SOURCE_STATUS     0x8A80
#define SHM_COMPRESSOR        0x8B10
#define SHM_DSP_CONFIG        0x9830
#define SHM_INTERMEDIATE_MIX  0x9892
#define SHM_SOURCE_CONFIG     0xA292
#define SHM_ADPCM_COEFS       0xAB92
#define SHM_UNKNOWN10         0xAD12
#define SHM_UNKNOWN11         0xAE12
#define SHM_UNKNOWN12         0xAED2
#define SHM_UNKNOWN13         0xB052
#define SHM_UNKNOWN14         0xB05C
#define SHM_FRAME_COUNTER     0xB06C
#define SHM_END               0xB06D

// order expected by ndsp
#define SHM_NUM_STRUCTS 15
#define SHM_STRUCT_TABLE { SHM_FRAME_COUNTER, SHM_SOURCE_CONFIG, SHM_SOURCE_STATUS, \
    SHM_ADPCM_COEFS, SHM_DSP_CONFIG, SHM_DSP_STATUS, SHM_FINAL_SAMPLES,             \
    SHM_INTERMEDIATE_MIX, SHM_COMPRESSOR, SHM_DSP_DEBUG, SHM_UNKNOWN10,              \
    SHM_UNKNOWN11, SHM_UNKNOWN12, SHM_UNKNOWN13, SHM_UNKNOWN14 }
