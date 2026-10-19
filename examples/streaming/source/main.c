// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <opendsp.h>
#include <opendsp/audio.h>

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    const char *path = argc > 1 ? argv[1] : "sdmc:/3ds/opendsp-streaming/music.aac";
    bool romfs = !strncmp(path, "romfs:/", 7);
    Result r = romfs ? romfsInit() : 0;
    bool romfs_ready = romfs && R_SUCCEEDED(r);
    if (R_SUCCEEDED(r)) r = opendspInit();
    bool initialized = R_SUCCEEDED(r);
    opendspAudioStream *music = NULL, *ambient = NULL;
    if (initialized) r = opendspAudioInit((1u << 0) | (1u << 1));
    if (R_SUCCEEDED(r)) r = opendspAudioOpen(&music, path, NULL);
    if (R_SUCCEEDED(r)) {
        opendspAudioSetLoop(music, true);
        opendspAudioPlay(music);
        opendspAudioConfig cfg = OPENDSP_AUDIO_CONFIG_INIT;
        cfg.channel = 1;
        if (R_SUCCEEDED(opendspAudioOpen(&ambient, "sdmc:/3ds/opendsp-streaming/ambient.wav", &cfg))) {
            opendspAudioSetLoop(ambient, true);
            opendspAudioSetVolume(ambient, 0.25f);
            opendspAudioPlay(ambient);
        }
    }
    while (aptMainLoop()) {
        hidScanInput();
        u32 down = hidKeysDown();
        if (down & KEY_START) break;
        if (music) {
            if (down & KEY_A) {
                if (opendspAudioGetState(music) == OPENDSP_AUDIO_PAUSED) opendspAudioResume(music);
                else opendspAudioPause(music);
            }
            if (down & KEY_B) opendspAudioStop(music);
            if (down & KEY_X) opendspAudioPlay(music);
            opendspAudioInfo info = {.struct_size = sizeof(info)};
            opendspAudioStats stats = {.struct_size = sizeof(stats)};
            opendspAudioGetInfo(music, &info);
            opendspAudioGetStats(music, &stats);
            printf("\x1b[1;1HOpenDSP streaming\n\n%lu Hz / %u channels\nState %d / AAC backend %lu\n"
                   "Position %.2f s\nQueued %lu / %lu frames\nUnderruns %lu\nError %08lX\n\n"
                   "Ambient WAV: %s\n\nA pause/resume  B stop  X play\nSTART exit\x1b[J",
                (unsigned long)info.sample_rate, info.channels, opendspAudioGetState(music),
                (unsigned long)opendspAudioGetBackend(music),
                (double)opendspAudioGetPosition(music) / info.sample_rate,
                (unsigned long)stats.queued_frames, (unsigned long)stats.capacity_frames,
                (unsigned long)stats.underruns, (unsigned long)opendspAudioGetLastError(music),
                ambient ? "playing independently" : "optional file absent");
        } else printf("\x1b[1;1HOpen failed: %08lX\n%s\n\nSTART exits\x1b[J", (unsigned long)r, path);
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    opendspAudioClose(&music);
    opendspAudioClose(&ambient);
    opendspAudioExit();
    if (initialized) opendspExit();
    // Retry retained handles after DSP shutdown.
    opendspAudioClose(&music);
    opendspAudioClose(&ambient);
    opendspAudioExit();
    if (romfs_ready) romfsExit();
    gfxExit();
    return 0;
}
