// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "gfx.h"
#include "wav.h"
#include "monitor.h"

static monitor_view view;
static u64 previous_tick, previous_decode;
static unsigned previous_backend;
static opendspAudioFormat previous_format;
static bool primed;

void monitor_reset(void)
{
    memset(&view, 0, sizeof(view));
    view.arm_decode = -1;
    previous_tick = previous_decode = 0;
    primed = false;
}
void monitor_hold(void)
{
    view.held = !view.held;
    primed = false;
    view.sampled_at = 0;
}
void monitor_rebase(void)
{
    primed = false;
    view.held = false;
    view.sampled_at = 0;
}
const monitor_view *monitor_get(void)
{
    return &view;
}

const char *monitor_backend(void)
{
    if (!view.have_stream)
        return "None";
    if (view.info.format == OPENDSP_AUDIO_WAV)
        return "PCM";
    if (view.info.format == OPENDSP_AUDIO_MP3) {
        if (view.backend == OPENDSP_MP3_BACKEND_DSP)
            return "MP3 / DSP";
        if (view.backend == OPENDSP_MP3_BACKEND_ARM11)
            return "MP3 / ARM11";
        return "MP3 / --";
    }
    if (view.info.format == OPENDSP_AUDIO_AAC) {
        if (view.backend == OPENDSP_AAC_BACKEND_DSP)
            return "AAC / DSP";
        if (view.backend == OPENDSP_AAC_BACKEND_ARM11)
            return "AAC / ARM11";
        return "AAC / --";
    }
    return "--";
}

bool monitor_update(u64 now)
{
    if (view.held || (view.sampled_at && now - view.sampled_at < SYSCLOCK_ARM11 / 4))
        return false;
    view.dsp.struct_size = sizeof(view.dsp);
    view.stream.struct_size = sizeof(view.stream);
    view.info.struct_size = sizeof(view.info);
    view.have_dsp = R_SUCCEEDED(opendspGetStats(&view.dsp, false));
    view.have_stream = wav_playing() && wav_stream_info(&view.info) && wav_stats(&view.stream);
    view.backend = view.have_stream ? wav_active_backend() : OPENDSP_AAC_BACKEND_AUTO;
    view.arm_decode = -1;
    // Rebase after counter resets, decoder changes or suspension.
    u64 delta = now - previous_tick;
    if (primed && (now < previous_tick || delta > 2 * SYSCLOCK_ARM11)) {
        view.head = view.count = 0;
    }
    if (primed && now > previous_tick && delta <= 2 * SYSCLOCK_ARM11) {
        if (view.have_stream &&
            (view.info.format == OPENDSP_AUDIO_AAC || view.info.format == OPENDSP_AUDIO_MP3) &&
            view.backend == OPENDSP_AAC_BACKEND_ARM11 && previous_backend == view.backend &&
            previous_format == view.info.format && view.stream.decode_ticks >= previous_decode)
            view.arm_decode = (float)((double)(view.stream.decode_ticks - previous_decode) / delta);
    }
    previous_tick = now;
    previous_decode = view.have_stream ? view.stream.decode_ticks : 0;
    previous_backend = view.backend;
    previous_format = view.have_stream ? view.info.format : OPENDSP_AUDIO_WAV;
    primed = true;
    view.dsp_history[view.head] = view.have_dsp ? view.dsp.load : -1;
    view.arm_history[view.head] = view.arm_decode;
    view.head = (view.head + 1) % MONITOR_HISTORY;
    if (view.count < MONITOR_HISTORY)
        ++view.count;
    view.sampled_at = now;
    return true;
}

static void meter(int y, float value, u32 color)
{
    gfx_rect(12, y, 376, 8, RGB(30, 42, 64));
    if (isfinite(value) && value >= 0) {
        if (value > 1)
            value = 1;
        gfx_rect(12, y, (int)(value * 376), 8, color);
    }
}
static void plot(int y, const float *values, float limit, u32 color)
{
    const u32 grid = RGB(38, 50, 75);
    gfx_rect(12, y, 376, 52, RGB(18, 26, 44));
    for (int i = 0; i < 3; ++i)
        gfx_hline(12, y + i * 25, 376, grid);
    bool previous = false;
    int px = 0, py = 0;
    for (unsigned i = 0; i < view.count; ++i) {
        unsigned at = (view.head + MONITOR_HISTORY - view.count + i) % MONITOR_HISTORY;
        float v = values[at];
        if (!isfinite(v) || v < 0) {
            previous = false;
            continue;
        }
        if (v > limit)
            v = limit;
        int x = 12 + (MONITOR_HISTORY - view.count + i) * 375 / (MONITOR_HISTORY - 1);
        int yy = y + 50 - (int)(v * 50 / limit);
        if (previous)
            gfx_line(px, py, x, yy, color);
        else
            gfx_rect(x, yy, 2, 2, color);
        px = x;
        py = yy;
        previous = true;
    }
}
void monitor_draw_top(void)
{
    const u32 cyan = RGB(80, 220, 255), orange = RGB(255, 170, 60), white = RGB(235, 240, 255);
    char text[80];
    gfx_begin(RGB(10, 14, 30));
    gfx_text(12, 8, "OPENDSP MONITOR", white, 2);
    if (view.held)
        gfx_text(352, 12, "HOLD", orange, 1);
    if (view.have_dsp)
        snprintf(text, sizeof(text), "DSP %.1f%%  PEAK %.1f%%", view.dsp.load * 100, view.dsp.load_max * 100);
    else
        snprintf(text, sizeof(text), "DSP --");
    gfx_text(12, 32, text, cyan, 1);
    meter(43, view.have_dsp ? view.dsp.load : -1, cyan);
    plot(57, view.dsp_history, 1.2f, cyan);
    gfx_text(284, 59, "SCALE 0-120%", white, 1);
    gfx_text(12, 119, "CPU DECODER (EST.)", white, 1);
    if (view.arm_decode >= 0)
        snprintf(text, sizeof(text), "%.1f%%", view.arm_decode * 100);
    else
        snprintf(text, sizeof(text), "%s",
                 view.have_stream && view.backend == OPENDSP_AAC_BACKEND_DSP ? "INACTIVE" : "--");
    gfx_text(12, 132, text, orange, 1);
    meter(143, view.arm_decode, orange);
    plot(157, view.arm_history, 1, orange);
    gfx_text(284, 159, "SCALE 0-100%", white, 1);
    gfx_text(12, 217, monitor_backend(), cyan, 1);
    gfx_text(12, 230, "LAST 24 SECONDS", white, 1);
}
