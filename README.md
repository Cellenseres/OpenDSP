<p align="center"><img src="docs/logo.png" width="160" alt="OpenDSP"></p>

# OpenDSP

OpenDSP replaces the Nintendo 3DS DSP firmware used by libctru's ndsp.
Link it into your homebrew so users don't need to dump `dspfirm.cdc`.
It was written from scratch and contains no Nintendo firmware code.

## Getting started

With devkitPro and libctru installed:

```sh
make -C library install
```

Add OpenDSP to your Makefile:

```make
LIBS    := -lopendsp -lctru -lm
LIBDIRS := $(PORTLIBS) $(CTRULIB)
```

Include `<opendsp.h>`, use `opendspInit()` instead of `ndspInit()`, and call
`opendspExit()` when you're done. Your other ndsp calls stay the same.
[The guide](docs/GUIDE.md) covers adding it directly to a project.

## Audio and effects

OpenDSP supports 24 voices, PCM8/PCM16, DSP-ADPCM, mono/stereo, looping buffers,
all three interpolation modes, filters, mixing, capture and sleep/wake.
Surround output is currently mixed down to stereo; the rear ratio still works.

Reverb and delay run on the DSP through ndsp's two aux buses. Include
`<opendsp/effects.h>` to configure them. The [guide](docs/GUIDE.md#reverb-and-delay)
has an example, and Sound Lab includes Room, Hall, Plate and Cave presets.

## WAV, AAC and MP3

Use `<opendsp/audio.h>` to stream WAV, AAC and MP3 with pause, looping and WAV seeking.
It reads and buffers files in the background. See the [streaming guide](docs/STREAMING.md)
and [example](examples/streaming).

AAC-LC uses the DSP by default. ARM11 is available for standalone decoding and
formats outside the DSP's limits. If you only need the decoder, use
`<opendsp/aac.h>`; [AAC.md](docs/AAC.md) covers its API and format limits.

MP3 runs on DSP by default, with ARM11 available explicitly. Both support
MPEG-1 Layer III up to 320 kbit/s. Use `<opendsp/mp3.h>` for raw-frame decoding;
[MP3.md](docs/MP3.md) covers formats and API use.

## Sound Lab

[Sound Lab](examples/soundlab) lets you try music, sound effects, WAV/AAC/MP3 files,
filters and effects. The live monitor shows DSP load and the active decoder. Put your music in
`sdmc:/3ds/opendsp-lab/` and press X in the player to switch decoders.

[CIA and 3DSX downloads](dist/soundlab) are included. You can also build it with
`make -C examples/soundlab` or `make -C examples/soundlab cia`.

Tested on a New Nintendo 3DS with Luma3DS. The library includes the firmware;
see [BUILDING.md](docs/BUILDING.md) if you want to change it.

## License

BSD 2-Clause. Include the copyright notice and license text when distributing OpenDSP.

Made by Cellenseres. Thanks to Gericom for teak-llvm, wwylele for teakra,
and the devkitPro and libctru developers.

OpenDSP is not affiliated with Nintendo.
