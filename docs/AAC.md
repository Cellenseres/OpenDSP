# AAC decoding

OpenDSP decodes AAC-LC ADTS files (`.aac`) into interleaved PCM16 for ndsp.
It uses the DSP when available and falls back to ARM11 if it can't open a DSP
decoder. You can also choose either backend yourself.

```c
#include <opendsp/aac.h>

opendspAacDecoder *decoder = NULL;
Result result = opendspAacOpen(&decoder, NULL);
```

Once opened, call `opendspAacDecode()` from your streaming thread and queue the
PCM with ndsp. Close the decoder with `opendspAacClose()` before shutting down
OpenDSP.

## Choosing a backend

| Backend | Sample rates | Blocks per ADTS frame |
| --- | --- | --- |
| DSP | 8-48 kHz | 1 |
| ARM11 | 8-96 kHz | 1-4 |

Both handle mono and stereo. DSP input is limited to 2048 bytes per frame and
needs a running OpenDSP instance. ARM11 works without ndsp or the DSP.

`NULL` config and `OPENDSP_AAC_BACKEND_AUTO` try DSP first when the decoder is
opened. If it is unavailable or already in use, they select ARM11. The backend
then stays fixed until close; check it with `opendspAacGetBackend()`.

Choose ARM11 explicitly for higher rates or multi-block streams:

```c
opendspAacConfig config = {sizeof(config), OPENDSP_AAC_BACKEND_ARM11, 0};
Result result = opendspAacOpen(&decoder, &config);
```

Use `OPENDSP_AAC_BACKEND_DSP` to require DSP decoding. No opt-in flag is needed.
[AAC_DSP.md](AAC_DSP.md) covers buffering and DSP-specific behaviour.

## Decoding

Each call handles one ADTS frame. Leave room for 1024 PCM frames per raw data
block, with two samples per frame for stereo. Advance your input by
`bytes_consumed` and use `frames_written` for the output length.

`ERR_NEED_MORE_DATA` and `ERR_OUTPUT_TOO_SMALL` consume nothing. Skipped junk,
corrupt frames and unsupported frames report how much input was consumed.
Other errors with no progress should be handled rather than retried in a tight loop.

`info` reports the rate, channels and format changes. Reset the decoder after
seeking. Use one decoder per stream and serialize calls, including reset and close.

All spectral codebooks, TNS, PNS, M/S, intensity stereo, window sequences and
sine/KBD windows are supported. HE-AAC plays its LC core and sets
`OPENDSP_AAC_INFO_SBR_IGNORED`; SBR and PS aren't decoded. Other exclusions
include Main/SSR/LTP, more than two channels, channel configuration 0, CCE/LFE
and 7350 Hz. ADTS CRC words are skipped, not checked.
