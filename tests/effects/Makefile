# SPDX-License-Identifier: BSD-2-Clause
# Host tests for the firmware effects (no DSP toolchain needed).
CC     ?= cc
CFLAGS ?= -O2 -g -Wall -Wextra -Wno-unused-function -Wno-int-to-pointer-cast
FW     := ../../firmware

all: test

effects_test: effects_test.c $(FW)/reverb.c $(FW)/reverb.h $(FW)/extension.h
	$(CC) $(CFLAGS) -I$(FW) effects_test.c $(FW)/reverb.c -lm -o $@

test: effects_test
	./effects_test

clean:
	rm -f effects_test

.PHONY: all test clean
