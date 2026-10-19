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
ARCH      := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS    := -O2 -Wall -Wextra $(ARCH) -mword-relocations -ffunction-sections -fdata-sections \
             -D__3DS__ -Iinclude -I../firmware -I$(DEVKITPRO)/libctru/include
INSTALL   := $(DEVKITPRO)/portlibs/3ds

all: lib/libopendsp.a

build/%.o: source/%.c $(wildcard include/*.h include/opendsp/*.h source/*.h) ../firmware/extension.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

AAC_OBJECTS := build/aac_api.o build/aac_dsp.o $(addprefix build/aac/,decoder.o syntax.o spectrum.o filterbank.o tables.o)
AUDIO_OBJECTS := $(addprefix build/audio/,input.o codec.o wav.o aac.o output.o engine.o)

build/audio/%.o: source/audio/%.c $(wildcard source/audio/*.h include/opendsp/*.h)
	@mkdir -p build/audio
	$(CC) $(CFLAGS) -c $< -o $@

$(AAC_OBJECTS): include/opendsp/aac.h $(wildcard source/aac/*.h) ../firmware/aac_protocol.h

build/aac/%.o: source/aac/%.c
	@mkdir -p build/aac
	$(CC) $(CFLAGS) -c $< -o $@

build/firmware.o: source/firmware.s $(FIRMWARE) ../firmware/aac_tables.bin
	@mkdir -p build
	$(CC) $(ARCH) -x assembler-with-cpp -Wa,-I,$(dir $(FIRMWARE)) -c $< -o $@

lib/libopendsp.a: build/opendsp.o build/embedded.o build/firmware.o build/mailbox.o build/effects.o $(AAC_OBJECTS) $(AUDIO_OBJECTS)
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
