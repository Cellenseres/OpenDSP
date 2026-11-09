/* SPDX-License-Identifier: BSD-2-Clause */
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ogg/ogg.h>
#include "ogg.h"
#include "../internal.h"

static u32 little32(const u8 *p)
{
    return p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
}

void audio_loop_tag_init(audio_loop_tags *tags)
{
    memset(tags, 0, sizeof(*tags));
}

static bool field_name(const char *comment, size_t bytes, const char *name)
{
    size_t n = strlen(name);
    if (bytes < n + 1 || comment[n] != '=')
        return false;
    for (size_t i = 0; i < n; ++i) {
        unsigned char ch = (unsigned char)comment[i];
        if (ch >= 'a' && ch <= 'z')
            ch -= 'a' - 'A';
        if (ch != (unsigned char)name[i])
            return false;
    }
    return true;
}

void audio_loop_tag_add(audio_loop_tags *tags, const char *comment, size_t length)
{
    u64 *value;
    bool *present;
    size_t prefix;
    if (field_name(comment, length, "LOOPSTART")) {
        value = &tags->start; present = &tags->has_start; prefix = 10;
    } else if (field_name(comment, length, "LOOPEND")) {
        value = &tags->end; present = &tags->has_end; prefix = 8;
    } else if (field_name(comment, length, "LOOPLENGTH")) {
        value = &tags->length; present = &tags->has_length; prefix = 11;
    } else
        return;
    tags->present = true;
    u64 parsed = 0;
    if (length == prefix) {
        tags->invalid = true;
        return;
    }
    for (size_t i = prefix; i < length; ++i) {
        unsigned digit = (unsigned char)comment[i] - '0';
        if (digit > 9 || parsed > (UINT64_MAX - digit) / 10) {
            tags->invalid = true;
            return;
        }
        parsed = parsed * 10 + digit;
    }
    if (*present && *value != parsed)
        tags->invalid = true;
    *present = true;
    *value = parsed;
}

void audio_loop_tag_finish(const audio_loop_tags *tags, u64 total, opendspAudioLoopInfo *info)
{
    memset(info, 0, sizeof(*info));
    info->struct_size = sizeof(*info);
    if (!tags->present)
        return;
    info->from_tags = true;
    info->status = OPENDSP_AUDIO_LOOP_INVALID;
    if (tags->invalid || !tags->has_start)
        return;
    u64 end = tags->has_end ? tags->end : total;
    if (tags->has_length) {
        if (tags->length > UINT64_MAX - tags->start)
            return;
        u64 derived = tags->start + tags->length;
        if (tags->has_end && end != derived)
            return;
        end = derived;
    }
    if (tags->start >= end || end > total)
        return;
    info->start_frame = tags->start;
    info->end_frame = end;
    info->available = true;
    info->status = OPENDSP_AUDIO_LOOP_VALID;
}

int audio_ogg_read(audio_ogg_io *io, void *out, size_t bytes)
{
    size_t got;
    Result r = opendsp_audio_input_read(&io->codec->in, out, bytes, &got);
    if (R_FAILED(r)) {
        io->error = r;
        return -1;
    }
    return (int)got;
}

int audio_ogg_seek(audio_ogg_io *io, s64 offset, int whence)
{
    s64 at = (s64)opendsp_audio_input_tell(&io->codec->in);
    s64 base;
    if (whence == SEEK_SET)
        base = 0;
    else if (whence == SEEK_CUR)
        base = at;
    else if (whence == SEEK_END)
        base = io->codec->in.size;
    else
        return -1;
    if (offset < -base || offset > (s64)io->codec->in.size - base)
        return -1;
    Result r = opendsp_audio_input_seek(&io->codec->in, (u64)(base + offset));
    if (R_FAILED(r))
        io->error = r;
    return R_FAILED(r) ? -1 : 0;
}

Result audio_ogg_error(audio_ogg_io *io, Result fallback)
{
    return R_FAILED(io->error) ? io->error : fallback;
}

static Result exact(audio_codec *c, void *out, size_t bytes)
{
    size_t got;
    Result r = opendsp_audio_input_read(&c->in, out, bytes, &got);
    return R_FAILED(r) ? r : got == bytes ? 0 : OPENDSP_AUDIO_ERR_TRUNCATED;
}

static Result comments(audio_loop_tags *tags, const u8 *p, size_t size, bool vorbis)
{
    size_t prefix = vorbis ? 7 : 8;
    if (size < prefix + 8 || (vorbis ? memcmp(p, "\003vorbis", 7) : memcmp(p, "OpusTags", 8)))
        return OPENDSP_AUDIO_ERR_FORMAT;
    if (size > AUDIO_OGG_COMMENT_LIMIT)
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    p += prefix;
    size -= prefix;
    u32 vendor = little32(p);
    if (vendor > size - 8)
        return OPENDSP_AUDIO_ERR_FORMAT;
    p += 4 + vendor;
    size -= 4 + vendor;
    u32 count = little32(p);
    p += 4; size -= 4;
    if (count > AUDIO_OGG_COMMENT_COUNT)
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    for (u32 i = 0; i < count; ++i) {
        if (size < 4)
            return OPENDSP_AUDIO_ERR_FORMAT;
        u32 bytes = little32(p);
        p += 4; size -= 4;
        if (bytes > size)
            return OPENDSP_AUDIO_ERR_FORMAT;
        audio_loop_tag_add(tags, (const char *)p, bytes);
        p += bytes; size -= bytes;
    }
    if (vorbis && (!size || !(p[0] & 1)))
        return OPENDSP_AUDIO_ERR_FORMAT;
    return 0;
}

static Result identification(const u8 *packet, u32 bytes, bool vorbis)
{
    if (vorbis) {
        if (bytes != 30 || !(packet[29] & 1))
            return OPENDSP_AUDIO_ERR_FORMAT;
        if (little32(packet + 7) || (packet[11] != 1 && packet[11] != 2))
            return OPENDSP_AUDIO_ERR_UNSUPPORTED;
        u32 rate = little32(packet + 12);
        if (rate < 1000 || rate > 130912)
            return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    } else {
        if (bytes < 19)
            return OPENDSP_AUDIO_ERR_FORMAT;
        if (packet[8] > 15 || (packet[9] != 1 && packet[9] != 2) || packet[18])
            return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    }
    return 0;
}

/* Find the page ending the file. */
static Result final_page(audio_codec *c, u8 *page, u64 start, u32 serial, u32 sequence, u64 granule)
{
    u64 span = c->in.size - start;
    for (u32 want = 8192;; want = 65307) {
        u32 bytes = span < want ? (u32)span : want;
        if (bytes < 27)
            return OPENDSP_AUDIO_ERR_TRUNCATED;
        Result r = opendsp_audio_input_seek(&c->in, c->in.size - bytes);
        if (R_SUCCEEDED(r))
            r = exact(c, page, bytes);
        if (R_FAILED(r))
            return r;
        for (u32 at = bytes - 27 + 1; at--;) {
            u8 *p = page + at;
            if (memcmp(p, "OggS", 4) || p[4] || (p[5] & ~7))
                continue;
            u32 segments = p[26], body = 0;
            if (at + 27 + segments > bytes)
                continue;
            for (u32 i = 0; i < segments; ++i) body += p[27 + i];
            if (at + 27 + segments + body != bytes)
                continue;
            u8 crc[4];
            ogg_page og = {p, 27 + segments, p + 27 + segments, body};
            memcpy(crc, p + 22, 4);
            memset(p + 22, 0, 4);
            ogg_page_checksum_set(&og);
            if (memcmp(crc, p + 22, 4)) {
                memcpy(p + 22, crc, 4);
                continue;
            }
            u64 last = little32(p + 6) | (u64)little32(p + 10) << 32;
            if (little32(p + 14) != serial || (p[5] & 2))
                return OPENDSP_AUDIO_ERR_UNSUPPORTED;
            if (!(p[5] & 4))
                return OPENDSP_AUDIO_ERR_TRUNCATED;
            if (little32(p + 18) < sequence || last == UINT64_MAX || last > INT64_MAX || last < granule ||
                (segments && p[26 + segments] == 255))
                return OPENDSP_AUDIO_ERR_FORMAT;
            return 0;
        }
        if (bytes == span || want == 65307)
            return OPENDSP_AUDIO_ERR_TRUNCATED;
    }
}

/* Check headers and the final page. */
static Result inspect(audio_codec *c, audio_loop_tags *tags, bool *vorbis, audio_vorbis_start *start)
{
    u8 *page = malloc(65307), *packet = malloc(AUDIO_OGG_PACKET_LIMIT);
    if (!page || !packet) {
        free(page); free(packet);
        return OPENDSP_AUDIO_ERR_MEMORY;
    }
    Result r = opendsp_audio_input_seek(&c->in, 0);
    u64 offset = 0, last_granule = 0;
    u32 serial = 0, sequence = 0, packets = 0, packet_bytes = 0, header_bytes = 0;
    u8 id[30];
#ifndef OPENDSP_CODEC_BANKS
    (void)start;
#endif
    bool eos = false;
    while (R_SUCCEEDED(r) && offset < c->in.size) {
        r = exact(c, page, 27);
        if (R_FAILED(r)) break;
        if (memcmp(page, "OggS", 4) || page[4] || (page[5] & ~7)) {
            r = OPENDSP_AUDIO_ERR_FORMAT; break;
        }
        u32 page_serial = little32(page + 14), page_sequence = little32(page + 18);
        if (offset && (eos || page_serial != serial || (page[5] & 2))) {
            r = OPENDSP_AUDIO_ERR_UNSUPPORTED; break;
        }
        if (!offset) {
            if (!(page[5] & 2) || (page[5] & 1) || page_sequence) {
                r = OPENDSP_AUDIO_ERR_FORMAT; break;
            }
            serial = page_serial;
        }
        if (page_sequence != sequence++ || !!(page[5] & 1) != !!packet_bytes) {
            r = OPENDSP_AUDIO_ERR_FORMAT; break;
        }
        u32 segments = page[26], body = 0;
        r = exact(c, page + 27, segments);
        if (R_FAILED(r)) break;
        for (u32 i = 0; i < segments; ++i) body += page[27 + i];
        r = exact(c, page + 27 + segments, body);
        if (R_FAILED(r)) break;
        u32 crc = little32(page + 22);
        ogg_page og = {page, 27 + segments, page + 27 + segments, body};
        memset(page + 22, 0, 4);
        ogg_page_checksum_set(&og);
        if (crc != little32(page + 22)) {
            r = OPENDSP_AUDIO_ERR_FORMAT; break;
        }
        u64 granule = little32(page + 6) | (u64)little32(page + 10) << 32;
        if (granule != UINT64_MAX) {
            if (granule > INT64_MAX || granule < last_granule) {
                r = OPENDSP_AUDIO_ERR_FORMAT; break;
            }
            last_granule = granule;
        }
        u32 data_offset = 27 + segments;
        for (u32 i = 0; i < segments; ++i) {
            u32 bytes = page[27 + i];
            if (packet_bytes > AUDIO_OGG_PACKET_LIMIT - bytes) {
                r = OPENDSP_AUDIO_ERR_UNSUPPORTED; break;
            }
            bool header = packets < (*vorbis ? 3u : 2u);
            if (header)
                memcpy(packet + packet_bytes, page + data_offset, bytes);
            packet_bytes += bytes;
            data_offset += bytes;
            if (bytes == 255) continue;
            if (!packets) {
                if (packet_bytes >= 7 && !memcmp(packet, "\001vorbis", 7))
                    *vorbis = true;
                else if (packet_bytes >= 8 && !memcmp(packet, "OpusHead", 8))
                    *vorbis = false;
                else { r = OPENDSP_AUDIO_ERR_UNSUPPORTED; break; }
                r = identification(packet, packet_bytes, *vorbis);
                if (R_FAILED(r)) break;
                if (*vorbis) memcpy(id, packet, sizeof(id));
            }
            if (packets < (*vorbis ? 3u : 2u)) {
                header_bytes += packet_bytes;
                if (header_bytes > AUDIO_OGG_PACKET_LIMIT) {
                    r = OPENDSP_AUDIO_ERR_UNSUPPORTED; break;
                }
            }
            if (packets == 1) {
                r = comments(tags, packet, packet_bytes, *vorbis);
                if (R_FAILED(r)) break;
            }
            if (packets == 2 && *vorbis &&
                (packet_bytes < 7 || memcmp(packet, "\005vorbis", 7))) {
                r = OPENDSP_AUDIO_ERR_FORMAT; break;
            }
#ifdef OPENDSP_CODEC_BANKS
            // Setup overlaps the reads below.
            if (packets == 2 && *vorbis && start)
                audio_vorbis_start_dsp(start, c, id, packet, packet_bytes);
#endif
            ++packets;
            packet_bytes = 0;
        }
        eos = !!(page[5] & 4);
        offset += 27 + segments + body;
        if (eos && (packet_bytes || granule == UINT64_MAX))
            r = OPENDSP_AUDIO_ERR_FORMAT;
        // Full scans took 3.6 s on HW.
        if (R_SUCCEEDED(r) && !eos && packets >= (*vorbis ? 3u : 2u)) {
            // First packet's TOC guides AUTO.
            if (!*vorbis && R_SUCCEEDED(exact(c, page, 27)) && !memcmp(page, "OggS", 4) && page[26] &&
                R_SUCCEEDED(exact(c, page + 27, page[26])) && page[27] &&
                R_SUCCEEDED(exact(c, page + 27 + page[26], 1))) {
                c->first_toc = page[27 + page[26]];
                c->first_toc_known = true;
            }
            r = final_page(c, page, offset, serial, sequence, last_granule);
            eos = true;
            packet_bytes = 0;
            break;
        }
    }
    if (R_SUCCEEDED(r) && (!eos || packet_bytes || packets < (*vorbis ? 3u : 2u)))
        r = OPENDSP_AUDIO_ERR_TRUNCATED;
    free(packet); free(page);
    return R_FAILED(r) ? r : opendsp_audio_input_seek(&c->in, 0);
}

Result opendsp_audio_ogg_open(audio_codec *c, u32 backend)
{
    if (backend != OPENDSP_AAC_BACKEND_AUTO && backend != OPENDSP_AAC_BACKEND_ARM11 &&
        backend != OPENDSP_AAC_BACKEND_DSP)
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    audio_loop_tags tags;
    audio_loop_tag_init(&tags);
    bool vorbis = false;
    audio_vorbis_start start = {0};
    Result r = inspect(c, &tags, &vorbis, backend != OPENDSP_AAC_BACKEND_ARM11 ? &start : NULL);
    if (R_FAILED(r)) {
#ifdef OPENDSP_CODEC_BANKS
        audio_vorbis_start_cancel(&start, c);
#endif
        opendsp_codec_error("Ogg container validation", r);
        return r;
    }
#ifndef OPENDSP_CODEC_BANKS
    if (vorbis && backend == OPENDSP_AAC_BACKEND_DSP) return OPENDSP_AUDIO_ERR_UNSUPPORTED;
#endif
    return vorbis ? opendsp_audio_vorbis_open(c, &tags, backend, &start) : opendsp_audio_opus_open(c, &tags, backend);
}
