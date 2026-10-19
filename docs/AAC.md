# AAC decoding

For WAV/AAC playback, use the [streaming API](STREAMING.md). This page covers
the decoder API, which returns interleaved PCM16 for you to queue with ndsp.

```c
#include <opendsp/aac.h>

opendspAacDecoder *decoder = NULL;
Result result = opendspAacOpen(&decoder, NULL);
```

## Backends

| Backend | Sample rates | Blocks per ADTS frame |
| --- | --- | --- |
| DSP | 8-48 kHz | 1 |
| ARM11 | 8-96 kHz | 1-4 |

Both support mono and stereo. DSP needs a running OpenDSP instance and accepts
at most 2048 encoded bytes per frame, including headers. ARM11 works without ndsp.

`NULL` config and `OPENDSP_AAC_BACKEND_AUTO` try DSP at Open, then ARM11 if the
DSP decoder is unavailable or occupied. The backend stays fixed; check it with
`opendspAacGetBackend()`.
Choose ARM11 for higher rates or multi-block streams:

```c
opendspAacConfig config = {sizeof(config), OPENDSP_AAC_BACKEND_ARM11, 0};
Result result = opendspAacOpen(&decoder, &config);
```

Use `OPENDSP_AAC_BACKEND_DSP` to require DSP. See [AAC_DSP.md](AAC_DSP.md) for
scheduling and context handling.

## Decoding

For playback, decode on a worker thread. Each `opendspAacDecode()` call handles
one ADTS frame.
Reserve 1024 PCM frames per raw data block, with two samples per frame for stereo.
Advance input by `bytes_consumed` and queue `frames_written` frames.

`ERR_NEED_MORE_DATA` and `ERR_OUTPUT_TOO_SMALL` consume nothing. Skipped junk,
corrupt frames and unsupported frames report consumed bytes. Handle errors
without progress instead of retrying in a tight loop.

`info` reports rate, channels and format changes. Reset after seeking. Serialize
calls to each decoder, including reset and close. Call `opendspAacClose()` before
shutting down OpenDSP.

## Formats

AAC-LC ADTS supports all spectral codebooks, TNS, PNS, M/S, intensity stereo,
window sequences and sine/KBD windows. HE-AAC plays its LC core and sets
`OPENDSP_AAC_INFO_SBR_IGNORED`; SBR and PS aren't decoded.

Main/SSR/LTP, more than two channels, channel configuration 0, CCE/LFE and
7350 Hz are unsupported. ADTS CRC words are skipped, not checked.
