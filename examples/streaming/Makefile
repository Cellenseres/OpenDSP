# SPDX-License-Identifier: BSD-2-Clause
.SUFFIXES:
ifeq ($(strip $(DEVKITARM)),)
$(error Set DEVKITARM)
endif
TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET := opendsp-streaming
BUILD := build
OPENDSP := $(TOPDIR)/../../library
APP_TITLE := OpenDSP Streaming
APP_DESCRIPTION := WAV and AAC streaming
APP_AUTHOR := Cellenseres
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS := -g -Wall -Wextra -O2 -mword-relocations -ffunction-sections $(ARCH) $(INCLUDE) -D__3DS__
LDFLAGS = -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := -lopendsp -lctru -lm
LIBDIRS := $(OPENDSP) $(CTRULIB)

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)
export VPATH := $(CURDIR)/source
export DEPSDIR := $(CURDIR)/$(BUILD)
export LD := $(CC)
export OFILES := main.o
export INCLUDE := $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export _3DSXFLAGS := --smdh=$(CURDIR)/$(TARGET).smdh
.PHONY: all library clean
all: library
	@[ -d $(BUILD) ] || mkdir -p $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
library:
	@$(MAKE) --no-print-directory -C $(OPENDSP)
clean:
	@rm -rf $(BUILD) $(TARGET).3dsx $(TARGET).smdh $(TARGET).elf
else
$(OUTPUT).3dsx: $(OUTPUT).elf $(OUTPUT).smdh
$(OUTPUT).elf: $(OFILES) $(OPENDSP)/lib/libopendsp.a
-include $(DEPSDIR)/*.d
endif
