# SPDX-License-Identifier: BSD-2-Clause
# Sound Lab: .3dsx, or .cia with `make cia`

.SUFFIXES:
ifeq ($(strip $(DEVKITARM)),)
$(error Set DEVKITARM, e.g. export DEVKITARM=$$DEVKITPRO/devkitARM)
endif
TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET      := opendsp-soundlab
BUILD       := build
SOURCES     := source
OPENDSP     := $(TOPDIR)/../../library

APP_TITLE       := OpenDSP Sound Lab
APP_DESCRIPTION := Music, sound effects and DSP demos on OpenDSP
APP_AUTHOR      := Cellenseres
ICON            := $(TOPDIR)/meta/icon.png

ARCH     := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS   := -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH) $(INCLUDE) -D__3DS__
LDFLAGS   = -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS     := -lopendsp -lctru -lm
LIBDIRS  := $(OPENDSP) $(CTRULIB)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)
CFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export LD       := $(CC)
export OFILES   := $(CFILES:.c=.o)
export INCLUDE  := $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export _3DSXFLAGS := --smdh=$(CURDIR)/$(TARGET).smdh

.PHONY: all cia library clean

all: library
	@[ -d $(BUILD) ] || mkdir -p $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

library:
	@$(MAKE) --no-print-directory -C $(OPENDSP)

cia: all
	bannertool makebanner -i meta/banner.png -a meta/banner.wav -o $(BUILD)/banner.bnr
	makerom -f cia -o $(TARGET).cia -target t -exefslogo -rsf meta/soundlab.rsf \
		-elf $(TARGET).elf -icon $(TARGET).smdh -banner $(BUILD)/banner.bnr \
		-major 1 -minor 1 -micro 0

clean:
	@rm -rf $(BUILD) $(TARGET).3dsx $(TARGET).smdh $(TARGET).elf $(TARGET).cia

else

$(OUTPUT).3dsx: $(OUTPUT).elf $(OUTPUT).smdh
$(OUTPUT).elf: $(OFILES)

-include $(DEPSDIR)/*.d

endif
