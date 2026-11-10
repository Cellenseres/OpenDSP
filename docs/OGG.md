# Ogg Vorbis and Opus

Use the [streaming API](STREAMING.md) for Ogg files. Open reads the first packet
after `OggS`: `\x01vorbis` selects Vorbis, `OpusHead` selects Opus. File
extensions don't matter.

```c
opendspAudioStream *music = NULL;
opendspAudioOpen(&music, "sdmc:/music.ogg", NULL);
opendspAudioSetLoop(music, true);
opendspAudioPlay(music);
```

## Formats

Vorbis supports mono/stereo at 1000-130912 Hz. Opus supports channel mapping
family 0 (mono/stereo) with SILK, CELT and hybrid packets; positions use 48 kHz
with pre-skip removed. Header output gain is applied. Chained or multiplexed
streams return UNSUPPORTED. Files must be seekable and smaller than 2 GiB.

Packets and codec headers are limited to 1 MiB each, comments to 256 KiB and
4,096 fields. Decoder allocations are capped at 8 MiB per stream and 16 MiB in total.

## Backends

Both codecs decode on ARM11 (libogg, Tremor, libopus, opusfile; see
[third_party](../third_party)). The default firmware also decodes them on the DSP.

AUTO picks DSP for Vorbis up to 48 kHz and 288 kbit/s. Opus uses DSP for frames of
at least 10 ms up to 288 kbit/s, or 128 kbit/s for 10 ms frames. Other files use
ARM11. AUTO falls back to ARM11 if the DSP is busy or setup fails; explicit DSP
returns the error instead. DSP decoding shares the single DSP decoder with AAC and MP3.

`opendspAudioSupportsBackend()` reports compiled support without reserving the DSP.

## Loops

Loop points come from comment tags, in source frames:

```text
LOOPSTART=12000
LOOPEND=50000
```

`LOOPLENGTH=38000` can replace LOOPEND. Start is inclusive, end exclusive. The
intro before LOOPSTART plays once. LOOPSTART alone loops to the end of the file.
Names are case-insensitive; values are plain decimal numbers.

SetLoop uses the tagged range, or the whole file without tags. Invalid tags
return FORMAT when enabling a loop; normal playback still works. GetLoopInfo
reports the range. SetLoopRange overrides it; ClearLoopRange restores the tags.

Loops up to 2 MiB of PCM are cached on the first pass and repeat without seeks.
Longer loops cache the first 250 ms and then seek the decoder.

## Seeking

Seek takes a source frame position, also inside a packet. Opus seeks use 80 ms
of pre-roll. GetPosition follows the audible frame and jumps back at loop ends.
To resume later, save GetPosition with the file, reopen, then Seek before Play.
