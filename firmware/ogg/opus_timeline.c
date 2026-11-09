// SPDX-License-Identifier: BSD-2-Clause
#include "opus_timeline.h"

typedef struct { u32 low, high; } frame_position;

static __attribute__((always_inline)) inline frame_position split_position(uint64_t value)
{
    frame_position position = {(u32)value, (u32)(value >> 32)};
    return position;
}

static __attribute__((always_inline)) inline uint64_t join_position(frame_position position)
{
    return ((uint64_t)position.high << 32) | position.low;
}

static __attribute__((always_inline)) inline int position_before(frame_position first, frame_position last)
{
    return first.high < last.high || (first.high == last.high && first.low < last.low);
}

static __attribute__((always_inline)) inline frame_position advance_position(frame_position position, u16 frames)
{
    u32 next = position.low + frames;
    position.high += next < position.low;
    position.low = next;
    return position;
}

static __attribute__((always_inline)) inline frame_position rewind_position(frame_position position, u16 frames)
{
    position.high -= position.low < frames;
    position.low -= frames;
    return position;
}

int ogg_opus_timeline_open(ogg_opus_timeline *timeline, u16 pre_skip, uint64_t total,
                           uint64_t first, uint64_t last, uint64_t decode_start)
{
    frame_position total_position = split_position(total), first_position = split_position(first);
    frame_position last_position = split_position(last), start_position = split_position(decode_start), first_raw;
    if (!timeline) return -1;
    timeline->failed = 1;
    if (total_position.high > 0x7fffffffUL ||
        (total_position.high == 0x7fffffffUL && total_position.low > ~(u32)0 - pre_skip) ||
        position_before(last_position, first_position) || position_before(total_position, last_position))
        return -1;
    first_raw = advance_position(first_position, pre_skip);
    if (position_before(first_raw, start_position) ||
        ((start_position.high || start_position.low) && position_before(first_raw, advance_position(start_position, 3840))))
        return -1;
    timeline->decoded_frame = decode_start;
    timeline->first_raw = join_position(first_raw);
    timeline->last_raw = join_position(advance_position(last_position, pre_skip));
    timeline->source_frame = first;
    timeline->pre_skip = pre_skip;
    timeline->failed = 0;
    return 0;
}

int ogg_opus_timeline_slice(ogg_opus_timeline *timeline, u16 frames, ogg_opus_span *span)
{
    frame_position decoded, next, begin, end, first_raw, last_raw, source;
    if (span) {
        span->source_frame = 0;
        span->skip = span->frames = 0;
    }
    if (!timeline || !span || timeline->failed) return -1;
    span->source_frame = timeline->source_frame;
    decoded = split_position(timeline->decoded_frame);
    next = advance_position(decoded, frames);
    if (frames > 5760 || position_before(next, decoded)) {
        timeline->failed = 1;
        return -1;
    }
    first_raw = split_position(timeline->first_raw);
    last_raw = split_position(timeline->last_raw);
    begin = position_before(decoded, first_raw) ? first_raw : decoded;
    end = position_before(next, last_raw) ? next : last_raw;
    if (position_before(begin, end)) {
        // Packet spans fit sixteen bits.
        span->skip = (u16)(begin.low - decoded.low);
        span->frames = (u16)(end.low - begin.low);
        source = rewind_position(begin, timeline->pre_skip);
        span->source_frame = join_position(source);
        timeline->source_frame = join_position(advance_position(source, span->frames));
    }
    timeline->decoded_frame = join_position(next);
    return 0;
}

int ogg_opus_timeline_finish(ogg_opus_timeline *timeline)
{
    frame_position source, last;
    if (!timeline || timeline->failed) return -1;
    source = split_position(timeline->source_frame);
    last = rewind_position(split_position(timeline->last_raw), timeline->pre_skip);
    if (source.high != last.high || source.low != last.low) {
        timeline->failed = 1;
        return -1;
    }
    return 0;
}
