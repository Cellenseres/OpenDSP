# SPDX-License-Identifier: BSD-2-Clause
# Shared by the target library and the host codec regression harness.
AUDIO_CODECS_ROOT ?= ../third_party
include $(AUDIO_CODECS_ROOT)/opus/opus_sources.mk
include $(AUDIO_CODECS_ROOT)/opus/celt_sources.mk
include $(AUDIO_CODECS_ROOT)/opus/silk_sources.mk

OGG_CODEC_SOURCES := libogg/src/bitwise.c libogg/src/framing.c
TREMOR_CODEC_SOURCES := $(addprefix tremor/,mdct.c block.c window.c synthesis.c info.c floor1.c floor0.c vorbisfile.c res012.c mapping0.c registry.c codebook.c sharedbook.c)
# Compile portable upstream C. The linker discards unused encoder sections.
OPUS_CODEC_SOURCES := $(addprefix opus/,$(OPUS_SOURCES) $(CELT_SOURCES) $(SILK_SOURCES) $(SILK_SOURCES_FIXED))
OPUSFILE_CODEC_SOURCES := opusfile/src/info.c opusfile/src/internal.c opusfile/src/opusfile.c opusfile/src/stream.c
AUDIO_CODEC_SOURCES := $(OGG_CODEC_SOURCES) $(TREMOR_CODEC_SOURCES) $(OPUS_CODEC_SOURCES) $(OPUSFILE_CODEC_SOURCES)
AUDIO_CODEC_INCLUDES := -I$(AUDIO_CODECS_ROOT)/config -I$(AUDIO_CODECS_ROOT)/libogg/include -I$(AUDIO_CODECS_ROOT)/tremor -I$(AUDIO_CODECS_ROOT)/opus/include -I$(AUDIO_CODECS_ROOT)/opus -I$(AUDIO_CODECS_ROOT)/opus/celt -I$(AUDIO_CODECS_ROOT)/opus/silk -I$(AUDIO_CODECS_ROOT)/opus/silk/fixed -I$(AUDIO_CODECS_ROOT)/opusfile/include
# Use heap-backed, per-stream switched Opus scratch; Tremor alloca is redirected
# to bounded, call-lifetime heap scratch by opendsp_alloc.h.
# Keep host and ARM arithmetic identical.
AUDIO_CODEC_FLAGS := -DFIXED_POINT -DOP_FIXED_POINT -DOPUS_FAST_INT64=0 -DNONTHREADSAFE_PSEUDOSTACK -DOPUS_BUILD -DHAVE_LRINTF -DHAVE_LRINT -DHAVE_INTTYPES_H -DHAVE_STDINT_H -DOPENDSP_CODEC_ALLOCATOR -include $(AUDIO_CODECS_ROOT)/opendsp_alloc.h
