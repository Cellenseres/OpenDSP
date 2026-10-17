// SPDX-License-Identifier: BSD-2-Clause
// Reverb behaviour and mailbox layout checks on the host.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "reverb.h"
#include "extension.h"

#define F FRAME_SAMPLES

static int failures;
#define CHECK(c, ...) do { int ok_ = !!(c); printf(ok_ ? "ok:   " : "FAIL: "); printf(__VA_ARGS__); printf("\n"); if (!ok_) failures++; } while (0)

static s32 buf[4][2 * F];
static s32 *quad[4] = { buf[0], buf[1], buf[2], buf[3] };

static reverb_params_t preset(s16 gain, s16 damping)
{
    reverb_params_t p = { 1, { 1777, 1543, 1291, 1009 }, { gain, gain, gain, gain }, damping, 19660, 23000, 4000, 16000 };
    return p;
}

// one frame into a bus; returns the energy of the wet output
static double frame(u16 bus, s16 impulse, s16 noise_amp, unsigned *seed)
{
    double e = 0;
    for (int c = 0; c < 4; c++)
        for (int n = 0; n < F; n++) {
            s16 v = 0;
            if (impulse && c == 0 && n == 0) v = impulse;
            if (noise_amp) { *seed = *seed * 1103515245u + 12345u; v = (s16)((int)((*seed >> 16) & 0xFFFF) - 32768) * noise_amp / 32768; }
            buf[c][2 * n] = v;
        }
    reverb_run(bus, quad);
    for (int n = 0; n < F; n++)
        e += (double)buf[0][2 * n] * buf[0][2 * n] + (double)buf[1][2 * n] * buf[1][2 * n];
    for (int n = 0; n < F; n++)
        if (buf[2][2 * n] || buf[3][2 * n]) return -1;     // rear must be cleared
    return e;
}

int main(void)
{
    unsigned seed = 1;
    reverb_params_t p;

    // mailbox layout
    CHECK(EXT_ARGS + EXT_ARG_WORDS <= EXT_REPLY, "argument area ends before the reply");
    CHECK(EXT_REPLY + EXT_REPLY_WORDS <= EXT_WORDS, "reply fits the mailbox");
    CHECK(EXT_REVERB_ARG_WORDS <= EXT_ARG_WORDS, "reverb arguments fit");
    CHECK(EXT_BASE >= 0x0D00 && EXT_BASE + EXT_WORDS <= 0x1000, "mailbox sits between pipe buffers and globals");

    reverb_init();
    CHECK(!reverb_enabled(0) && !reverb_enabled(1), "both buses start disabled");

    // validation
    p = preset(28000, 8000);
    p.length[0] = 1778;
    CHECK(reverb_set(0, &p) == EXT_ERR_ARGUMENT && !reverb_enabled(0), "line longer than its buffer rejected");
    p = preset(28000, 8000);
    p.length[3] = EXT_REVERB_LINE_MIN - 1;
    CHECK(reverb_set(0, &p) == EXT_ERR_ARGUMENT, "line shorter than a frame rejected");
    p = preset(-1, 8000);
    CHECK(reverb_set(0, &p) == EXT_ERR_ARGUMENT, "negative feedback rejected");

    // impulse response
    p = preset(28000, 8000);
    CHECK(reverb_set(0, &p) == EXT_OK && reverb_enabled(0), "bus 0 enabled");
    double e[400];
    e[0] = frame(0, 20000, 0, &seed);
    for (int f = 1; f < 400; f++) e[f] = frame(0, 0, 0, &seed);
    int first = 0, rear_ok = 1;
    for (int f = 0; f < 400; f++) { if (e[f] < 0) rear_ok = 0; if (!first && e[f] > 0) first = f; }
    double early = 0, late = 0, end = 0;
    for (int f = first; f < first + 20; f++) early += e[f];
    for (int f = first + 100; f < first + 120; f++) late += e[f];
    for (int f = 380; f < 400; f++) end += e[f];
    CHECK(rear_ok, "rear channels cleared, output is front only");
    CHECK(first >= 5 && first <= 8, "first echo after the shortest line (frame %d)", first);
    CHECK(early > 1e5, "wet output present (%.3g)", early);
    CHECK(late < early * 0.1 && late > 0, "tail decays (%.1f dB after 0.5 s)", 10 * log10(late / early));
    CHECK(end < early * 1e-3, "tail below -30 dB after 2 s (%.1f dB)", 10 * log10(end / early + 1e-30));

    // the tail ends in exact silence, and input wakes the network again
    int silent_from = -1;
    for (int f = 400; f < 3000; f++) {
        double x = frame(0, 0, 0, &seed);
        if (x != 0) silent_from = -1;
        else if (silent_from < 0) silent_from = f;
    }
    CHECK(silent_from > 0 && silent_from < 2000, "tail ends in exact silence (frame %d)", silent_from);
    double wake = frame(0, 20000, 0, &seed);
    for (int f = 1; f < 20; f++) wake += frame(0, 0, 0, &seed);
    CHECK(wake > early * 0.5, "input after silence produces a tail again");

    // bus 1 stays silent while bus 0 runs
    double other = 0;
    p = preset(28000, 0);
    reverb_set(1, &p);
    for (int f = 0; f < 50; f++) other += frame(1, 0, 0, &seed);
    CHECK(other == 0, "bus 1 output silent without input");

    // full-scale noise: output saturates but stays bounded and decays afterwards
    p = preset(32767, 0);
    reverb_set(0, &p);
    double loud = 0, after10 = 0, after20 = 0;
    for (int f = 0; f < 60; f++) loud = frame(0, 0, 32767, &seed);
    for (int f = 0; f < 2000; f++) after10 = frame(0, 0, 0, &seed);
    for (int f = 0; f < 2000; f++) after20 = frame(0, 0, 0, &seed);
    CHECK(after10 < loud * 0.03 && after20 < after10 * 0.05, "feedback 1.0 is capped: tail decays after full-scale input (%.1f dB at 10 s, %.1f dB at 20 s)",
          10 * log10(after10 / loud), 10 * log10(after20 / loud));

    // disable and enable clears the lines
    p = preset(28000, 8000);
    reverb_set(0, &p);
    frame(0, 30000, 0, &seed);
    p.enable = 0;
    reverb_set(0, &p);
    p.enable = 1;
    reverb_set(0, &p);
    double stale = 0;
    for (int f = 0; f < 20; f++) stale += frame(0, 0, 0, &seed);
    CHECK(stale == 0, "re-enable starts from silence");

    printf("%s (%d failure(s))\n", failures ? "EFFECTS TESTS FAILED" : "EFFECTS TESTS PASSED", failures);
    return failures ? 1 : 0;
}
