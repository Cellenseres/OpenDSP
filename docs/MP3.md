# MP3 decoding

Use the [streaming API](STREAMING.md) for file playback. The decoder API in
`<opendsp/mp3.h>` accepts raw MP3 frames and returns interleaved PCM16.

```c
#include <opendsp/mp3.h>

opendspMp3Decoder *decoder = NULL;
Result result = opendspMp3Open(&decoder, NULL);
```

## Backends

AUTO selects DSP. Set `OPENDSP_MP3_BACKEND_ARM11` explicitly for CPU decoding:

```c
opendspMp3Config config = {sizeof(config), OPENDSP_MP3_BACKEND_ARM11, 0};
Result result = opendspMp3Open(&decoder, &config);
```

DSP requires a running OpenDSP instance. AAC and MP3 share one DSP decoder;
Open returns BUSY when it is occupied. ARM11 works independently of ndsp.
The selected backend stays fixed until Close; MP3 has no automatic fallback.
Use `opendspMp3GetBackend()` to check it.

## Decoding

Each `opendspMp3Decode()` call consumes at most one frame. Reserve space for
1,152 PCM frames, with two samples per frame for stereo. Advance input by
`bytes_consumed` and queue `frames_written` frames.

NEED_MORE_DATA and OUTPUT_TOO_SMALL consume nothing for incomplete input or
insufficient output space. SKIPPED can consume input without producing PCM
while finding sync or priming the reservoir. Handle errors without progress
instead of retrying indefinitely.

Serialize calls to each decoder. Reset after seeking and close before shutting
down OpenDSP. The streaming API handles file tags, buffering and playback.

## Formats

Both backends support MPEG-1 Layer III at 32, 44.1 and 48 kHz, mono/stereo,
normal stereo, dual channel and M/S stereo. Standard bitrates from 32 to
320 kbit/s support CBR, VBR and ABR. CRC16 is checked when present.

MPEG-2/2.5, intensity stereo, free format and de-emphasis are unsupported.
Rate and channel count must remain fixed. Xing/VBRI/LAME gapless trimming is
not applied; encoder delay and padding remain in the output. MP3 seeking is
limited to position zero.

Tables and transform dataflows derive from minimp3 (CC0).
See [references](../firmware/mp3/REFERENCES.md) for papers and architecture documentation.
