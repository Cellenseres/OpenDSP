#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Convert MSU-1 PCM tracks to OPUZ or Ogg Opus with LOOPSTART.

Needs ffmpeg with libopus. Copies the pack's .msu data file, nothing else.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

TRACK = re.compile(r"^(.*)-(\d+)\.pcm$", re.I)


def ogg_packets(data):
    """Packets of the first stream, with the granule of their last page."""
    packets, pending, at, serial = [], bytearray(), 0, None
    while at < len(data):
        if data[at:at + 4] != b"OggS":
            raise ValueError("bad Ogg page")
        granule, page_serial = struct.unpack_from("<qI", data, at + 6)
        count = data[at + 26]
        lacing = data[at + 27:at + 27 + count]
        at += 27 + count
        if serial is None:
            serial = page_serial
        for size in lacing:
            if page_serial == serial:
                pending += data[at:at + size]
            at += size
            if size < 255 and page_serial == serial:
                packets.append((bytes(pending), granule))
                pending = bytearray()
    return packets


def packet_samples(packet):
    toc = packet[0]
    config = toc >> 3
    if config < 12:
        size = (480, 960, 1920, 2880)[config & 3]
    elif config < 16:
        size = (480, 960)[config & 1]
    else:
        size = (120, 240, 480, 960)[config & 3]
    code = toc & 3
    count = 1 if code == 0 else 2 if code < 3 else packet[1] & 0x3F
    return size * count


def ogg_to_opuz(ogg, loop):
    """OPUZ as zelda3 writes it; loop None means no repeat range."""
    packets = ogg_packets(ogg)
    head = packets[0][0]
    if head[:8] != b"OpusHead" or packets[1][0][:8] != b"OpusTags":
        raise ValueError("not an Ogg Opus stream")
    pre_skip = struct.unpack_from("<H", head, 10)[0]
    audio = packets[2:]
    total = audio[-1][1] - pre_skip
    body, frames, sample, celt = bytearray(), [], 0, 0
    for packet, _ in audio:
        frames.append((sample, len(body)))
        sample += packet_samples(packet)
        celt += packet[0] >> 3 >= 16
        payload, code = packet, len(packet)
        if packet[0] == 0xFC:
            payload, code = packet[1:], (len(packet) - 1) | 0x8000
        if len(payload) > 1275:
            raise ValueError("Opus packet too large")
        body += struct.pack("<H", code) + payload
    ranges = [(0, False)] + ([(loop, True)] if loop is not None else [])
    header = b"OPUZ" + struct.pack("<I", 0)
    data = len(header) + 10 * len(ranges)
    for i, (start, repeat) in enumerate(ranges):
        frame = max(f for f in frames if f[0] <= start)
        skip = pre_skip + start - frame[0]
        if skip >= 0x4000:
            raise ValueError("pre-skip too large")
        flags = skip | (0x4000 if repeat else 0) | (0x8000 if i == len(ranges) - 1 else 0)
        header += struct.pack("<IIH", frame[1] + data, total - start, flags)
    return header + body, celt == len(audio)


def encode(ffmpeg, pcm, bitrate, loop48):
    with open(pcm, "rb") as f:
        tagged = f.read(4) == b"MSU1"
    base = [ffmpeg, "-v", "error", "-skip_initial_bytes", "8" if tagged else "0", "-f", "s16le",
            "-ar", "44100", "-ac", "2", "-i", str(pcm)]
    tail = ["-c:a", "libopus", "-b:a", f"{bitrate}k", "-vbr", "on", "-application", "audio",
            "-frame_duration", "20", "-compression_level", "10", "-metadata", f"LOOPSTART={loop48}",
            "-fflags", "+bitexact", "-flags:a", "+bitexact", "-f", "ogg", "pipe:1"]
    for resample in ("aresample=48000:resampler=soxr:precision=28", "aresample=48000"):
        result = subprocess.run([*base, "-af", resample, *tail], capture_output=True)
        if result.returncode == 0:
            return result.stdout
    raise RuntimeError(result.stderr.decode(errors="replace"))


def convert(ffmpeg, pcm, output, fmt, bitrate):
    data = pcm.read_bytes()
    tagged = data[:4] == b"MSU1"
    frames = (len(data) - (8 if tagged else 0)) // 4
    loop44 = struct.unpack_from("<I", data, 4)[0] if tagged else 0
    if loop44 >= frames:
        loop44 = 0
    loop48 = loop44 * 48000 // 44100
    ogg = encode(ffmpeg, pcm, bitrate, loop48)
    if fmt == "ogg":
        path = output / (pcm.stem + ".opus")
        path.write_bytes(ogg)
        return path, True
    opuz, celt = ogg_to_opuz(ogg, loop48)
    path = output / (pcm.stem + ".opuz")
    path.write_bytes(opuz)
    return path, celt


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("source", type=Path, help="pack directory or <prefix>")
    parser.add_argument("output", type=Path)
    parser.add_argument("--format", choices=("opuz", "ogg"), default="opuz")
    parser.add_argument("--bitrate", type=int, default=128, help="kbit/s, default 128")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    args = parser.parse_args()
    ffmpeg = shutil.which("ffmpeg")
    if not ffmpeg:
        parser.error("needs ffmpeg with libopus")
    folder = args.source if args.source.is_dir() else args.source.parent
    prefix = None if args.source.is_dir() else args.source.name
    tracks = [p for p in sorted(folder.iterdir()) if TRACK.match(p.name) and
              (prefix is None or TRACK.match(p.name).group(1) == prefix)]
    if not tracks:
        parser.error("no <name>-<N>.pcm tracks found")
    args.output.mkdir(parents=True, exist_ok=True)
    with ThreadPoolExecutor(args.jobs) as pool:
        jobs = [pool.submit(convert, ffmpeg, p, args.output, args.format, args.bitrate) for p in tracks]
        hybrid = 0
        for pcm, job in zip(tracks, jobs):
            path, celt = job.result()
            hybrid += not celt
            print(f"{path.name}: {pcm.stat().st_size // 1024} KiB -> {path.stat().st_size // 1024} KiB")
    for base in {TRACK.match(p.name).group(1) for p in tracks}:
        data = folder / (base + ".msu")
        if data.exists():
            shutil.copyfile(data, args.output / data.name)
    if hybrid:
        print(f"{hybrid} tracks use SILK; zelda3 decodes CELT only", file=sys.stderr)


if __name__ == "__main__":
    main()
