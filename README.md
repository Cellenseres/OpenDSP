<p align="center"><img src="docs/logo.png" width="160" alt="OpenDSP"></p>

# OpenDSP

An open-source replacement for `dspfirm.cdc`, the DSP firmware that libctru's ndsp needs on the
Nintendo 3DS.

Normally, a homebrew app with sound only works after the user has dumped `dspfirm.cdc` from
their console onto the SD card. OpenDSP gets linked into your app instead, so there is
nothing to dump and nothing to copy. ndsp works as before. You only call `opendspInit()` instead
of `ndspInit()`.

OpenDSP was written from scratch and contains no code from Nintendo's firmware.

## Usage

Build and install the library (devkitPro with libctru required):

```sh
make -C library install
```

In your Makefile:

```make
LIBS    := -lopendsp -lctru -lm
LIBDIRS := $(PORTLIBS) $(CTRULIB)
```

In your code:

```c
#include <opendsp.h>

opendspInit();   // instead of ndspInit()
...
opendspExit();   // instead of ndspExit()
```

More details and other ways to integrate it are in [docs/GUIDE.md](docs/GUIDE.md).

## Status

Supported: 24 voices, PCM8/PCM16 (mono and stereo), DSP-ADPCM, looping wave buffers, playback
rates up to 4x, all three interpolation modes, the per-voice filters, mix gains, the aux
settings, master volume, output modes, clipping, capture and sleep mode.

Missing: AAC decoding and the reverb effect. Surround output is mixed down to stereo.

Tested on a New 3DS with Luma3DS.

## Demo

`examples/soundlab` is a small app with music, sound effects, a WAV player and mixer settings.
All of it runs on OpenDSP.

## Building the firmware

The library already contains a prebuilt `firmware/opendsp.cdc`. You only have to build it
yourself if you want to change the firmware. See [docs/BUILDING.md](docs/BUILDING.md).

## License

BSD 2-Clause. If you ship OpenDSP inside a binary, include the copyright notice and the
license text in your credits or documentation.

Made by Cellenseres. Thanks to Gericom for teak-llvm, wwylele for teakra, and the
devkitPro and libctru developers.

OpenDSP is not affiliated with Nintendo.
