# SPDX-License-Identifier: BSD-2-Clause
# Builds opendsp.cdc with the patched teak-llvm (see docs/BUILDING.md).

TEAK_LLVM ?= ../toolchain/teak-llvm/build
CC     := $(TEAK_LLVM)/bin/clang
LD     := $(TEAK_LLVM)/bin/ld.lld
PYTHON ?= python3

CFLAGS := --target=teak -march=teak -O2 -ffreestanding -fno-builtin -nostdlib \
          -integrated-as -Wall -Wno-int-to-pointer-cast -I. -Iaac
ASFLAGS := --target=teak -march=teak -integrated-as

OBJECTS := crt0.o main.o output.o dma.o voice.o source.o mixer.o reverb.o reverb_kernels.o surround.o mailbox.o stats.o math.o kernels.o
OBJECTS += aac/aac_parse.o aac/aac_stages.o aac/aac_dequant.o aac/aac_tns.o \
           aac/aac_fb.o aac/aac_tables_dsp.o aac/aac_fft.o aac/aac_window.o \
           aac/aac_bits.o aac/aac_arith.o aac/stream.o aac/task.o
HEADERS := $(wildcard *.h aac/*.h)

all: opendsp.cdc aac_tables.bin

polyphase_table.h: ../tools/gen_polyphase.py
	$(PYTHON) $< $@

%.o: %.c $(HEADERS) polyphase_table.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(ASFLAGS) -c $< -o $@

opendsp.elf: $(OBJECTS) link.ld
	$(LD) -T link.ld -Map=opendsp.map $(OBJECTS) -o $@

aac_tables.bin: opendsp.elf ../tools/aac_blob.py
	$(PYTHON) ../tools/aac_blob.py $< $@

opendsp.cdc: opendsp.elf ../tools/mkdsp1.py
	$(PYTHON) ../tools/mkdsp1.py $< -o $@

clean:
	rm -f *.o *.elf *.map aac/*.o

.PHONY: all clean
