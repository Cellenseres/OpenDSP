# Building OpenDSP

The library includes a prebuilt firmware and codec tables. For a normal app you
only need devkitPro and libctru:

```sh
make -C library
make -C examples/soundlab
```

The Ogg codecs are bundled in `third_party`; `python3 third_party/verify.py`
checks them against their pinned releases.

For a CIA, install bannertool and makerom, then run
`make -C examples/soundlab cia`. Use makerom 0.18.4 or a build with the minor-version
fix; 0.19.0 writes the wrong version. The build checks the generated CIA.
Set `MAKEROM=/path/to/makerom` to use a particular executable.

## Changing the firmware

The DSP uses the [OpenDSP teak-llvm fork](https://github.com/Cellenseres/teak-llvm),
pinned in `toolchain/build_teak_llvm.sh`. On Linux or WSL, install git, cmake,
ninja, python3 and clang, then:

```sh
toolchain/build_teak_llvm.sh
make -C firmware
```

For an existing compiler build, pass `TEAK_LLVM=/path/to/build` to make. Keep
`opendsp.cdc`, `aac_tables.bin` and `mp3_tables.bin` together, then rebuild the library and app.
`BUILD_JOBS` sets the compiler build's parallel jobs (default 2).

The MP3 decoder and the mixer use only 32-bit values and build with
`-mllvm -teak-spill40=false` (`SPILL32` in `firmware/Makefile`).

`make -C firmware` builds the firmware without DSP Ogg decoding. The library
embeds the prebuilt image in `firmware/banked`, which loads its codecs in banks.
Build the library with `CODEC_BANKS=` to use the firmware from `make -C firmware`.

To regenerate the DSP table source:

```sh
python3 tools/gen_dsp_tables.py library/source/aac/tables.c firmware/aac
```

## Teak quirks

The compiler is limited, and hardware is stricter than the emulator:

- Leave an instruction between writing r7 and accessing `[r7+offset]`.
- Dual memory operands must use X and Y memory. Multiplier tables belong in Y.
- Protect live multiplier products from interrupts and keep those spans short.
- Large stack frames, local arrays and functions with more than four arguments
  can break code generation. Keep DSP C functions small.

Check assembly changes on hardware as well as in the emulator.
