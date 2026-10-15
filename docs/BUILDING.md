# Building the firmware

You only need this if you want to change the firmware. The library uses the prebuilt
`firmware/opendsp.cdc`.

## Compiler

The firmware is compiled with teak-llvm, an LLVM fork with a backend for the 3DS DSP. It needs
the four patches in `toolchain/patches`; without them the firmware does not work on real
hardware.

```sh
toolchain/build_teak_llvm.sh
```

The script clones teak-llvm into `toolchain/teak-llvm`, applies the patches and builds clang and
lld. You need git, cmake, ninja, python3 and clang. The build takes about half an hour.

## Firmware, library, demo

```sh
make -C firmware                   # firmware/opendsp.cdc
make -C library                    # library/lib/libopendsp.a
make -C examples/soundlab          # .3dsx
make -C examples/soundlab cia      # .cia, needs makerom and bannertool
```

If the compiler is somewhere else, use `make -C firmware TEAK_LLVM=/path/to/build`.

## Things to watch out for

On the real hardware, a memory access via `[r7+offset]` directly after an instruction that
writes r7 still sees the old r7. Patch 0004 takes care of this in compiled code. In
hand-written assembly you need at least one instruction in between. Emulators don't show this
bug, so test firmware changes on a console.

teak-llvm also has a few gaps: no 32-bit multiplication (hence `mul16` and shifts), and local
arrays crash it. That's why many buffers in the firmware are static.

Inner loops that use the multiplier read their second operand from Y memory. Tables they use
need `YMEM` or `YMEM_RO` (see `voice.h`).
