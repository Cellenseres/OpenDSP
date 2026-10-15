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

## License

When you ship your app, include the copyright line and the text from `LICENSE` in your credits
or documentation. Nothing else is required, and your own code is not affected.
