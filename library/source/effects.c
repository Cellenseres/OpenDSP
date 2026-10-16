// SPDX-License-Identifier: BSD-2-Clause
// Delay and reverb control.
#include <math.h>
#include <string.h>
#include "opendsp.h"
#include "opendsp/effects.h"
#include "internal.h"
#include "extension.h"

#define SAMPLE_RATE      32728.0f
#define FRAME_SAMPLES    160
#define FRAME_BYTES      1280       // 4 channels of s16
#define DELAY_MAX_FRAMES 200

typedef struct { void *buffer; u32 frames; } delay_buffer;
static delay_buffer delay_buffers[2];

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }

static s16 to_q7(float v)
{
    float q = v * 128.0f;
    q = clampf(q, -32768.0f, 32767.0f);
    return (s16)(q < 0 ? q - 0.5f : q + 0.5f);
}

static void free_delay(unsigned bus)
{
    if (delay_buffers[bus].buffer)
        linearFree(delay_buffers[bus].buffer);
    delay_buffers[bus].buffer = NULL;
    delay_buffers[bus].frames = 0;
}

static Result send_delay(unsigned bus, u16 enable, u16 frames, s16 g, s16 a, s16 b, u32 address)
{
    // buffers are zeroed by the library
    u16 args[9] = { (u16)bus, enable, frames, (u16)g, (u16)a, (u16)b, (u16)(address >> 16), (u16)address, 1 };
    return opendsp_request(EXT_CMD_DELAY_SET, args, 9, NULL, 0);
}

Result opendspDelaySet(unsigned bus, const opendspDelayConfig *config)
{
    Result rc;
    u32 frames;
    float level, feedback, damping;
    s16 g, a, b;
    if (bus > 1 || (config && config->struct_size != sizeof(*config)))
        return OPENDSP_ERR_ARGUMENT;
    if (!config) {
        rc = send_delay(bus, 0, 0, 0, 0, 0, 0);
        if (R_SUCCEEDED(rc))
            free_delay(bus);
        return rc;
    }
    frames = (u32)(clampf(config->delay_ms, 0.0f, 1000.0f) * SAMPLE_RATE / (1000.0f * FRAME_SAMPLES) + 0.5f);
    frames = frames < 1 ? 1 : frames > DELAY_MAX_FRAMES ? DELAY_MAX_FRAMES : frames;
    level = clampf(config->level, 0.01f, 1.0f);
    feedback = clampf(config->feedback, 0.0f, 0.95f);
    damping = clampf(config->damping, 0.0f, 1.0f);

    // y = lowpass(a * line), line = in - g * y; DC gain a / (128 - b)
    b = to_q7(damping * 0.9f);
    a = to_q7(level * (1.0f - damping * 0.9f));
    g = to_q7(-feedback / level);

    if (delay_buffers[bus].frames != frames) {
        void *fresh = linearAlloc(frames * FRAME_BYTES);
        if (!fresh)
            return OPENDSP_ERR_NO_MEMORY;
        memset(fresh, 0, frames * FRAME_BYTES);
        DSP_FlushDataCache(fresh, frames * FRAME_BYTES);
        rc = send_delay(bus, 1, (u16)frames, g, a, b, osConvertVirtToPhys(fresh));
        if (R_FAILED(rc)) {
            linearFree(fresh);
            return rc;
        }
        free_delay(bus);
        delay_buffers[bus].buffer = fresh;
        delay_buffers[bus].frames = frames;
        return 0;
    }
    return send_delay(bus, 1, (u16)frames, g, a, b, osConvertVirtToPhys(delay_buffers[bus].buffer));
}

static s16 to_q15(float v)
{
    float q = clampf(v, -1.0f, 1.0f) * 32767.0f;
    return (s16)(q < 0 ? q - 0.5f : q + 0.5f);
}

Result opendspReverbSet(unsigned bus, const opendspReverbConfig *config)
{
    static const u16 line_max[4] = EXT_REVERB_LINE_MAX;
    u16 args[EXT_REVERB_ARG_WORDS] = { (u16)bus };
    float size, decay, width, level, energy = 0, input;
    int k;
    if (bus > 1 || (config && config->struct_size != sizeof(*config)))
        return OPENDSP_ERR_ARGUMENT;
    if (!config)
        return opendsp_request(EXT_CMD_REVERB_SET, args, EXT_REVERB_ARG_WORDS, NULL, 0);
    size = 0.35f + 0.65f * clampf(config->room_size, 0.0f, 1.0f);
    decay = clampf(config->decay_time, 0.1f, 10.0f);
    width = clampf(config->width, 0.0f, 1.0f);
    level = clampf(config->level, 0.0f, 1.0f);
    args[1] = 1;
    for (k = 0; k < 4; k++) {
        u16 length = (u16)(line_max[k] * size + 0.5f);
        if (length < EXT_REVERB_LINE_MIN)
            length = EXT_REVERB_LINE_MIN;
        args[2 + k] = length;
        // -60 dB after decay seconds
        float g = powf(10.0f, -3.0f * length / (decay * SAMPLE_RATE));
        args[6 + k] = (u16)to_q15(g);
        energy += g * g / 4;
    }
    args[10] = (u16)to_q15(clampf(config->damping, 0.0f, 1.0f) * 0.9f);
    args[11] = (u16)to_q15(clampf(config->diffusion, 0.0f, 1.0f) * 0.7f);
    args[12] = (u16)to_q15(level * (1.0f + width) * 0.5f);
    args[13] = (u16)to_q15(level * (1.0f - width) * 0.5f);
    // keeps long tails from building up
    input = 0.87f * sqrtf(1.0f - energy);
    args[14] = (u16)to_q15(input > 1.0f ? 1.0f : input);
    return opendsp_request(EXT_CMD_REVERB_SET, args, EXT_REVERB_ARG_WORDS, NULL, 0);
}

void opendsp_effects_release(void)
{
    free_delay(0);
    free_delay(1);
}
