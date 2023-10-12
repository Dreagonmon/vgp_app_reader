#pragma once

#include <stdint.h>
#include <stddef.h>
#include "bmfont.h"
#include "framebuf.h"
#include "u8str.h"

#define ui_ALIGN_HLEFT                0b00000001
#define ui_ALIGN_HCENTER              0b00000010
#define ui_ALIGN_HRIGHT               0b00000100
#define ui_ALIGN_VTOP                 0b00010000
#define ui_ALIGN_VCENTER              0b00100000
#define ui_ALIGN_VBOTTOM              0b01000000

void ui_text_area(bmf_BitmapFont *font, U8String text, gfb_FrameBuffer *frame, int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t align, uint16_t color, uint16_t bg_color);

