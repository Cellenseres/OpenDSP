# Adding OpenDSP to a project

OpenDSP only replaces the firmware that `ndspInit()` would load from the SD card. Everything
else you do with ndsp stays the same.

## As an installed library

```sh
make -C library install
```

This puts `libopendsp.a` and `opendsp.h` into `$DEVKITPRO/portlibs/3ds`. The devkitPro template
does not search portlibs by default, so add it:

```make
LIBS    := -lopendsp -lctru -lm
LIBDIRS := $(PORTLIBS) $(CTRULIB)
```

## From a subfolder

If you keep OpenDSP in your project, for example as a git submodule:

```make
OPENDSP := $(TOPDIR)/opendsp/library
LIBS    := -lopendsp -lctru -lm
LIBDIRS := $(OPENDSP) $(CTRULIB)
```

Run `make -C opendsp/library` once before building your app. `examples/soundlab/Makefile` does
this automatically.

## Without the library

Copy `library/include/opendsp.h` and `library/source/opendsp.c` into your project, and
`firmware/opendsp.cdc` into your `data` folder as `opendsp.bin`. The devkitPro template turns
it into `opendsp_bin` and `opendsp_bin_size`:

```c
#include "opendsp_bin.h"

opendspInitFromMemory(opendsp_bin, opendsp_bin_size);
```

## In the code

Replace `ndspInit()` with `opendspInit()` and `ndspExit()` with `opendspExit()`. The return
value is the same as `ndspInit()`'s.

A minimal example with a looping 440 Hz tone:

```c
#include <3ds.h>
#include <math.h>
#include <opendsp.h>

#define RATE    32728
#define SAMPLES 4096

int main(void)
{
    gfxInitDefault();
    if (R_FAILED(opendspInit())) {
        gfxExit();
        return 0;
    }

    s16 *tone = linearAlloc(SAMPLES * sizeof(s16));
    for (int i = 0; i < SAMPLES; i++)
        tone[i] = (s16)(8000 * sinf(2 * M_PI * 440.0f * i / RATE));
    DSP_FlushDataCache(tone, SAMPLES * sizeof(s16));

    ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM16);
    ndspChnSetRate(0, RATE);
    float mix[12] = {1.0f, 1.0f};
    ndspChnSetMix(0, mix);

    ndspWaveBuf buf = {0};
    buf.data_pcm16 = tone;
    buf.nsamples = SAMPLES;
    buf.looping = true;
    ndspChnWaveBufAdd(0, &buf);

    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_START)
            break;
        gspWaitForVBlank();
    }

    ndspChnWaveBufClear(0);
    opendspExit();
    linearFree(tone);
    gfxExit();
    return 0;
}
```

As with the original firmware, sample data has to be in linear memory and flushed with
`DSP_FlushDataCache()`.

## Reverb and delay

OpenDSP has a reverb and a delay that run on the DSP. They sit on the two aux buses ndsp
already has: a voice sends to bus 0 with mix gains 4 to 7 and to bus 1 with gains 8 to 11,
and `ndspAuxSetVolume()` sets how much comes back.

```c
#include <opendsp/effects.h>

u32 caps = 0;
opendspGetCapabilities(&caps);
if (caps & OPENDSP_CAP_REVERB) {
    opendspReverbConfig hall = {
        .struct_size = sizeof(hall),
        .room_size = 0.9f, .decay_time = 2.5f, .damping = 0.35f,
        .diffusion = 0.75f, .width = 1.0f, .level = 0.7f,
    };
    opendspReverbSet(0, &hall);
    ndspAuxSetVolume(0, 0.7f);
}

float mix[12] = {0};
mix[0] = mix[1] = 1.0f;     // dry
mix[4] = mix[5] = 0.4f;     // send to the reverb
ndspChnSetMix(0, mix);
```

`opendspDelaySet()` works the same way and gets its buffer from linear memory. `NULL`
switches an effect off.

Don't call `ndspAuxSetEnable()` on a bus with a DSP effect. That hands the bus to the ARM
first, ndsp never writes it back, and the effect only gets silence.

The effects only exist in the OpenDSP firmware, so check the capability bits if your app
might run with another one.

A playing reverb takes about 13 % of the DSP time, including what it costs to send music
to it. Once the input is silent and the tail is gone, it costs nothing. 24 voices with
both reverbs and a delay still fit.

The Sound Lab presets are a good place to start. They are our own settings, not copies of
any Nintendo sound.

## License

When you ship your app, include the copyright line and the text from `LICENSE` in your credits
or documentation. Nothing else is required, and your own code is not affected.
