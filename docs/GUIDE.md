# Using OpenDSP

OpenDSP loads its own firmware and keeps the usual ndsp API.

## Install the library

```sh
make -C library install
```

Add it to your app's Makefile:

```make
LIBS    := -lopendsp -lctru -lm
LIBDIRS := $(PORTLIBS) $(CTRULIB)
```

Or keep OpenDSP in your project and build its library before your app:

```make
OPENDSP := $(TOPDIR)/opendsp/library
LIBS    := -lopendsp -lctru -lm
LIBDIRS := $(OPENDSP) $(CTRULIB)
```

Sound Lab's Makefile does this automatically.

## In your code

Include `<opendsp.h>`. Replace `ndspInit()` with `opendspInit()` and `ndspExit()`
with `opendspExit()`. Check the return value before using audio.
Buffers you manage yourself still need linear memory and `DSP_FlushDataCache()`.

If you want to supply your own firmware image, link the library and use
`opendspInitFromMemory(image, size)` instead.

## Reverb and delay

Both run on the DSP through ndsp's aux buses. Bus 0 uses mix gains 4-7;
bus 1 uses gains 8-11. `ndspAuxSetVolume()` controls the return level.

```c
#include <opendsp/effects.h>

opendspReverbConfig hall = {
    .struct_size = sizeof(hall),
    .room_size = 0.9f, .decay_time = 2.5f, .damping = 0.35f,
    .diffusion = 0.75f, .width = 1.0f, .level = 0.7f,
};
opendspReverbSet(0, &hall);
ndspAuxSetVolume(0, 0.7f);

float mix[12] = {0};
mix[0] = mix[1] = 1.0f;
mix[4] = mix[5] = 0.4f;
ndspChnSetMix(0, mix);
```

`opendspDelaySet()` works similarly. Pass `NULL` to turn an effect off.
Don't enable ARM aux processing with `ndspAuxSetEnable()` on the same bus;
that prevents the DSP effect from receiving its input.

Check `opendspGetCapabilities()` if your app can run with another firmware.
Effects, voices and AAC share the DSP; `opendspGetStats()` reports its load.

## Playing files

Use `<opendsp/audio.h>` for WAV/AAC playback. Reserve unused channels with
`opendspAudioInit(mask)`, then call `opendspAudioOpen()` and `opendspAudioPlay()`.
The library handles buffering. See the [streaming guide](STREAMING.md) and
[example](../examples/streaming/source/main.c).

For manual AAC decoding, use `<opendsp/aac.h>`. [AAC.md](AAC.md) covers the API
and format limits.

## License

Include the copyright notice and text from `LICENSE` when distributing OpenDSP.
