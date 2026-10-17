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

## Reverb and delay

OpenDSP also has a reverb and a delay that run on the DSP, on ndsp's two aux buses. Voices
send to them with the usual mix gains, and `ndspAuxSetVolume()` brings the result back.

```c
#include <opendsp/effects.h>

opendspReverbConfig hall = {
    .struct_size = sizeof(hall),
    .room_size = 0.9f, .decay_time = 2.5f, .damping = 0.35f,
    .diffusion = 0.75f, .width = 1.0f, .level = 0.7f,
};
opendspReverbSet(0, &hall);
ndspAuxSetVolume(0, 0.7f);
```

See [docs/GUIDE.md](docs/GUIDE.md) for the delay and what it all costs.

## Status

Supported: 24 voices, PCM8/PCM16 (mono and stereo), DSP-ADPCM, looping wave buffers, playback
rates up to 4x, all three interpolation modes, the per-voice filters, mix gains, the aux
settings, master volume, output modes, clipping, capture and sleep mode. Plus the reverb and
delay above.

Missing: AAC decoding. Surround output is mixed down to stereo, though the rear ratio works.

Tested on a New 3DS with Luma3DS.

## Demo

`examples/soundlab` is a small app with music, sound effects, a WAV player, mixer settings and
a reverb and delay page. The DSP lab shows how busy the DSP is. All of it runs on OpenDSP.

## Building the firmware

The library already contains a prebuilt `firmware/opendsp.cdc`. You only have to build it
yourself if you want to change the firmware. See [docs/BUILDING.md](docs/BUILDING.md).

## License

BSD 2-Clause. If you ship OpenDSP inside a binary, include the copyright notice and the
license text in your credits or documentation.

Made by Cellenseres. Thanks to Gericom for teak-llvm, wwylele for teakra, and the
devkitPro and libctru developers.

OpenDSP is not affiliated with Nintendo.
