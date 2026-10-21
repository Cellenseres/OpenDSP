# SPDX-License-Identifier: BSD-2-Clause
# Builds opendsp.cdc with the patched teak-llvm (see docs/BUILDING.md).

TEAK_LLVM ?= ../toolchain/teak-llvm/build
.DEFAULT_GOAL := all
CC     := $(TEAK_LLVM)/bin/clang
LD     := $(TEAK_LLVM)/bin/ld.lld
PYTHON ?= python3

CFLAGS := --target=teak -march=teak -O2 -ffreestanding -fno-builtin -nostdlib \
          -integrated-as -Wall -Wno-int-to-pointer-cast -I. -Iaac
ASFLAGS := --target=teak -march=teak -integrated-as

OBJECTS := crt0.o main.o output.o dma.o voice.o source.o mixer.o reverb.o reverb_kernels.o surround.o mailbox.o stats.o math.o kernels.o
OBJECTS += aac/aac_parse.o aac/aac_stages.o aac/aac_dequant.o aac/aac_tns.o \
           aac/aac_fb.o aac/aac_tables_dsp.o aac/aac_fft.o aac/aac_window.o \
           aac/aac_window_pcm.o aac/aac_bits.o aac/aac_arith.o aac/stream.o aac/task.o
OBJECTS += mp3/decoder.o mp3/product.o mp3/transforms.o mp3/window.o mp3/synth.o mp3/requant.o mp3/normalize.o mp3/bits.o mp3/huffman.o mp3/schedule.o
HEADERS := $(wildcard *.h aac/*.h mp3/*.h)
mp3/decoder.o aac/stream.o: CFLAGS += -fno-strict-aliasing

mp3/decoder.o: mp3/decoder.c mp3/math.inc mp3/hybrid.inc $(HEADERS) ../tools/check_mp3_codegen.py
	$(CC) $(CFLAGS) -S $< -o mp3/decoder.check.s
	$(PYTHON) ../tools/check_mp3_codegen.py mp3/decoder.check.s
	$(CC) $(CFLAGS) -c $< -o $@

aac/stream.o: aac/stream.c aac/mp3_stream.inc $(HEADERS) ../tools/check_mp3_codegen.py
	$(CC) $(CFLAGS) -S $< -o aac/stream.check.s
	$(PYTHON) ../tools/check_mp3_codegen.py aac/stream.check.s
	$(CC) $(CFLAGS) -c $< -o $@

all: opendsp.cdc aac_tables.bin mp3_tables.bin

mp3_tables.bin: mp3/tables.h mp3/hybrid.inc mp3/table_layout.h ../tools/mp3_blob.py
	$(PYTHON) ../tools/mp3_blob.py $@

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
	rm -f *.o *.elf *.map aac/*.o aac/*.check.s mp3/*.o mp3/*.check.s

.PHONY: all clean
