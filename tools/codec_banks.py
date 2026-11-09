#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Pack decoder code and data."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

DATA_BASE = 0x10000000
DATA_SECTIONS = {".decoder", ".decoder_y_low", ".decoder_y_high", ".probe_y_tables",
                 ".opus_packet_state", ".opus_window_state"}
PROFILE_SECTIONS = {".vorbis_decoder": 2}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def stamp_identity(image):
    image = bytearray(image)
    header = struct.unpack_from("<16sHHIIIIIHHHHHH", image)
    sections = [struct.unpack_from("<10I", image, header[6] + index * header[11]) for index in range(header[12])]
    locations = []
    for section in sections:
        if section[1] != 2:
            continue
        strings = sections[section[6]]
        names = image[strings[4]:strings[4] + strings[5]]
        for offset in range(section[4], section[4] + section[5], section[9]):
            name, address, size, _, _, index = struct.unpack_from("<IIIBBH", image, offset)
            if names[name:].split(b"\0", 1)[0] != b"codec_bank_identity":
                continue
            target = sections[index]
            assert size == 8 and target[1] == 1
            location = target[4] + address - target[3]
            assert target[4] <= location <= target[4] + target[5] - 8
            locations.append(location)
    assert len(locations) == 1, "Bank identity symbol"
    location = locations[0]
    image[location:location + 8] = bytes(8)
    fingerprint = hashlib.sha256(image).digest()[:8]
    tags = [int.from_bytes(fingerprint[index:index + 4], "big") for index in [0, 4]]
    image[location:location + 8] = struct.pack("<4H", tags[0] >> 16, tags[0] & 65535, tags[1] >> 16, tags[1] & 65535)
    return bytes(image), tags


def pack(image, manifest):
    assert digest(image) == manifest["image"], "Image differs from manifest"
    assert manifest["legacy_codecs"] and manifest["framed_input"]
    assert len(image) >= 52 and image[:7] == b"\x7fELF\x01\x01\x01", "ELF format"
    header = struct.unpack_from("<16sHHIIIIIHHHHHH", image)
    assert header[9] == 32 and header[11] == 40, "ELF table sizes"
    assert header[5] + header[9] * header[10] <= len(image)
    assert header[6] + header[11] * header[12] <= len(image)
    window = manifest["coexist_layout"]["window"]
    assert 0 < window < 0x40000 and window % 2 == 0
    payload, banks, blocks = bytearray(), [], []

    def append(data):
        offset = len(payload)
        assert offset % 128 == 0 and len(data) % 2 == 0
        payload.extend(data)
        payload.extend(b"\xa5" * ((-len(payload)) % 128))
        return dict(offset=offset, bytes=len(data), sha256=digest(data))

    resident = 0
    for index in range(header[10]):
        kind, offset, address, _, size, memory, _, _ = struct.unpack_from("<8I", image, header[5] + index * header[9])
        if kind != 1:
            continue
        assert size <= memory and offset + size <= len(image)
        if address == window:
            assert size == memory and 0 < size <= 0x40000 - window
            banks.append(dict(id=len(banks), **append(image[offset:offset + size])))
        elif address < DATA_BASE:
            assert address + memory <= window, "Resident overlaps code banks"
            resident = max(resident, address + memory)
    assert 0 < resident <= window
    assert len(banks) == manifest.get("bank_count", manifest["mp3_bank"] + 1) and len(banks) <= 16
    assert 0 <= manifest["aac_bank"] < manifest["mp3_bank"] < len(banks)
    sections = [struct.unpack_from("<10I", image, header[6] + index * header[11]) for index in range(header[12])]
    assert header[13] < len(sections)
    strings = sections[header[13]]
    assert strings[4] + strings[5] <= len(image)
    names = image[strings[4]:strings[4] + strings[5]]
    shared = [section for section in sections
              if names[section[0]:].split(b"\0", 1)[0] == b".celt_shared"]
    shared_enabled = manifest.get("opus_layout", {}).get("shared_celt_prefix", False)
    assert bool(shared) == shared_enabled, "CELT prefix manifest"
    if shared_enabled:
        assert len(shared) == 1
        section = shared[0]
        size = section[5]
        assert section[1] == 1 and section[2] & 6 == 6 and section[3] == window
        assert 0 < size < 65536 and not size % 32, "CELT prefix alignment"
        assert size == manifest["opus_layout"]["shared_prefix"]["prefix_bytes"]
        assert section[4] + size <= len(image)
        prefix = image[section[4]:section[4] + size]
        for index in [2, 3]:
            bank = banks[index]
            assert bank["bytes"] > size, "CELT prefix covers bank"
            assert payload[bank["offset"]:bank["offset"] + size] == prefix, "CELT prefix differs"
        values = []
        for symbols in sections:
            if symbols[1] != 2:
                continue
            strings = sections[symbols[6]]
            symbol_names = image[strings[4]:strings[4] + strings[5]]
            for offset in range(symbols[4], symbols[4] + symbols[5], symbols[9]):
                name, address, width, _, _, index = struct.unpack_from("<IIIBBH", image, offset)
                if symbol_names[name:].split(b"\0", 1)[0] != b"ogg_celt_prefix_bytes":
                    continue
                target = sections[index]
                location = target[4] + address - target[3]
                assert target[1] == 1 and names[target[0]:].split(b"\0", 1)[0] == b".decoder", "CELT loader context"
                assert 0x10028800 <= address <= 0x10028bfe, "CELT loader context"
                assert target[4] <= location <= target[4] + target[5] - 2
                values.append(struct.unpack_from("<H", image, location)[0])
        assert values == [size], "CELT loader prefix size"
    for section in sections:
        assert section[0] < len(names)
        name = names[section[0]:].split(b"\0", 1)[0].decode()
        if name not in DATA_SECTIONS | PROFILE_SECTIONS.keys() or not section[5]:
            continue
        _, kind, flags, address, offset, size, _, _, _, _ = section
        assert kind in [1, 8] and flags & 2 and not (address & 1 or size & 1)
        start, end = (address - DATA_BASE) // 2, (address + size - DATA_BASE) // 2
        if name in PROFILE_SECTIONS:
            assert 0x12d80 <= start < end <= 0x14400, "Profile overlaps shared control"
        assert (0x10040 <= start < end <= 0x17000 or 0x18000 <= start < end <= 0x18800 or
                0x1b080 <= start < end <= 0x20000), name
        if kind == 8:
            data = bytes(size)
        else:
            assert offset + size <= len(image)
            data = image[offset:offset + size]
        for previous in blocks:
            first, last = previous["word_address"], previous["word_address"] + previous["bytes"] // 2
            if end <= first or start >= last:
                continue
            pair = {name, previous["name"]}
            assert pair == {".decoder", ".vorbis_decoder"}, "Data blocks overlap"
            override_start, override_end = (start, end) if name == ".vorbis_decoder" else (first, last)
            assert 0x12d80 <= override_start < override_end <= 0x14400, "Profile overlaps shared control"
        block = dict(name=name, word_address=start, **append(data))
        if name in PROFILE_SECTIONS:
            block["profile"] = PROFILE_SECTIONS[name]
        blocks.append(block)
    assert {block["name"] for block in blocks} >= DATA_SECTIONS - {".opus_window_state"}
    report = dict(image_sha256=digest(image), payload_sha256=digest(payload), payload_bytes=len(payload),
                  resident_bytes=resident, window_words=window // 2, code_banks=banks, data_blocks=blocks,
                  aac_bank=manifest["aac_bank"], mp3_bank=manifest["mp3_bank"])
    report["bank_identity"] = manifest.get("bank_identity", [0, 0])
    return bytes(payload), report


def write_header(report):
    lines = ["// SPDX-License-Identifier: BSD-2-Clause", "#pragma once", "#include <stdint.h>",
             "typedef struct { uint32_t offset, bytes; } codec_bank_source;",
             "typedef struct { uint32_t word_address, offset, bytes; } codec_data_source;"]
    for key, value in [("CODEC_PROGRAM_WINDOW", report["window_words"]), ("CODEC_BANK_COUNT", len(report["code_banks"])),
                       ("CODEC_DATA_COUNT", len(report["data_blocks"])), ("CODEC_AAC_BANK", report["aac_bank"]),
                       ("CODEC_MP3_BANK", report["mp3_bank"]), ("CODEC_PAYLOAD_BYTES", report["payload_bytes"])]:
        lines.append("#define " + key + " " + hex(value) + "u")
    for index, value in enumerate(report["bank_identity"]):
        lines.append("#define CODEC_IMAGE_TAG" + str(index) + " " + hex(value) + "u")
    lines.append('#define CODEC_IMAGE_SHA256 "' + report["image_sha256"] + '"')
    lines.append("static const codec_bank_source codec_banks[CODEC_BANK_COUNT] = {")
    lines += ["    {" + hex(bank["offset"]) + "u, " + hex(bank["bytes"]) + "u}," for bank in report["code_banks"]]
    lines += ["};", "static const codec_data_source codec_data[CODEC_DATA_COUNT] = {"]
    lines += ["    {" + ", ".join(hex(block[key]) + "u" for key in ["word_address", "offset", "bytes"]) + "},"
              for block in report["data_blocks"]]
    if any("profile" in block for block in report["data_blocks"]):
        lines += ["};", "#define CODEC_DATA_PROFILES 1", "static const uint16_t codec_data_profiles[CODEC_DATA_COUNT] = {"]
        lines += ["    " + str(block.get("profile", 0)) + "u," for block in report["data_blocks"]]
    return "\n".join([*lines, "};", ""])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image", type=Path)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    payload, report = pack(args.image.read_bytes(), json.loads(args.manifest.read_text()))
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "codec_banks.bin").write_bytes(payload)
    (args.output / "codec_banks.h").write_text(write_header(report))
    report["packer_sha256"] = digest(Path(__file__).read_bytes())
    report["manifest_sha256"] = digest(args.manifest.read_bytes())
    (args.output / "codec_banks.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(dict(bytes=len(payload), banks=len(report["code_banks"]), data_blocks=len(report["data_blocks"]))))


if __name__ == "__main__":
    main()
