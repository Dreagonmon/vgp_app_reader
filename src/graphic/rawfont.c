#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "bmfont.h"
#include "bin_data.h"

static bool rawf_get_char_data(bmf_BitmapFont *font, uint32_t unicode, uint8_t *buffer) {
    uint16_t char_count = bin_read_uint16(font->font_file->data + 97);
    if (unicode <= char_count) {
        uint8_t ch_w = font->char_width;
        uint8_t ch_h = font->char_height;
        uint8_t wp = ch_w / 8;
        wp += (ch_w % 8) ? 1 : 0;
        uint16_t data_buffer_size = wp * ch_h;
        uint16_t idx = unicode - 1;
        const uint8_t *FONT_BASE = (const uint8_t *)(font->font_file->data + 100 + (idx * data_buffer_size));
        memcpy(buffer, FONT_BASE, data_buffer_size);
        return true;
    }
    return false;
}

static bmf_BitmapFontFile vfile_obj;
static bmf_BitmapFont font_raw_obj = {
    .get_char_image = rawf_get_char_data,
    .font_file = &vfile_obj,
    .char_width = 0,
    .char_height = 0,
    .ascii_width = NULL,
};
bmf_BitmapFont *font_raw = &font_raw_obj;

void init_raw_font(const uint8_t *FONT_BIN_DATA, uint16_t NOVAL_FONT_DSIZE) {
    vfile_obj.data = (void *) FONT_BIN_DATA;
    *((uint8_t *)&(font_raw->char_width)) = FONT_BIN_DATA[0];
    *((uint8_t *)&(font_raw->char_height)) = FONT_BIN_DATA[1];
    font_raw->ascii_width = FONT_BIN_DATA + 2;
}

