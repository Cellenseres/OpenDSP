// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>

#define RGB(r, g, b) ((u32)(((r) << 16) | ((g) << 8) | (b)))

void gfx_begin(u32 bg);                 // clears the top screen
void gfx_rect(int x, int y, int w, int h, u32 c);
void gfx_hline(int x, int y, int w, u32 c);
void gfx_vline(int x, int y, int h, u32 c);
void gfx_line(int x0, int y0, int x1, int y1, u32 c);
void gfx_text(int x, int y, const char *s, u32 c, int scale);   // uppercase only
int  gfx_text_width(const char *s, int scale);
