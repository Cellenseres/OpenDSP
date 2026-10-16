# SPDX-License-Identifier: BSD-2-Clause
# Builds opendsp.cdc with the patched teak-llvm (see docs/BUILDING.md).

TEAK_LLVM ?= ../toolchain/teak-llvm/build
CC     := $(TEAK_LLVM)/bin/clang
LD     := $(TEAK_LLVM)/bin/ld.lld
PYTHON ?= python3

CFLAGS := --target=teak -march=teak -O2 -ffreestanding -fno-builtin -nostdlib \
          -integrated-as -Wall -Wno-int-to-pointer-cast

OBJECTS := crt0.o main.o output.o dma.o voice.o source.o mixer.o surround.o stats.o math.o kernels.o
HEADERS := $(wildcard *.h)

all: opendsp.cdc

polyphase_table.h: ../tools/gen_polyphase.py
	$(PYTHON) $< $@

%.o: %.c $(HEADERS) polyphase_table.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

opendsp.elf: $(OBJECTS) link.ld
	$(LD) -T link.ld $(OBJECTS) -o $@

opendsp.cdc: opendsp.elf ../tools/mkdsp1.py
	$(PYTHON) ../tools/mkdsp1.py $< -o $@

clean:
	rm -f *.o *.elf

.PHONY: all clean
