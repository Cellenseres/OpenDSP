# AAC decoding

OpenDSP decodes AAC-LC ADTS files (`.aac`) into interleaved PCM16 on ARM11.
It works without ndsp or a running DSP.

```c
#include <opendsp/aac.h>

opendspAacDecoder *decoder = NULL;
Result result = opendspAacOpen(&decoder, NULL);
```

Call `opendspAacDecode()` from your streaming thread and queue the output with
ndsp. Each call handles one ADTS frame. Advance input by `bytes_consumed` and
use `frames_written` for the output length. Close with `opendspAacClose()`.

Mono/stereo, 8-96 kHz and up to four raw data blocks per frame are supported.
Leave room for 1024 PCM frames per block. Reset the decoder after seeking and
serialize calls on each decoder.

`ERR_NEED_MORE_DATA` and `ERR_OUTPUT_TOO_SMALL` consume nothing. Corrupt or
unsupported frames report how much input was consumed. Handle errors that
make no progress instead of retrying in a tight loop.

All spectral codebooks, TNS, PNS, M/S, intensity stereo, window sequences and
sine/KBD windows are supported. HE-AAC plays its LC core; SBR and PS aren't
decoded. Other exclusions include Main/SSR/LTP, more than two channels,
channel configuration 0, CCE/LFE and 7350 Hz. ADTS CRC words aren't checked.
