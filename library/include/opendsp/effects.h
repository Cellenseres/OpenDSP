// SPDX-License-Identifier: BSD-2-Clause
// DSP effects on the two aux buses.
#pragma once
#include <opendsp.h>

#ifdef __cplusplus
extern "C" {
#endif

// Voices send with ndspChnSetMix() gains 4-7 (bus 0) and 8-11 (bus 1).
// The effect output is fully wet, ndspAuxSetVolume() sets the return.
// On one bus, the delay runs first.

typedef struct {
    u32 struct_size;        // sizeof(opendspDelayConfig)
    float delay_ms;         // 5 to 975, in 4.9 ms steps
    float feedback;         // echo to echo ratio, 0 to 0.95
    float damping;          // high cut per repeat, 0 to 1
    float level;            // first echo gain, 0.01 to 1
} opendspDelayConfig;

// NULL config turns the delay off. Uses a linear memory buffer.
Result opendspDelaySet(unsigned bus, const opendspDelayConfig *config);

typedef struct {
    u32 struct_size;        // sizeof(opendspReverbConfig)
    float room_size;        // 0 small to 1 large
    float decay_time;       // seconds, 0.1 to 10
    float damping;          // high cut in the tail, 0 to 1
    float diffusion;        // echo density, 0 to 1
    float width;            // 0 mono to 1 wide
    float level;            // output gain, 0 to 1
} opendspReverbConfig;

// NULL turns the reverb off. About 13% DSP time per bus while it plays,
// nothing once the tail is gone.
Result opendspReverbSet(unsigned bus, const opendspReverbConfig *config);

#ifdef __cplusplus
}
#endif
