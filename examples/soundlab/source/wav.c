// SPDX-License-Identifier: BSD-2-Clause
// WAV streaming from the SD card.
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include "wav.h"
#include "audio.h"

#define NBUF 4
#define BUF_FRAMES 8192
#define MAX_FILES 64

static char *names[MAX_FILES];
static int nfiles;

static ndspWaveBuf wb[NBUF];
static u8 *mem[NBUF];
static FILE *fp;
static u32 data_off, data_len, data_pos, frame_bytes, rate;
static u16 chans, bits;
static volatile bool playing, looping, at_eof, run;
static Thread th;
static LightLock lk;
static char info[64];

static int cmp(const void *a, const void *b) { return strcasecmp(*(char *const *)a, *(char *const *)b); }

int wav_scan(void)
{
    for (int i = 0; i < nfiles; i++) free(names[i]);
    nfiles = 0;
    mkdir("sdmc:/3ds", 0777);
    mkdir(WAV_DIR, 0777);
    DIR *d = opendir(WAV_DIR);
    if (!d) return 0;
    struct dirent *e;
    while ((e = readdir(d)) && nfiles < MAX_FILES) {
        size_t l = strlen(e->d_name);
        if (l > 4 && !strcasecmp(e->d_name + l - 4, ".wav")) names[nfiles++] = strdup(e->d_name);
    }
    closedir(d);
    qsort(names, nfiles, sizeof names[0], cmp);
    return nfiles;
}
int wav_count(void) { return nfiles; }
const char *wav_name(int i) { return i >= 0 && i < nfiles ? names[i] : ""; }

static u32 rd32(const u8 *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24); }
static u16 rd16(const u8 *p) { return p[0] | (p[1] << 8); }

static bool parse(char *err, size_t errlen)
{
    u8 h[12];
    if (fread(h, 1, 12, fp) != 12 || memcmp(h, "RIFF", 4) || memcmp(h + 8, "WAVE", 4)) {
        snprintf(err, errlen, "not a RIFF/WAVE file"); return false;
    }
    bool have_fmt = false;
    for (;;) {
        u8 c[8];
        if (fread(c, 1, 8, fp) != 8) { snprintf(err, errlen, "no data chunk"); return false; }
        u32 sz = rd32(c + 4);
        if (!memcmp(c, "fmt ", 4)) {
            u8 f[16];
            if (sz < 16 || fread(f, 1, 16, fp) != 16) { snprintf(err, errlen, "bad fmt chunk"); return false; }
            u16 tag = rd16(f);
            chans = rd16(f + 2); rate = rd32(f + 4); bits = rd16(f + 14);
            if (tag != 1 && tag != 0xFFFE) { snprintf(err, errlen, "only PCM WAV (format %u)", tag); return false; }
            if (chans < 1 || chans > 2 || (bits != 8 && bits != 16)) {
                snprintf(err, errlen, "%u ch / %u bit: need mono/stereo, 8/16 bit", chans, bits); return false;
            }
            if (rate < 1000 || rate > 4 * 32728) { snprintf(err, errlen, "sample rate %lu Hz not supported", rate); return false; }
            fseek(fp, (long)((sz - 16) + (sz & 1)), SEEK_CUR);
            have_fmt = true;
        } else if (!memcmp(c, "data", 4)) {
            if (!have_fmt) { snprintf(err, errlen, "data before fmt"); return false; }
            data_off = (u32)ftell(fp); data_len = sz; data_pos = 0;
            frame_bytes = chans * bits / 8;
            data_len -= data_len % frame_bytes;
            return true;
        } else {
            fseek(fp, (long)(sz + (sz & 1)), SEEK_CUR);
        }
    }
}

// returns frames read
static u32 fill(int k)
{
    u32 want = BUF_FRAMES * frame_bytes, got = 0;
    while (got < want) {
        if (data_pos >= data_len) {
            if (!looping) break;
            fseek(fp, (long)data_off, SEEK_SET); data_pos = 0;
        }
        u32 n = want - got;
        if (n > data_len - data_pos) n = data_len - data_pos;
        size_t r = fread(mem[k] + got, 1, n, fp);
        if (r == 0) { data_pos = data_len; if (!looping) break; continue; }
        got += (u32)r; data_pos += (u32)r;
    }
    got -= got % frame_bytes;
    if (bits == 8) for (u32 i = 0; i < got; i++) mem[k][i] ^= 0x80;   // 8-bit WAV is unsigned
    DSP_FlushDataCache(mem[k], got);
    return got / frame_bytes;
}

static void queue_free_buffers(void)
{
    for (int k = 0; k < NBUF; k++) {
        if (wb[k].status != NDSP_WBUF_FREE && wb[k].status != NDSP_WBUF_DONE) continue;
        if (at_eof) continue;
        u32 n = fill(k);
        if (!n) { at_eof = true; continue; }
        wb[k].nsamples = n;
        ndspChnWaveBufAdd(CH_WAV, &wb[k]);
    }
    if (at_eof) {
        bool busy = false;
        for (int k = 0; k < NBUF; k++)
            if (wb[k].status == NDSP_WBUF_QUEUED || wb[k].status == NDSP_WBUF_PLAYING) busy = true;
        if (!busy) playing = false;
    }
}

static void stream_main(void *arg)
{
    (void)arg;
    while (run) {
        LightLock_Lock(&lk);
        if (playing && fp) queue_free_buffers();
        LightLock_Unlock(&lk);
        svcSleepThread(4000000);
    }
}

void wav_apply(void)
{
    if (!rate) return;
    float r = rate * (S.speed / 100.f);
    if (r > 4 * 32728.f) r = 4 * 32728.f;
    ndspChnSetRate(CH_WAV, r);
    ndspChnSetInterp(CH_WAV, S.interp);
}

static void close_file(void)
{
    ndspChnWaveBufClear(CH_WAV);
    if (fp) { fclose(fp); fp = NULL; }
    playing = false;
}

bool wav_play(int i, char *err, size_t errlen)
{
    if (!th) {
        LightLock_Init(&lk);
        for (int k = 0; k < NBUF; k++) mem[k] = linearAlloc(BUF_FRAMES * 4);
        run = true;
        s32 prio = 0x30;
        svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
        th = threadCreate(stream_main, NULL, 16 * 1024, prio - 1, -2, false);
    }
    LightLock_Lock(&lk);
    close_file();
    char path[300];
    snprintf(path, sizeof path, "%s/%s", WAV_DIR, wav_name(i));
    fp = fopen(path, "rb");
    if (!fp) { snprintf(err, errlen, "cannot open %s", wav_name(i)); LightLock_Unlock(&lk); return false; }
    if (!parse(err, errlen)) { close_file(); LightLock_Unlock(&lk); return false; }
    snprintf(info, sizeof info, "%lu Hz %s PCM%u", rate, chans == 2 ? "stereo" : "mono", bits);
    u16 fmt = bits == 16 ? (chans == 2 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16)
                         : (chans == 2 ? NDSP_FORMAT_STEREO_PCM8 : NDSP_FORMAT_MONO_PCM8);
    ndspChnWaveBufClear(CH_WAV);
    ndspChnSetFormat(CH_WAV, fmt);
    wav_apply();
    audio_set_mix(CH_WAV, 1.0f, 1.0f);
    for (int k = 0; k < NBUF; k++) { memset(&wb[k], 0, sizeof wb[k]); wb[k].data_vaddr = mem[k]; }
    at_eof = false;
    playing = true;
    queue_free_buffers();
    LightLock_Unlock(&lk);
    return true;
}

void wav_stop(void)
{
    if (!th) return;
    LightLock_Lock(&lk);
    close_file();
    rate = 0;
    LightLock_Unlock(&lk);
}
bool wav_playing(void) { return playing; }
void wav_set_loop(bool l) { looping = l; }
bool wav_loop(void) { return looping; }
const char *wav_info(void) { return playing ? info : ""; }
float wav_progress(void) { return data_len ? (float)data_pos / data_len : 0; }

void wav_exit(void)
{
    if (!th) return;
    wav_stop();
    run = false;
    threadJoin(th, U64_MAX); threadFree(th); th = NULL;
    for (int k = 0; k < NBUF; k++) linearFree(mem[k]);
}
