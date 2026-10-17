# Building OpenDSP

The library includes a prebuilt firmware. For a normal app you
only need devkitPro and libctru:

```sh
make -C library
make -C examples/soundlab
```

For a CIA, install bannertool and makerom, then run
`make -C examples/soundlab cia`. Use makerom 0.18.4 or a build with the minor-version
fix; 0.19.0 writes the wrong version. The build checks the generated CIA.
Set `MAKEROM=/path/to/makerom` to use a particular executable.

## Changing the firmware

The DSP uses teak-llvm with the four patches in `toolchain/patches`.
On Linux or WSL, install git, cmake, ninja, python3 and clang, then:

```sh
toolchain/build_teak_llvm.sh
make -C firmware
```

The compiler build takes about half an hour. If it's already installed elsewhere,
pass `TEAK_LLVM=/path/to/build` to make. Keep the resulting `opendsp.cdc`, then rebuild the library and app.

## Teak quirks

The compiler is limited, and hardware is stricter than the emulator:

- Leave an instruction between writing r7 and accessing `[r7+offset]`.
- Dual memory operands must use X and Y memory. Multiplier tables belong in Y.
- Protect live multiplier products from interrupts and keep those spans short.
- Large stack frames, local arrays and functions with more than four arguments
  can break code generation. Keep DSP C functions small.

The patches handle several of these cases, but hand-written assembly still
needs care. Check changes on hardware as well as in the emulator.
