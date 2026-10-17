# AAC on the DSP

The DSP handles the AAC-LC decode pipeline while the mixer and effects keep
running. ARM11 reads the file, submits compressed frames and queues the PCM.

The default AAC configuration tries DSP first. To require it:

```c
opendspAacConfig config = {sizeof(config), OPENDSP_AAC_BACKEND_DSP, 0};
opendspAacDecoder *decoder = NULL;
Result result = opendspAacOpen(&decoder, &config);
```

Call this after `opendspInit()`. `OPENDSP_CAP_AAC_DSP` reports firmware support.

## Limits

AAC-LC ADTS, mono/stereo, 8-48 kHz, one raw data block per frame and at most
2048 encoded bytes including headers. See [AAC.md](AAC.md) for the other
format limits. There is one DSP decoder at a time.

This uses OpenDSP's AAC API. It doesn't implement Nintendo's binary-pipe AAC
interface, so replacing `dspfirm.cdc` alone won't add AAC support to an existing app.

## Playback

Decode on a streaming thread and keep several frames queued.

The mixer has priority. Heavy audio load can return `OPENDSP_AAC_ERR_BUSY`
without consuming input. A decode timeout returns `ERR_TIMEOUT`; reset before
retrying. The backend won't switch during playback.

After sleep or a firmware reload, restart the stream. A lost DSP context is
reported through `OPENDSP_AAC_INFO_DISCONTINUITY`. Close the decoder before
`opendspExit()`, and don't call decode, reset or close concurrently.

## Firmware work

The decoder yields between bounded pieces of work so audio can keep playing.
Its fixed-point output matches the ARM11 backend in local comparisons. Reverb
settings and audio precision are unchanged by the assembly optimizations.

Keep `opendsp.cdc` and `aac_tables.bin` together. The library embeds both and
uploads the tables when opening the decoder. Memory bounds are checked by the
linker; the transport layout is in `firmware/aac_protocol.h`.

`opendspAacGetStats()` reports decode time, scheduling latency, yields and stack
use. Sound Lab's DSP meter includes the mixer, effects and AAC together.
