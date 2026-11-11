// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "gfx.h"
#include "wav.h"
#include "monitor.h"

static monitor_view view;
static opendspLoad previous_load;
static u64 previous_tick, previous_decode, previous_read, previous_decoded;
static u32 previous_idle;
static bool previous_have_load, previous_cpu, previous_stream;
static unsigned previous_backend;
static opendspAudioFormat previous_format;
static bool primed;
static opendspAudioStream *(*other_source)(void);

void monitor_set_source(opendspAudioStream *(*source)(void))
{
    other_source = source;
}

void monitor_reset(void)
{
    memset(&view, 0, sizeof(view));
    for (unsigned i = 0; i < MONITOR_SERIES; ++i)
        view.value[i] = -1;
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

float monitor_peak(unsigned series)
{
    float peak = -1;
    for (unsigned i = 0; i < view.count; ++i) {
        float v = view.history[series][(view.head + MONITOR_HISTORY - 1 - i) % MONITOR_HISTORY];
        if (isfinite(v) && v > peak)
            peak = v;
    }
    return peak;
}

const char *monitor_backend(void)
{
    if (!view.have_stream)
        return "None";
    if (view.info.format == OPENDSP_AUDIO_WAV)
        return "PCM";
    if (view.info.format == OPENDSP_AUDIO_MSU1)
        return "MSU-1 PCM";
    if (view.info.format == OPENDSP_AUDIO_OPUZ) {
        if (view.backend == OPENDSP_AAC_BACKEND_DSP)
            return "OPUZ / DSP";
        return view.backend == OPENDSP_AAC_BACKEND_ARM11 ? "OPUZ / ARM11" : "OPUZ / --";
    }
    if (view.info.format == OPENDSP_AUDIO_VORBIS) {
        if (view.backend == OPENDSP_AAC_BACKEND_DSP)
            return "VORBIS / DSP";
        return view.backend == OPENDSP_AAC_BACKEND_ARM11 ? "VORBIS / ARM11" : "VORBIS / --";
    }
    if (view.info.format == OPENDSP_AUDIO_OPUS) {
        if (view.backend == OPENDSP_AAC_BACKEND_DSP)
            return "OPUS / DSP";
        return view.backend == OPENDSP_AAC_BACKEND_ARM11 ? "OPUS / ARM11" : "OPUS / --";
    }
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

static void sample_dsp(void)
{
    if (view.have_load && previous_have_load) {
        u32 period = (view.load.period_ticks - previous_load.period_ticks) & OPENDSP_LOAD_MASK;
        u32 mix = (view.load.mix_ticks - previous_load.mix_ticks) & OPENDSP_LOAD_MASK;
        u32 codec = (view.load.codec_ticks - previous_load.codec_ticks) & OPENDSP_LOAD_MASK;
        if (view.load.frames == previous_load.frames || !period)
            return;
        view.value[SERIES_DSP_MIX] = (float)mix / period;
        view.value[SERIES_DSP_DECODE] = (float)codec / period;
        view.value[SERIES_DSP_TOTAL] = ((float)mix + codec) / period;
    } else if (view.have_dsp) {
        // Older firmware: last frame only.
        view.value[SERIES_DSP_TOTAL] = view.dsp.load;
    }
}

static void sample_stream(u64 delta)
{
    bool same = view.have_stream && previous_stream && previous_backend == view.backend &&
                previous_format == view.info.format && view.stream.read_ticks >= previous_read &&
                view.stream.decode_ticks >= previous_decode;
    if (!same)
        return;
    u64 read = view.stream.read_ticks - previous_read, work = view.stream.decode_ticks - previous_decode;
    view.value[SERIES_SD_READ] = (float)((double)read / delta);
    if (view.info.format == OPENDSP_AUDIO_WAV || view.info.format == OPENDSP_AUDIO_MSU1 ||
        view.backend != OPENDSP_AAC_BACKEND_ARM11)
        return;
    // Ogg decoders read inside ov_read.
    if (view.info.format == OPENDSP_AUDIO_VORBIS || view.info.format == OPENDSP_AUDIO_OPUS)
        work = work > read ? work - read : 0;
    view.value[SERIES_CPU_DECODE] = (float)((double)work / delta);
}

bool monitor_update(u64 now)
{
    if (view.held || (view.sampled_at && now - view.sampled_at < SYSCLOCK_ARM11 / 4))
        return false;
    // Idle read where now was taken.
    u32 idle;
    bool cpu = cpu_meter_idle(&idle);
    view.dsp.struct_size = sizeof(view.dsp);
    view.load.struct_size = sizeof(view.load);
    view.stream.struct_size = sizeof(view.stream);
    view.info.struct_size = sizeof(view.info);
    view.have_dsp = R_SUCCEEDED(opendspGetStats(&view.dsp, false));
    view.have_load = R_SUCCEEDED(opendspGetLoad(&view.load));
    view.have_stream = wav_stream_info(&view.info) && wav_stats(&view.stream);
    opendspAudioStream *other = !view.have_stream && other_source ? other_source() : NULL;
    if (other)
        view.have_stream = R_SUCCEEDED(opendspAudioGetInfo(other, &view.info)) &&
                           R_SUCCEEDED(opendspAudioGetStats(other, &view.stream));
    view.backend = view.have_stream ? wav_active_backend() : OPENDSP_AAC_BACKEND_AUTO;
    view.decoder.struct_size = sizeof(view.decoder);
    view.have_decoder = view.have_stream && view.backend == OPENDSP_AAC_BACKEND_DSP &&
                        wav_decoder_stats(&view.decoder);
    // Cached loops stop the decoder.
    view.loop_cached = view.have_stream && previous_stream && view.stream.loops &&
                       view.stream.decoded_frames == previous_decoded;
    for (unsigned i = 0; i < MONITOR_SERIES; ++i)
        view.value[i] = -1;
    // Rebase after counter resets or suspension.
    u64 delta = now - previous_tick;
    bool valid = primed && now > previous_tick && delta <= 2 * SYSCLOCK_ARM11;
    if (primed && !valid)
        view.head = view.count = 0;
    if (valid) {
        sample_dsp();
        sample_stream(delta);
        if (cpu && previous_cpu) {
            float busy = 1 - (float)((double)(u32)(idle - previous_idle) / delta);
            view.value[SERIES_CPU_TOTAL] = busy < 0 ? 0 : busy;
        }
    }
    previous_tick = now;
    previous_load = view.load;
    previous_have_load = view.have_load;
    previous_idle = idle;
    previous_cpu = cpu;
    previous_stream = view.have_stream;
    previous_decode = view.have_stream ? view.stream.decode_ticks : 0;
    previous_read = view.have_stream ? view.stream.read_ticks : 0;
    previous_decoded = view.have_stream ? view.stream.decoded_frames : 0;
    previous_backend = view.backend;
    previous_format = view.have_stream ? view.info.format : OPENDSP_AUDIO_WAV;
    primed = true;
    for (unsigned i = 0; i < MONITOR_SERIES; ++i)
        view.history[i][view.head] = view.value[i];
    view.head = (view.head + 1) % MONITOR_HISTORY;
    if (view.count < MONITOR_HISTORY)
        ++view.count;
    view.sampled_at = now;
    return true;
}

#define CELL_W 188
#define PLOT_H 54

static void plot(int x, int y, const float *values, u32 color)
{
    const u32 grid = RGB(38, 50, 75);
    gfx_rect(x, y, CELL_W, PLOT_H, RGB(18, 26, 44));
    for (int i = 1; i < 4; ++i)
        gfx_hline(x, y + i * PLOT_H / 4, CELL_W, grid);
    bool previous = false;
    int px = 0, py = 0;
    for (unsigned i = 0; i < view.count; ++i) {
        unsigned at = (view.head + MONITOR_HISTORY - view.count + i) % MONITOR_HISTORY;
        float v = values[at];
        if (!isfinite(v) || v < 0) {
            previous = false;
            continue;
        }
        if (v > 1)
            v = 1;
        int xx = x + (int)(MONITOR_HISTORY - view.count + i) * (CELL_W - 2) / (MONITOR_HISTORY - 1);
        int yy = y + PLOT_H - 2 - (int)(v * (PLOT_H - 2));
        if (previous)
            gfx_line(px, py, xx, yy, color);
        else
            gfx_rect(xx, yy, 2, 2, color);
        px = xx;
        py = yy;
        previous = true;
    }
}

static void cell(unsigned series, const char *name, int x, int y, u32 color)
{
    char text[16];
    float v = view.value[series];
    if (v >= 0)
        snprintf(text, sizeof(text), "%.1f%%", v * 100);
    else
        snprintf(text, sizeof(text), "--");
    gfx_text(x, y, name, RGB(235, 240, 255), 1);
    gfx_text(x + CELL_W - gfx_text_width(text, 1), y, text, color, 1);
    plot(x, y + 10, view.history[series], color);
}

void monitor_draw_top(void)
{
    const u32 cyan = RGB(80, 220, 255), orange = RGB(255, 170, 60), green = RGB(120, 230, 120);
    const u32 pink = RGB(255, 110, 200), yellow = RGB(240, 220, 90), blue = RGB(150, 160, 255);
    const u32 white = RGB(235, 240, 255);
    gfx_begin(RGB(10, 14, 30));
    gfx_text(8, 5, "OPENDSP MONITOR", white, 2);
    if (view.held)
        gfx_text(196, 10, "HOLD", orange, 1);
    else if (view.loop_cached)
        gfx_text(196, 10, "LOOP CACHE", orange, 1);
    const char *backend = monitor_backend();
    gfx_text(392 - gfx_text_width(backend, 1), 10, backend, cyan, 1);
    // DSP left, ARM11 right; 0-100%, 24 s.
    cell(SERIES_DSP_TOTAL, "DSP TOTAL", 8, 26, cyan);
    cell(SERIES_DSP_DECODE, "DSP DECODER", 8, 96, orange);
    cell(SERIES_DSP_MIX, "DSP MIX+FX", 8, 166, green);
    cell(SERIES_CPU_TOTAL, "CPU CORE 0", 204, 26, pink);
    cell(SERIES_CPU_DECODE, "CPU DECODER", 204, 96, yellow);
    cell(SERIES_SD_READ, "SD READ WAIT", 204, 166, blue);
}
