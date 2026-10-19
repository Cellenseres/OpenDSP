# Streaming audio

`<opendsp/audio.h>` plays WAV and AAC from SD or RomFS. It reads files in the
background, using a fixed amount of memory.

## Setup

Initialize the filesystem and OpenDSP, then reserve unused ndsp channels:

```c
#include <opendsp.h>
#include <opendsp/audio.h>

opendspInit();
opendspAudioInit((1u << 0) | (1u << 1));
opendspAudioStream *music = NULL;
opendspAudioOpen(&music, "sdmc:/music.aac", NULL);
opendspAudioSetLoop(music, true);
opendspAudioPlay(music);
```

Check return values in your app. The [complete example](../examples/streaming/source/main.c)
includes error handling and cleanup. Build with `make -C examples/streaming`.
It plays `sdmc:/3ds/opendsp-streaming/music.aac` and an optional `ambient.wav`.
Pass a path to use another file. RomFS assets must be included in your build.

Open probes the file synchronously; Play buffers it on a worker. Keep the
filesystem mounted until Close succeeds. Files must be smaller than 2 GiB.

## Formats

WAV supports integer PCM8/PCM16, mono/stereo, 1000-130912 Hz. Compressed WAV,
WAVE_FORMAT_EXTENSIBLE and MP4/M4A are unsupported. Detection uses file contents.
See [AAC.md](AAC.md) for AAC-LC ADTS limits. HE-AAC plays its LC core;
`opendspAudioInfo.aac_flags` reports `OPENDSP_AAC_INFO_SBR_IGNORED`.

AUTO prefers DSP, falling back to ARM11 at Open if the decoder is occupied or
the first frame exceeds its limits. Explicit DSP returns an error instead.
GetBackend reports the choice, which stays fixed until Close. DSP has one decoder,
shared with `opendspAacOpen()`. Later format changes, timeouts or lost context
stop playback.

## Controls

Commands are asynchronous; check GetState and GetLastError. FINISHED means the
final queued PCM has played.

- Pause retains the queue; Resume continues it. Play does nothing while active.
- Stop cancels pending work and rewinds. Play restarts a finished or failed stream.
- WAV Seek accepts a PCM frame position and preserves pause/play state. On a
  stopped stream it sets the next starting position. AAC only supports Seek to zero.

GetPosition reports audible PCM frames and wraps on loops. WAV duration is known
at Open; AAC duration appears after a full pass. AAC loops reset decoder history;
encoder padding can leave a gap. Changing loop mode cannot retract queued PCM.

## Channels and buffers

The Init mask reserves channels for the library. Each stream takes one;
GetChannel returns it. Keep direct ndsp playback on other channels.
Use `OPENDSP_AUDIO_CONFIG_INIT` for custom settings. A zeroed config selects
channel 0 rather than AUTO; NULL uses defaults.

The library owns format, rate, pause state and wave buffers. ndsp interpolation
and filters remain available. SetRate accepts 1-130912 Hz. SetVolume accepts 0-1
and scales the mix captured at Open or supplied by SetMix. Update routing through
SetMix when using library volume. Gains 4/5 and 8/9 send to
[reverb and delay](GUIDE.md#reverb-and-delay).

PCM and AAC use separate workers. Forward your ndsp callback to
`opendspAudioNotifyFrame()` for prompt refills; otherwise active workers wait up
to 5 ms. Idle workers sleep until a command.

Defaults are four buffers of 2048 frames for WAV or 1024 for AAC: 32 KiB and 16 KiB
for stereo PCM16. Config controls queue size. AAC also needs 8 KiB input,
4 KiB scratch PCM for DSP decoding or 16 KiB for ARM11, plus decoder storage.
Each worker has a 16 KiB stack. An underrun pauses playback and buffers again.

GetStats reports queue occupancy, underruns, loops, memory and timing.
Read/decode/worker ticks include waits, so they aren't CPU load.
Use `opendspGetStats()` for DSP load.

## Cleanup and sleep

Run Init, Open, Close and Exit on the main thread. Commands and snapshot getters
can run on other app threads. Don't call lifecycle functions from ndsp/DSP hooks
or use a handle concurrently with Close.

Close streams, then call AudioExit before opendspExit. Close sets the handle to
NULL on success. It waits up to about one second; BUSY retains the handle, memory
and channel. Retry later. An active file read cannot be forcibly cancelled.

Sleep/wake reports `OPENDSP_AUDIO_ERR_CONTEXT`; restart or reopen after wake.
Commands while asleep return `OPENDSP_AUDIO_ERR_STATE`. Close may stay BUSY while
DSP frames are stopped. Extra ndsp references can keep DSP alive after
`opendspExit()`; retry any retained handles.
