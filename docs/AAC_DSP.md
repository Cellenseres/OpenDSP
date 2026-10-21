# AAC on the DSP

The DSP decodes AAC-LC alongside the mixer and effects. ARM11 reads the file,
submits compressed frames and queues PCM. The default configuration tries DSP.
To require it, call this after `opendspInit()`:

```c
opendspAacConfig config = {sizeof(config), OPENDSP_AAC_BACKEND_DSP, 0};
opendspAacDecoder *decoder = NULL;
Result result = opendspAacOpen(&decoder, &config);
```

`OPENDSP_CAP_AAC_DSP` reports firmware support. There is one DSP decoder, supporting
mono/stereo AAC-LC ADTS at 8-48 kHz, one block and at most 2048 encoded bytes per
frame. See [AAC.md](AAC.md) for other format limits.

Replacing `dspfirm.cdc` alone won't add AAC to an existing app. AAC uses OpenDSP's
API rather than Nintendo's binary-pipe interface.

## Playback

Use the [streaming API](STREAMING.md) for file playback. With the decoder API,
decode on a worker thread and keep several frames queued. Sound Lab uses four
1024-frame buffers and selects DSP by default; X switches to ARM11.

The mixer has priority. `OPENDSP_AAC_ERR_BUSY` consumes no input. A timeout returns
`ERR_TIMEOUT`; reset before retrying. The backend doesn't switch during playback.

After sleep or a firmware reload, restart the stream. Lost context sets
`OPENDSP_AAC_INFO_DISCONTINUITY`. Close before `opendspExit()` and serialize
decode, reset and close calls.

## Firmware

Decoding yields to the mixer between bounded stages. Keep `opendsp.cdc` and
`aac_tables.bin` together; the library embeds both and uploads tables at Open.
The linker checks memory bounds. Transport definitions are in `firmware/aac_protocol.h`.

`opendspAacGetStats()` reports decode time, scheduling latency, yields and stack use.
Sound Lab's DSP meter includes the mixer, effects and AAC.

AAC and MP3 share the DSP decoder. An explicit DSP Open returns BUSY while
the other codec owns it; AAC AUTO may select ARM11.
