// SPDX-License-Identifier: BSD-2-Clause
// OpenDSP Sound Lab demo app.
#include <3ds.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "audio.h"
#include "wav.h"
#include "gfx.h"
#include <opendsp.h>
#include <opendsp/aac.h>

#define APP_VERSION "1.2.0"

static PrintConsole bottom;

// bottom screen

#define C_RESET "\x1b[0m"
#define C_HEAD "\x1b[46;30m"
#define C_SEL "\x1b[47;30m"
#define C_VAL "\x1b[1;36m"
#define C_DIM "\x1b[2;37m"
#define C_HINT "\x1b[33m"
#define C_OK "\x1b[1;32m"
#define C_WARN "\x1b[1;31m"

static void line(int row, const char *fmt, ...)
{
    char b[160];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(b, sizeof b, fmt, ap);
    va_end(ap);
    printf("\x1b[%d;1H%s\x1b[K" C_RESET, row, b);
}

// 38 columns: a full row would wrap
static void item(int row, bool selected, const char *label, const char *value)
{
    char b[48];
    int width = 35 - (int)strlen(value);
    if (width < 1)
        width = 1;
    snprintf(b, sizeof b, "%-*.*s%s", width, width, label, value);
    printf("\x1b[%d;1H %s%c%-36.36s" C_RESET "\x1b[K", row, selected ? C_SEL : "", selected ? '>' : ' ', b);
}

static void header(const char *title)
{
    printf("\x1b[1;1H" C_HEAD " OpenDSP Sound Lab  %-19s" C_RESET, title);
}

static void hints(const char *h)
{
    printf("\x1b[29;1H" C_HINT "%-39.39s" C_RESET, h);
}

// pages

enum { P_MAIN, P_MUSIC, P_SFX, P_WAV, P_KEYS, P_FX, P_VERB, P_LAB, P_ABOUT, P_COUNT };
static int page = P_MAIN, cursor[P_COUNT], items;
static bool dirty = true, quit;
static char message[80];

static const char *main_items[] = {"Music: \"Teak Street\"",
                                   "Sound effects",
                                   "WAV / AAC player (SD card)",
                                   "Touch keyboard",
                                   "Mixer & filters",
                                   "Reverb & delay",
                                   "DSP lab",
                                   "About",
                                   "Exit"};
#define MAIN_ITEMS 9

static const char *on_off(bool b)
{
    return b ? "ON" : "OFF";
}

static void draw_main(void)
{
    header("");
    line(3, C_DIM " Every sound here is played by");
    line(4, C_DIM " the OpenDSP firmware.");
    items = MAIN_ITEMS;
    for (int i = 0; i < items; i++)
        item(6 + i, cursor[page] == i, main_items[i], "");
    hints(" A select   B back   START exit");
}

static void draw_music(void)
{
    char v[32];
    header("Music");
    items = 3 + NUM_TRACKS;
    item(3, cursor[page] == 0, music_is_playing() ? "Stop" : "Play", music_is_playing() ? "playing" : "");
    snprintf(v, sizeof v, "%d BPM", music_bpm());
    item(4, cursor[page] == 1, "Tempo", v);
    snprintf(v, sizeof v, "%d %%", S.speed);
    item(5, cursor[page] == 2, "Pitch (also WAV speed)", v);
    line(7, C_DIM "  Tracks (A: mute)");
    for (int t = 0; t < NUM_TRACKS; t++) {
        char label[40];
        snprintf(label, sizeof label, "%-9s %s", music_track_name(t), music_track_format(t));
        item(8 + t, cursor[page] == 3 + t, label, music_muted(t) ? "muted" : "on");
    }
    if (music_is_playing())
        line(15, "  Bar " C_VAL "%d/8" C_RESET "   Step " C_VAL "%2d" C_RESET "   Chord " C_VAL "%s",
             music_bar() + 1, music_step() + 1, music_chord());
    else
        line(15, "");
    line(17, C_DIM "  8 bars in A minor on 6 channels:");
    line(18, C_DIM "  3 synth voices from 64-sample tables,");
    line(19, C_DIM "  drums as PCM16, PCM8 and DSP-ADPCM.");
    hints(" A select  </> change  L/R x10  B back");
}

static void draw_sfx(void)
{
    header("Sound effects");
    items = sfx_count();
    for (int i = 0; i < items; i++)
        item(3 + i, cursor[page] == i, sfx_name(i), sfx_format(i));
    line(14, C_DIM "  Six channels, every press");
    line(15, C_DIM "  starts a new sound.");
    hints(" A play   B back");
}

static void draw_wav(void)
{
    header("WAV / AAC player");
    if (!wav_count()) {
        items = 1;
        item(3, cursor[page] == 0, "Rescan SD card", "");
        line(5, "  No .wav or .aac files found. Copy");
        line(6, "  some to " C_VAL "SD:/3ds/opendsp-lab/");
        line(8, C_DIM "  WAV: PCM 8/16-bit, up to 130 kHz.");
        line(9, C_DIM "  AAC: AAC-LC in ADTS, mono/stereo,");
        line(10, C_DIM "  ARM11 or Teak DSP backend.");
        hints(" A rescan   B back");
        return;
    }
    items = 3 + wav_count();
    item(3, cursor[page] == 0, "Stop", "");
    item(4, cursor[page] == 1, "Loop", on_off(wav_loop()));
    item(5, cursor[page] == 2, "Rescan SD card", "");
    line(6, C_DIM "  AAC: %s  [X]", wav_backend() == OPENDSP_AAC_BACKEND_DSP ? "Teak DSP" : "ARM11 (CPU)");
    int first = cursor[page] - 3 - 6;
    if (first < 0)
        first = 0;
    for (int r = 0; r < 14; r++) {
        int i = first + r;
        if (i >= wav_count()) {
            line(7 + r, "");
            continue;
        }
        item(7 + r, cursor[page] == 3 + i, wav_name(i), "");
    }
    if (wav_playing()) {
        char progress[8];
        float fraction = wav_progress();
        if (fraction < 0)
            snprintf(progress, sizeof(progress), "--");
        else
            snprintf(progress, sizeof(progress), "%d%%", (int)(fraction * 100));
        float load = wav_decode_load();
        if (load >= 0)
            line(21, "  " C_DIM "CPU decode (est.): %.1f%%", load * 100);
        else if (wav_active_backend() == OPENDSP_AAC_BACKEND_DSP)
            line(21, "  " C_DIM "CPU: I/O + transport (unmeasured)");
        else
            line(21, "  " C_DIM "CPU: PCM streaming (unmeasured)");
        line(23, "  " C_OK "%s", wav_info());
        opendspAudioStats stream_stats = {.struct_size = sizeof(stream_stats)};
        if (wav_stats(&stream_stats))
            line(24, "  %4s  Q %lu  underruns %lu", progress, (unsigned long)stream_stats.queued_frames,
                 (unsigned long)stream_stats.underruns);
        else
            line(24, "  " C_VAL "%s", progress);
    } else {
        line(21, "  %.36s", wav_error());
        line(23, "");
        line(24, "");
    }
    opendspStats st = {sizeof st};
    if (R_SUCCEEDED(opendspGetStats(&st, false)))
        line(22, "  DSP total: " C_VAL "%.1f%%" C_RESET "  peak " C_VAL "%.1f%%", st.load * 100,
             st.load_max * 100);
    else
        line(22, "  " C_DIM "DSP total: unavailable");
    hints(" A play   X AAC backend   B back");
}

// keyboard: two octaves, white keys 2 columns wide
#define KB_TOP 16
#define KB_BLACK_ROWS 6
#define KB_ROWS 11
static const int white_offset[7] = {0, 2, 4, 5, 7, 9, 11};
static const int has_black[7] = {1, 1, 0, 1, 1, 1, 0};
static int touch_note = -1;
static int button_note[4] = {-1, -1, -1, -1};

static int keyboard_base(void)
{
    return 12 * (S.key_octave + 1);
}

static void draw_keys(void)
{
    header("Touch keyboard");
    items = 0;
    line(3, "  Wave   " C_VAL "%-12s" C_RESET " (L / R)", audio_wave_name(S.key_wave));
    line(4, "  Octave " C_VAL "C%d" C_RESET "           (Up / Down)", S.key_octave);
    line(6, C_DIM "  Touch the keys. A/X/Y: C major");
    for (int r = 0; r < KB_ROWS; r++) {
        printf("\x1b[%d;1H      ", KB_TOP + r);
        for (int k = 0; k < 14; k++) {
            int octave = k / 7, n = k % 7;
            int midi = keyboard_base() + 12 * octave + white_offset[n];
            bool on = touch_note == midi;
            printf("%s ", on ? "\x1b[43;30m" : "\x1b[47;30m");
            if (r < KB_BLACK_ROWS && has_black[n] && k != 13)
                printf("%s ", touch_note == midi + 1 ? "\x1b[41m" : "\x1b[40m");
            else
                printf("%s|", on ? "\x1b[43;30m" : "\x1b[47;30m");
        }
        printf(C_RESET "\x1b[K");
    }
    hints(" Touch: play   B back");
}

static int key_at(int px, int py)
{
    int col = px / 8, row = py / 8 + 1;
    if (row < KB_TOP || row >= KB_TOP + KB_ROWS || col < 6 || col >= 6 + 28)
        return -1;
    int k = (col - 6) / 2, n = k % 7, octave = k / 7;
    if (row < KB_TOP + KB_BLACK_ROWS && ((col - 6) & 1) && has_black[n] && k != 13)
        return keyboard_base() + 12 * octave + white_offset[n] + 1;
    return keyboard_base() + 12 * octave + white_offset[n];
}

static const char *output_names[3] = {"Mono", "Stereo", "Surround"};
static const char *interp_names[3] = {"Polyphase", "Linear", "None"};

static void draw_fx(void)
{
    char v[32];
    header("Mixer & filters");
    items = 9;
    snprintf(v, sizeof v, "%d %%", S.master);
    item(3, cursor[page] == 0, "Master volume", v);
    item(4, cursor[page] == 1, "Output mode", output_names[S.output]);
    item(5, cursor[page] == 2, "Clipping", S.clip ? "Soft" : "Normal");
    item(6, cursor[page] == 3, "Interpolation", interp_names[S.interp]);
    item(7, cursor[page] == 4, "Filter (music + WAV)", audio_filter_name(S.filter));
    snprintf(v, sizeof v, "%d Hz", S.cutoff);
    item(8, cursor[page] == 5, "Filter frequency", v);
    snprintf(v, sizeof v, "%+d", S.pan);
    item(9, cursor[page] == 6, "Music pan", v);
    snprintf(v, sizeof v, "%d %%", S.surround);
    item(10, cursor[page] == 7, "Surround depth", v);
    item(11, cursor[page] == 8, "Menu sounds", on_off(S.ui_sounds));
    line(13, C_DIM "  Play the music, then sweep the");
    line(14, C_DIM "  filter frequency with L/R.");
    line(16, C_DIM "  Compare Polyphase and None at");
    line(17, C_DIM "  50 %% pitch.");
    hints(" </> change   L/R x10   B back");
}

static void draw_verb(void)
{
    char v[32];
    header("Reverb & delay");
    if (!audio_effects_available()) {
        items = 0;
        line(3, C_WARN "  This firmware has no DSP effects.");
        hints(" B back");
        return;
    }
    items = 7;
    item(3, cursor[page] == 0, "Reverb", audio_reverb_name(S.reverb));
    snprintf(v, sizeof v, "%d %%", S.reverb_send);
    item(4, cursor[page] == 1, "Reverb send", v);
    snprintf(v, sizeof v, "%d %%", S.reverb_return);
    item(5, cursor[page] == 2, "Reverb return", v);
    snprintf(v, sizeof v, "%d.%d s", S.reverb_decay / 10, S.reverb_decay % 10);
    item(6, cursor[page] == 3, "Decay time", v);
    item(8, cursor[page] == 4, "Delay", on_off(S.delay));
    snprintf(v, sizeof v, "%d ms", S.delay_ms);
    item(9, cursor[page] == 5, "Delay time", v);
    snprintf(v, sizeof v, "%d %%", S.delay_feedback);
    item(10, cursor[page] == 6, "Delay feedback", v);
    line(13, C_DIM "  Both effects run on the DSP:");
    line(14, C_DIM "  reverb on aux bus 0, delay on 1.");
    line(16, C_DIM "  Play the music or the keyboard");
    line(17, C_DIM "  and change the settings.");
    line(19, C_DIM "  Firmware %d.%d", audio_firmware_version() >> 8, audio_firmware_version() & 0xFF);
    hints(" </> change   L/R x10   B back");
}

static int clamp_int(int v, int lo, int hi)
{
    return v < lo ? lo : v > hi ? hi : v;
}
static int wrap_int(int v, int n)
{
    return ((v % n) + n) % n;
}

static void adjust_verb(int index, int delta, int big)
{
    switch (index) {
    case 0:
        S.reverb = wrap_int(S.reverb + delta, audio_reverb_count());
        if (S.reverb)
            S.reverb_decay = (int)(audio_reverb_default_decay(S.reverb) * 10 + 0.5f);
        break;
    case 1:
        S.reverb_send = clamp_int(S.reverb_send + delta * (big ? 20 : 5), 0, 100);
        break;
    case 2:
        S.reverb_return = clamp_int(S.reverb_return + delta * (big ? 20 : 5), 0, 100);
        break;
    case 3:
        S.reverb_decay = clamp_int(S.reverb_decay + delta * (big ? 10 : 1), 1, 100);
        break;
    case 4:
        S.delay ^= 1;
        break;
    case 5:
        S.delay_ms = clamp_int(S.delay_ms + delta * (big ? 100 : 20), 20, 960);
        break;
    case 6:
        S.delay_feedback = clamp_int(S.delay_feedback + delta * (big ? 20 : 5), 0, 90);
        break;
    }
    audio_apply_effects();
    wav_apply();
}

static void draw_lab(void)
{
    header("DSP lab");
    items = 5;
    item(3, cursor[page] == 0, "24-voice chord pad", on_off(lab_chord_on()));
    item(4, cursor[page] == 1, "Re-trigger burst (36 sounds)", "");
    item(5, cursor[page] == 2, "Formats: PCM16 > PCM8 > ADPCM", "");
    item(6, cursor[page] == 3, "Interpolation", interp_names[S.interp]);
    item(7, cursor[page] == 4, "Stop everything", "");
    int active = 0;
    for (int ch = 0; ch < 24; ch++)
        active += ndspChnIsPlaying(ch);
    opendspStats st = {sizeof st};
    bool have_stats = R_SUCCEEDED(opendspGetStats(&st, false));
    line(10, "  Frames      " C_VAL "%lu", (unsigned long)audio_frames());
    line(11, "  Dropped     " C_VAL "%lu", (unsigned long)audio_dropped());
    line(12, "  Channels    " C_VAL "%d / 24" C_RESET " playing", active);
    if (have_stats) {
        line(13, "  DSP load    " C_VAL "%d %%" C_RESET "  peak " C_VAL "%d %%", (int)(st.load * 100 + 0.5f),
             (int)(st.load_max * 100 + 0.5f));
        line(14, "  Late frames " C_VAL "%lu", (unsigned long)st.late_frames);
    }
    hints(" A start  </> change  B back");
}

static void draw_about(void)
{
    header("About");
    items = 0;
    line(3, "  " C_OK "OpenDSP" C_RESET " - open-source DSP");
    line(4, "  firmware for the Nintendo 3DS");
    line(6, C_DIM "  24 voices, PCM8, PCM16, DSP-ADPCM,");
    line(7, C_DIM "  three interpolation modes, filters,");
    line(8, C_DIM "  aux buses, reverb, delay, surround.");
    line(10, "  The firmware is built into this app");
    line(11, "  and started with " C_VAL "opendspInit()" C_RESET ".");
    line(13, C_DIM "  Made by Cellenseres");
    line(14, C_DIM "  BSD 2-Clause license");
    line(15, C_DIM "  Sound Lab " APP_VERSION);
    hints(" B back");
}

static void draw_bottom(void)
{
    consoleSelect(&bottom);
    printf("\x1b[2J");
    switch (page) {
    case P_MAIN:
        draw_main();
        break;
    case P_MUSIC:
        draw_music();
        break;
    case P_SFX:
        draw_sfx();
        break;
    case P_WAV:
        draw_wav();
        break;
    case P_KEYS:
        draw_keys();
        break;
    case P_FX:
        draw_fx();
        break;
    case P_VERB:
        draw_verb();
        break;
    case P_LAB:
        draw_lab();
        break;
    case P_ABOUT:
        draw_about();
        break;
    }
    if (message[0])
        line(27, C_WARN " %s", message);
}

// input

static void adjust_fx(int index, int delta, int big)
{
    switch (index) {
    case 0:
        S.master = clamp_int(S.master + delta * (big ? 10 : 5), 0, 100);
        break;
    case 1:
        S.output = wrap_int(S.output + delta, 3);
        break;
    case 2:
        S.clip ^= 1;
        break;
    case 3:
        S.interp = wrap_int(S.interp + delta, 3);
        break;
    case 4:
        S.filter = wrap_int(S.filter + delta, audio_filter_count());
        break;
    case 5: {
        float f = S.cutoff * powf(big ? 1.5f : 1.12f, (float)delta);
        S.cutoff = clamp_int((int)f, 60, 12000);
        break;
    }
    case 6:
        S.pan = clamp_int(S.pan + delta * (big ? 50 : 10), -100, 100);
        break;
    case 7:
        S.surround = clamp_int(S.surround + delta * 10, 0, 100);
        break;
    case 8:
        S.ui_sounds ^= 1;
        break;
    }
    audio_apply_settings();
    wav_apply();
}

static void go(int p)
{
    page = p;
    message[0] = 0;
    dirty = true;
}

static void keyboard_input(u32 down, u32 repeat, u32 held)
{
    touchPosition t;
    int note = -1;
    if (held & KEY_TOUCH) {
        hidTouchRead(&t);
        note = key_at(t.px, t.py);
    }
    if (note != touch_note) {
        if (touch_note >= 0)
            key_off(0);
        if (note >= 0)
            key_on(0, note);
        touch_note = note;
        dirty = true;
    }
    static const u32 buttons[3] = {KEY_A, KEY_X, KEY_Y};
    static const int intervals[3] = {0, 4, 7};
    for (int i = 0; i < 3; i++) {
        if (down & buttons[i]) {
            button_note[i] = keyboard_base() + intervals[i];
            key_on(1 + i, button_note[i]);
        }
        if (button_note[i] >= 0 && !(held & buttons[i])) {
            key_off(1 + i);
            button_note[i] = -1;
        }
    }
    if (repeat & KEY_DUP) {
        S.key_octave = clamp_int(S.key_octave + 1, 2, 6);
        dirty = true;
    }
    if (repeat & KEY_DDOWN) {
        S.key_octave = clamp_int(S.key_octave - 1, 2, 6);
        dirty = true;
    }
    if (down & KEY_R) {
        S.key_wave = wrap_int(S.key_wave + 1, audio_wave_count());
        dirty = true;
    }
    if (down & KEY_L) {
        S.key_wave = wrap_int(S.key_wave - 1, audio_wave_count());
        dirty = true;
    }
    if (down & KEY_B) {
        if (touch_note >= 0)
            key_off(0);
        touch_note = -1;
        ui_blip(2);
        go(P_MAIN);
    }
}

static void input(void)
{
    hidScanInput();
    u32 down = hidKeysDown(), repeat = hidKeysDownRepeat(), held = hidKeysHeld();
    if (down & KEY_START) {
        quit = true;
        return;
    }
    if (down & KEY_SELECT) {
        stop_all();
        wav_stop();
        dirty = true;
    }
    if (page == P_KEYS) {
        keyboard_input(down, repeat, held);
        return;
    }

    int *c = &cursor[page];
    if (items > 0) {
        if (repeat & KEY_DUP) {
            *c = wrap_int(*c - 1, items);
            ui_blip(0);
            dirty = true;
        }
        if (repeat & KEY_DDOWN) {
            *c = wrap_int(*c + 1, items);
            ui_blip(0);
            dirty = true;
        }
    }
    int delta = (repeat & KEY_DRIGHT)  ? 1
                : (repeat & KEY_DLEFT) ? -1
                : (repeat & KEY_R)     ? 1
                : (repeat & KEY_L)     ? -1
                                       : 0;
    int big = (repeat & (KEY_L | KEY_R)) != 0;

    if (down & KEY_B) {
        if (page != P_MAIN) {
            ui_blip(2);
            go(P_MAIN);
        }
        return;
    }

    switch (page) {
    case P_MAIN:
        if (down & KEY_A) {
            static const int pages[8] = {P_MUSIC, P_SFX, P_WAV, P_KEYS, P_FX, P_VERB, P_LAB, P_ABOUT};
            ui_blip(1);
            if (*c == 8) {
                quit = true;
                return;
            }
            if (pages[*c] == P_WAV)
                wav_scan();
            go(pages[*c]);
        }
        break;
    case P_MUSIC:
        if (down & KEY_A) {
            if (*c == 0) {
                if (lab_chord_on())
                    lab_chord(false);
                music_play(!music_is_playing());
            } else if (*c >= 3) {
                music_set_muted(*c - 3, !music_muted(*c - 3));
            }
            ui_blip(1);
            dirty = true;
        }
        if (delta && *c == 1) {
            music_set_bpm(music_bpm() + delta * (big ? 10 : 2));
            dirty = true;
        }
        if (delta && *c == 2) {
            S.speed = clamp_int(S.speed + delta * (big ? 25 : 5), 50, 200);
            wav_apply();
            dirty = true;
        }
        break;
    case P_SFX:
        if (down & KEY_A)
            sfx_play(*c);
        break;
    case P_WAV:
        if (down & KEY_X) {
            wav_set_backend(wav_backend() == OPENDSP_AAC_BACKEND_DSP ? OPENDSP_AAC_BACKEND_ARM11
                                                                     : OPENDSP_AAC_BACKEND_DSP);
            dirty = true;
        }
        if (!wav_count()) {
            if (down & KEY_A) {
                wav_scan();
                ui_blip(1);
                dirty = true;
            }
            break;
        }
        if (down & KEY_A) {
            ui_blip(1);
            if (*c == 0) {
                wav_stop();
            } else if (*c == 1) {
                wav_set_loop(!wav_loop());
            } else if (*c == 2) {
                wav_stop();
                wav_scan();
                *c = 0;
            } else {
                char error[80];
                if (!wav_play(*c - 3, error, sizeof error))
                    snprintf(message, sizeof message, "%.70s", error);
                else
                    message[0] = 0;
                audio_apply_settings();
            }
            dirty = true;
        }
        break;
    case P_FX:
        if (delta) {
            adjust_fx(*c, delta, big);
            dirty = true;
        }
        if (down & KEY_A) {
            adjust_fx(*c, 1, 0);
            ui_blip(1);
            dirty = true;
        }
        break;
    case P_VERB:
        if (delta) {
            adjust_verb(*c, delta, big);
            dirty = true;
        }
        if (down & KEY_A) {
            adjust_verb(*c, 1, 0);
            ui_blip(1);
            dirty = true;
        }
        break;
    case P_LAB:
        if (delta && *c == 3) {
            adjust_fx(3, delta, 0);
            dirty = true;
        }
        if (down & KEY_A) {
            if (*c == 0) {
                if (!lab_chord_on()) {
                    wav_stop();
                    music_play(false);
                }
                lab_chord(!lab_chord_on());
            } else if (*c == 1) {
                lab_burst();
            } else if (*c == 2) {
                lab_formats();
            } else if (*c == 3) {
                adjust_fx(3, 1, 0);
            } else {
                stop_all();
                wav_stop();
            }
            dirty = true;
        }
        break;
    }
}

// top screen

static float level_l, level_r;

static void draw_top(void)
{
    const u32 bg = RGB(10, 14, 30), panel = RGB(20, 28, 54), grid = RGB(36, 48, 84);
    const u32 cyan = RGB(80, 220, 255), orange = RGB(255, 170, 60), white = RGB(235, 240, 255),
              dim = RGB(120, 140, 180);
    gfx_begin(bg);
    gfx_text(12, 8, "OPENDSP SOUND LAB", white, 2);
    gfx_text(400 - 12 - gfx_text_width("OPEN-SOURCE DSP FIRMWARE", 1), 14, "OPEN-SOURCE DSP FIRMWARE", cyan,
             1);

    // oscilloscope of the DSP output
    const int sx = 12, sy = 30, sw = 376, sh = 104, mid = sy + sh / 2;
    gfx_rect(sx, sy, sw, sh, panel);
    for (int k = 1; k < 8; k++)
        gfx_vline(sx + k * sw / 8, sy, sh, grid);
    gfx_hline(sx, mid, sw, grid);
    u32 pos, len;
    const s16 *samples = audio_capture(&pos, &len);
    int peak_l = 0, peak_r = 0, ly = mid, ry = mid;
    for (int x = 0; x < sw; x++) {
        u32 i = (pos + len - sw + x) % len;
        int l = samples[2 * i], r = samples[2 * i + 1];
        int yl = mid - l * (sh / 2 - 2) / 32768, yr = mid - r * (sh / 2 - 2) / 32768;
        if (x) {
            gfx_line(sx + x - 1, ry, sx + x, yr, orange);
            gfx_line(sx + x - 1, ly, sx + x, yl, cyan);
        }
        ly = yl;
        ry = yr;
        if (abs(l) > peak_l)
            peak_l = abs(l);
        if (abs(r) > peak_r)
            peak_r = abs(r);
    }
    gfx_text(sx + 4, sy + 4, "L", cyan, 1);
    gfx_text(sx + 14, sy + 4, "R", orange, 1);
    gfx_text(sx + sw - gfx_text_width("DSP OUTPUT", 1) - 4, sy + 4, "DSP OUTPUT", dim, 1);

    // level meters
    float nl = peak_l / 32768.f, nr = peak_r / 32768.f;
    level_l = nl > level_l ? nl : level_l * 0.92f;
    level_r = nr > level_r ? nr : level_r * 0.92f;
    const int my = 140, mw = 376 - 20;
    gfx_text(sx, my, "L", cyan, 1);
    gfx_text(sx, my + 10, "R", orange, 1);
    gfx_rect(sx + 12, my, mw, 7, panel);
    gfx_rect(sx + 12, my + 10, mw, 7, panel);
    gfx_rect(sx + 12, my, (int)(level_l * mw), 7, level_l > 0.95f ? RGB(255, 60, 60) : cyan);
    gfx_rect(sx + 12, my + 10, (int)(level_r * mw), 7, level_r > 0.95f ? RGB(255, 60, 60) : orange);

    // channel activity
    const int cy = 164;
    for (int ch = 0; ch < 24; ch++) {
        u32 color = ch <= CH_HAT             ? RGB(90, 200, 120)
                    : ch < CH_UI             ? RGB(240, 200, 70)
                    : ch == CH_UI            ? RGB(200, 120, 255)
                    : ch == CH_WAV           ? RGB(80, 170, 255)
                    : ch < CH_KEY0 + CH_NKEY ? RGB(255, 120, 150)
                                             : RGB(160, 160, 190);
        gfx_rect(sx + ch * 16 + 1, cy, 13, 13, ndspChnIsPlaying(ch) ? color : panel);
    }
    gfx_text(sx, cy + 17, "MUSIC", RGB(90, 200, 120), 1);
    gfx_text(sx + 6 * 16, cy + 17, "SFX", RGB(240, 200, 70), 1);
    gfx_text(sx + 12 * 16, cy + 17, "UI", RGB(200, 120, 255), 1);
    gfx_text(sx + 13 * 16 + 4, cy + 17, "WAV", RGB(80, 170, 255), 1);
    gfx_text(sx + 14 * 16 + 20, cy + 17, "KEYS", RGB(255, 120, 150), 1);
    gfx_text(sx + 18 * 16 + 12, cy + 17, "SPARE", RGB(160, 160, 190), 1);

    char b[80];
    snprintf(b, sizeof b, "FRAMES %lu   DROPPED %lu", (unsigned long)audio_frames(),
             (unsigned long)audio_dropped());
    gfx_text(sx, 204, b, white, 1);
    if (lab_chord_on())
        snprintf(b, sizeof b, "NOW: 24-VOICE CHORD PAD");
    else if (music_is_playing())
        snprintf(b, sizeof b, "NOW: TEAK STREET  %d BPM  BAR %d  %s", music_bpm(), music_bar() + 1,
                 music_chord());
    else if (wav_playing())
        snprintf(b, sizeof b, "NOW: WAV %s", wav_info());
    else
        snprintf(b, sizeof b, "READY");
    gfx_text(sx, 218, b, cyan, 1);
}

int main(void)
{
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, &bottom);
    hidSetRepeatParameters(18, 5);
    printf("\x1b[2J\x1b[1;1H" C_HEAD " OpenDSP Sound Lab                     " C_RESET "\n\n  Starting...\n");
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();

    char error[160];
    if (!audio_init(error, sizeof error)) {
        printf("\x1b[2J\x1b[1;1H" C_HEAD " OpenDSP Sound Lab                     " C_RESET "\n\n" C_WARN
               "  %s" C_RESET "\n\n  START: exit\n",
               error);
        while (aptMainLoop()) {
            hidScanInput();
            if (hidKeysDown() & KEY_START)
                break;
            gspWaitForVBlank();
        }
        gfxExit();
        return 0;
    }
    wav_scan();

    u64 last_refresh = 0;
    while (aptMainLoop() && !quit) {
        input();
        if (quit)
            break;
        u64 now = osGetTime();
        if ((page == P_MUSIC || page == P_WAV || page == P_LAB) && now - last_refresh > 200) {
            dirty = true;
            last_refresh = now;
        }
        if (dirty) {
            draw_bottom();
            dirty = false;
        }
        draw_top();
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    stop_all();
    wav_exit();
    audio_exit();
    gfxExit();
    return 0;
}
