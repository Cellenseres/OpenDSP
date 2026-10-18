// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <opendsp/aac.h>
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
static volatile bool stop_requested;
static Thread th;
static LightLock lk;
static LightEvent stream_event;
static volatile bool stream_event_ready;
static char info[64];

#define AAC_IN_SIZE 16384
#define AAC_MAX_OUT 4096 // frames from one ADTS frame
static bool aac;
static u32 requested_backend = OPENDSP_AAC_BACKEND_DSP;
static char stream_error[80];
static opendspAacDecoder *dec;
static u8 *aac_in;
static u32 in_len, in_off;
static bool in_eof;
static u64 decode_ticks, decoded_frames;
static s16 probe_pcm[AAC_MAX_OUT * 2];

static int cmp(const void *a, const void *b)
{
    return strcasecmp(*(char *const *)a, *(char *const *)b);
}

int wav_scan(void)
{
    for (int i = 0; i < nfiles; i++)
        free(names[i]);
    nfiles = 0;
    mkdir("sdmc:/3ds", 0777);
    mkdir(WAV_DIR, 0777);
    DIR *d = opendir(WAV_DIR);
    if (!d)
        return 0;
    struct dirent *e;
    while ((e = readdir(d)) && nfiles < MAX_FILES) {
        size_t l = strlen(e->d_name);
        if (l > 4 && (!strcasecmp(e->d_name + l - 4, ".wav") || !strcasecmp(e->d_name + l - 4, ".aac")))
            names[nfiles++] = strdup(e->d_name);
    }
    closedir(d);
    qsort(names, nfiles, sizeof names[0], cmp);
    return nfiles;
}
int wav_count(void)
{
    return nfiles;
}
const char *wav_name(int i)
{
    return i >= 0 && i < nfiles ? names[i] : "";
}

static u32 rd32(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}
static u16 rd16(const u8 *p)
{
    return p[0] | (p[1] << 8);
}

static bool parse(char *err, size_t errlen)
{
    u8 h[12];
    if (fread(h, 1, 12, fp) != 12 || memcmp(h, "RIFF", 4) || memcmp(h + 8, "WAVE", 4)) {
        snprintf(err, errlen, "not a RIFF/WAVE file");
        return false;
    }
    bool have_fmt = false;
    for (;;) {
        u8 c[8];
        if (fread(c, 1, 8, fp) != 8) {
            snprintf(err, errlen, "no data chunk");
            return false;
        }
        u32 sz = rd32(c + 4);
        if (!memcmp(c, "fmt ", 4)) {
            u8 f[16];
            if (sz < 16 || fread(f, 1, 16, fp) != 16) {
                snprintf(err, errlen, "bad fmt chunk");
                return false;
            }
            u16 tag = rd16(f);
            chans = rd16(f + 2);
            rate = rd32(f + 4);
            bits = rd16(f + 14);
            if (tag != 1 && tag != 0xFFFE) {
                snprintf(err, errlen, "only PCM WAV (format %u)", tag);
                return false;
            }
            if (chans < 1 || chans > 2 || (bits != 8 && bits != 16)) {
                snprintf(err, errlen, "%u ch / %u bit: need mono/stereo, 8/16 bit", chans, bits);
                return false;
            }
            if (rate < 1000 || rate > 4 * 32728) {
                snprintf(err, errlen, "sample rate %lu Hz not supported", rate);
                return false;
            }
            fseek(fp, (long)((sz - 16) + (sz & 1)), SEEK_CUR);
            have_fmt = true;
        } else if (!memcmp(c, "data", 4)) {
            if (!have_fmt) {
                snprintf(err, errlen, "data before fmt");
                return false;
            }
            data_off = (u32)ftell(fp);
            data_len = sz;
            data_pos = 0;
            frame_bytes = chans * bits / 8;
            data_len -= data_len % frame_bytes;
            return true;
        } else {
            fseek(fp, (long)(sz + (sz & 1)), SEEK_CUR);
        }
    }
}

static u32 fill(int k)
{
    u32 want = BUF_FRAMES * frame_bytes, got = 0;
    while (got < want && !stop_requested) {
        if (data_pos >= data_len) {
            if (!looping)
                break;
            fseek(fp, (long)data_off, SEEK_SET);
            data_pos = 0;
        }
        u32 n = want - got;
        if (n > data_len - data_pos)
            n = data_len - data_pos;
        size_t r = fread(mem[k] + got, 1, n, fp);
        if (r == 0) {
            data_pos = data_len;
            if (!looping)
                break;
            continue;
        }
        got += (u32)r;
        data_pos += (u32)r;
    }
    got -= got % frame_bytes;
    if (bits == 8)
        for (u32 i = 0; i < got; i++)
            mem[k][i] ^= 0x80; // 8-bit WAV is unsigned
    DSP_FlushDataCache(mem[k], got);
    return got / frame_bytes;
}

static void aac_read(void)
{
    if (stop_requested)
        return;
    if (in_off) {
        memmove(aac_in, aac_in + in_off, in_len - in_off);
        in_len -= in_off;
        in_off = 0;
    }
    while (in_len < AAC_IN_SIZE && !in_eof && !stop_requested) {
        size_t r = fread(aac_in + in_len, 1, AAC_IN_SIZE - in_len, fp);
        in_len += (u32)r;
        if (r)
            continue;
        if (!looping) {
            in_eof = true;
            break;
        }
        fseek(fp, 0, SEEK_SET);
        data_pos = 0;
    }
}

// one ADTS frame into out; returns frames, 0 at the end of the stream
static u32 aac_frame(s16 *out, size_t capacity, opendspAacInfo *fi)
{
    for (;;) {
        if (stop_requested)
            return 0;
        size_t used = 0, got = 0;
        if (in_len - in_off < 8192)
            aac_read();
        u64 t0 = svcGetSystemTick();
        Result r = opendspAacDecode(dec, aac_in + in_off, in_len - in_off, &used, out, capacity, &got, fi);
        decode_ticks += svcGetSystemTick() - t0;
        in_off += (u32)used;
        data_pos += (u32)used;
        if (r == OPENDSP_AAC_ERR_NEED_MORE_DATA) {
            if (in_eof)
                return 0;
            aac_read();
            continue;
        }
        if (R_FAILED(r) || !got) {
            if (r == OPENDSP_AAC_ERR_UNSUPPORTED && opendspAacGetBackend(dec) == OPENDSP_AAC_BACKEND_DSP) {
                snprintf(stream_error, sizeof(stream_error), "DSP AAC: LC, 8-48 kHz, one block, <=2048 B");
                return 0;
            }
            if (R_FAILED(r) && !used) {
                snprintf(stream_error, sizeof(stream_error), "AAC stopped: %08lX (%s)", (unsigned long)r,
                         r == OPENDSP_AAC_ERR_BUSY      ? "DSP resource limit"
                         : r == OPENDSP_AAC_ERR_TIMEOUT ? "DSP timeout"
                                                        : "decoder error");
                return 0;
            }
            continue; // skipped junk or a bad frame
        }
        if (fi->flags & OPENDSP_AAC_INFO_DISCONTINUITY) {
            snprintf(stream_error, sizeof(stream_error), "DSP resumed: press A to restart the track");
            return 0;
        }
        decoded_frames += got;
        return (u32)got;
    }
}

static u32 fill_aac(int k)
{
    s16 *out = (s16 *)mem[k];
    u32 frames = 0;
    bool dsp = opendspAacGetBackend(dec) == OPENDSP_AAC_BACKEND_DSP;
    u32 limit = dsp ? 1024 : BUF_FRAMES, block = dsp ? 1024 : AAC_MAX_OUT;
    opendspAacInfo fi;
    while (frames + block <= limit) {
        u32 n = aac_frame(out + frames * chans, limit - frames, &fi);
        if (!n || fi.channels != chans)
            break;
        frames += n;
    }
    DSP_FlushDataCache(mem[k], frames * chans * 2);
    return frames;
}

static void queue_free_buffers(void)
{
    for (int k = 0; k < NBUF; k++) {
        if (stop_requested)
            return;
        if (wb[k].status != NDSP_WBUF_FREE && wb[k].status != NDSP_WBUF_DONE)
            continue;
        if (at_eof)
            continue;
        u32 n = aac ? fill_aac(k) : fill(k);
        if (stop_requested)
            return;
        if (!n) {
            at_eof = true;
            continue;
        }
        wb[k].nsamples = n;
        ndspChnWaveBufAdd(CH_WAV, &wb[k]);
    }
    if (at_eof) {
        bool busy = false;
        for (int k = 0; k < NBUF; k++)
            if (wb[k].status == NDSP_WBUF_QUEUED || wb[k].status == NDSP_WBUF_PLAYING)
                busy = true;
        if (!busy)
            playing = false;
    }
}

static void stream_main(void *arg)
{
    (void)arg;
    while (run) {
        LightEvent_Wait(&stream_event);
        if (!run)
            break;
        if (stop_requested)
            continue;
        LightLock_Lock(&lk);
        if (!stop_requested && playing && fp)
            queue_free_buffers();
        LightLock_Unlock(&lk);
    }
}
void wav_frame_notify(void)
{
    if (stream_event_ready)
        LightEvent_Signal(&stream_event);
}

void wav_apply(void)
{
    if (!rate)
        return;
    float r = rate * (S.speed / 100.f);
    if (r > 4 * 32728.f)
        r = 4 * 32728.f;
    ndspChnSetRate(CH_WAV, r);
    ndspChnSetInterp(CH_WAV, S.interp);
}

// reads the first frame for rate and channels, then starts over
static bool open_aac(char *err, size_t errlen)
{
    opendspAacInfo fi;
    if (!dec) {
        opendspAacConfig config = {sizeof(config), requested_backend, 0};
        Result rc = opendspAacOpen(&dec, &config);
        if (R_FAILED(rc)) {
            snprintf(err, errlen, "AAC %s unavailable: %08lX",
                     requested_backend == OPENDSP_AAC_BACKEND_DSP ? "DSP" : "ARM11", (unsigned long)rc);
            return false;
        }
    }
    if (!aac_in && !(aac_in = malloc(AAC_IN_SIZE))) {
        snprintf(err, errlen, "no memory for the AAC decoder");
        return false;
    }
    fseek(fp, 0, SEEK_END);
    data_len = (u32)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    opendspAacReset(dec);
    in_len = in_off = 0;
    in_eof = false;
    data_pos = 0;
    bool loop = looping;
    looping = false;
    u32 n = aac_frame(probe_pcm, AAC_MAX_OUT, &fi);
    looping = loop;
    if (!n) {
        snprintf(err, errlen, "%s", stream_error[0] ? stream_error : "no AAC-LC frames found");
        return false;
    }
    rate = fi.sample_rate;
    chans = fi.channels;
    bits = 16;
    frame_bytes = chans * 2;
    if (rate > 4 * 32728) {
        snprintf(err, errlen, "sample rate %lu Hz not supported", rate);
        return false;
    }
    opendspAacReset(dec);
    fseek(fp, 0, SEEK_SET);
    in_len = in_off = 0;
    in_eof = false;
    data_pos = 0;
    decode_ticks = decoded_frames = 0;
    return true;
}

static void close_file(void)
{
    ndspChnWaveBufClear(CH_WAV);
    if (fp) {
        fclose(fp);
        fp = NULL;
    }
    playing = false;
}

bool wav_play(int i, char *err, size_t errlen)
{
    if (th)
        wav_stop();
    if (!th) {
        LightLock_Init(&lk);
        LightEvent_Init(&stream_event, RESET_ONESHOT);
        for (int k = 0; k < NBUF; k++) {
            mem[k] = linearAlloc(BUF_FRAMES * 4);
            if (!mem[k]) {
                for (int j = 0; j < k; j++) {
                    linearFree(mem[j]);
                    mem[j] = NULL;
                }
                snprintf(err, errlen, "no memory for audio buffers");
                return false;
            }
        }
        run = true;
        s32 prio = 0x30;
        svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
        th = threadCreate(stream_main, NULL, 16 * 1024, prio - 1, -2, false);
        if (!th) {
            run = false;
            for (int k = 0; k < NBUF; k++) {
                linearFree(mem[k]);
                mem[k] = NULL;
            }
            snprintf(err, errlen, "cannot start audio thread");
            return false;
        }
        stream_event_ready = true;
    }
    LightLock_Lock(&lk);
    close_file();
    stop_requested = false;
    stream_error[0] = 0;
    char path[300];
    const char *name = wav_name(i);
    size_t l = strlen(name);
    snprintf(path, sizeof path, "%s/%s", WAV_DIR, name);
    fp = fopen(path, "rb");
    if (!fp) {
        snprintf(err, errlen, "cannot open %s", name);
        LightLock_Unlock(&lk);
        return false;
    }
    aac = l > 4 && !strcasecmp(name + l - 4, ".aac");
    if (aac ? !open_aac(err, errlen) : !parse(err, errlen)) {
        close_file();
        LightLock_Unlock(&lk);
        return false;
    }
    snprintf(info, sizeof info, "%lu Hz %s %s", rate, chans == 2 ? "stereo" : "mono",
             aac ? (opendspAacGetBackend(dec) == OPENDSP_AAC_BACKEND_DSP ? "AAC / Teak DSP" : "AAC / ARM11")
             : bits == 16 ? "PCM16"
                          : "PCM8");
    u16 fmt = bits == 16 ? (chans == 2 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16)
                         : (chans == 2 ? NDSP_FORMAT_STEREO_PCM8 : NDSP_FORMAT_MONO_PCM8);
    ndspChnWaveBufClear(CH_WAV);
    ndspChnSetFormat(CH_WAV, fmt);
    ndspChnSetPaused(CH_WAV, true);
    wav_apply();
    audio_set_mix(CH_WAV, 1.0f, 1.0f);
    for (int k = 0; k < NBUF; k++) {
        memset(&wb[k], 0, sizeof wb[k]);
        wb[k].data_vaddr = mem[k];
    }
    at_eof = false;
    playing = true;
    queue_free_buffers();
    ndspChnSetPaused(CH_WAV, false);
    if (!playing && stream_error[0]) {
        snprintf(err, errlen, "%s", stream_error);
        close_file();
        LightLock_Unlock(&lk);
        return false;
    }
    LightLock_Unlock(&lk);
    LightEvent_Signal(&stream_event);
    return true;
}

void wav_stop(void)
{
    if (!th)
        return;
    // Publish cancellation before waiting for the higher-priority worker.
    stop_requested = true;
    __dsb();
    LightEvent_Signal(&stream_event);
    LightLock_Lock(&lk);
    close_file();
    rate = 0;
    LightLock_Unlock(&lk);
}
bool wav_playing(void)
{
    return playing;
}
void wav_set_loop(bool l)
{
    looping = l;
}
bool wav_loop(void)
{
    return looping;
}
const char *wav_info(void)
{
    return playing ? info : "";
}

float wav_decode_load(void)
{
    if (dec && opendspAacGetBackend(dec) == OPENDSP_AAC_BACKEND_DSP)
        return -1;
    if (!aac || !playing || !decoded_frames || !rate)
        return -1;
    return (float)decode_ticks / ((float)decoded_frames / rate * SYSCLOCK_ARM11);
}
unsigned wav_backend(void)
{
    return requested_backend;
}
unsigned wav_active_backend(void)
{
    return playing && aac && dec ? opendspAacGetBackend(dec) : OPENDSP_AAC_BACKEND_AUTO;
}
const char *wav_error(void)
{
    return stream_error;
}
void wav_set_backend(unsigned backend)
{
    if (backend != OPENDSP_AAC_BACKEND_ARM11 && backend != OPENDSP_AAC_BACKEND_DSP)
        return;
    wav_stop();
    if (th)
        LightLock_Lock(&lk);
    opendspAacClose(dec);
    dec = NULL;
    requested_backend = backend;
    stream_error[0] = 0;
    if (th)
        LightLock_Unlock(&lk);
}
float wav_progress(void)
{
    return data_len ? (float)data_pos / data_len : 0;
}

void wav_exit(void)
{
    if (!th)
        return;
    wav_stop();
    run = false;
    stream_event_ready = false;
    LightEvent_Signal(&stream_event);
    threadJoin(th, U64_MAX);
    threadFree(th);
    th = NULL;
    for (int k = 0; k < NBUF; k++)
        linearFree(mem[k]);
    opendspAacClose(dec);
    dec = NULL;
    free(aac_in);
    aac_in = NULL;
}
