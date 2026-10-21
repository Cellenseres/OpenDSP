# SPDX-License-Identifier: BSD-2-Clause
# libopendsp.a; `make install` copies it to portlibs.

ifeq ($(strip $(DEVKITPRO)),)
$(error Set DEVKITPRO, e.g. export DEVKITPRO=/opt/devkitpro)
endif

DEVKITARM ?= $(DEVKITPRO)/devkitARM
PREFIX    := $(DEVKITARM)/bin/arm-none-eabi-
CC        := $(PREFIX)gcc
AR        := $(PREFIX)gcc-ar

FIRMWARE  := ../firmware/opendsp.cdc
MP3_TABLES ?= ../firmware/mp3_tables.bin
ARCH      := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS    := -O2 -Wall -Wextra $(ARCH) -mword-relocations -ffunction-sections -fdata-sections \
             -D__3DS__ -Iinclude -I../firmware -I$(DEVKITPRO)/libctru/include
INSTALL   := $(DEVKITPRO)/portlibs/3ds

all: lib/libopendsp.a

build/%.o: source/%.c $(wildcard include/*.h include/opendsp/*.h source/*.h) ../firmware/extension.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

AAC_OBJECTS := build/aac_api.o build/aac_dsp.o build/mp3_api.o build/mp3_arm11.o $(addprefix build/aac/,decoder.o syntax.o spectrum.o filterbank.o tables.o)
AUDIO_OBJECTS := $(addprefix build/audio/,input.o codec.o wav.o aac.o mp3.o mp3_arm11.o output.o engine.o)

build/mp3_arm11.o: ../firmware/mp3/decoder.c ../firmware/mp3/decoder.h ../firmware/mp3/math.inc ../firmware/mp3/hybrid.inc ../firmware/mp3/tables.h ../firmware/hw.h
build/mp3_arm11.o: CFLAGS += -fno-strict-aliasing

build/audio/%.o: source/audio/%.c $(wildcard source/audio/*.h source/*.h include/opendsp/*.h) ../firmware/mp3_protocol.h
	@mkdir -p build/audio
	$(CC) $(CFLAGS) -c $< -o $@

$(AAC_OBJECTS): include/opendsp/aac.h $(wildcard source/aac/*.h) ../firmware/aac_protocol.h ../firmware/mp3_protocol.h ../firmware/mp3/table_layout.h

build/aac/%.o: source/aac/%.c
	@mkdir -p build/aac
	$(CC) $(CFLAGS) -c $< -o $@

$(MP3_TABLES): ../tools/mp3_blob.py ../firmware/mp3/tables.h ../firmware/mp3/hybrid.inc ../firmware/mp3/table_layout.h
	python3 ../tools/mp3_blob.py $@

build/firmware.o: source/firmware.s $(FIRMWARE) ../firmware/aac_tables.bin $(MP3_TABLES)
	@mkdir -p build
	$(CC) $(ARCH) -x assembler-with-cpp -Wa,-I,$(dir $(FIRMWARE)) -Wa,-I,$(dir $(MP3_TABLES)) -c $< -o $@

lib/libopendsp.a: build/opendsp.o build/embedded.o build/firmware.o build/mailbox.o build/codec_events.o build/effects.o $(AAC_OBJECTS) $(AUDIO_OBJECTS)
	@mkdir -p lib
	rm -f $@
	$(AR) rcs $@ $^

install: lib/libopendsp.a
	mkdir -p $(INSTALL)/lib $(INSTALL)/include/opendsp
	cp lib/libopendsp.a $(INSTALL)/lib/
	cp include/opendsp.h $(INSTALL)/include/
	cp include/opendsp/*.h $(INSTALL)/include/opendsp/

clean:
	rm -rf build lib

.PHONY: all install clean
