#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "bmfont.h"
#include "bin_data.h"

static bool compf_get_char_data(bmf_BitmapFont *font, uint32_t unicode, uint8_t *buffer) {
    uint16_t char_count = bin_read_uint16(font->font_file->data + 97);
    if (unicode <= char_count) {
        uint8_t ch_w = font->char_width;
        uint8_t ch_h = font->char_height;
        uint8_t wp = ch_w / 8;
        wp += (ch_w % 8) ? 1 : 0;
        uint16_t data_buffer_size = wp * ch_h;
        uint8_t block_char_size = ((const uint8_t *) font->font_file->data)[96];
        uint8_t block_count = (char_count / block_char_size) + 1;
        uint8_t *font_data_base_offset = font->font_file->data + 100 + (block_count * 2);
        uint16_t idx = unicode - 1;
        uint8_t bidx = idx / block_char_size;
        uint8_t boff = idx % block_char_size;
        uint16_t data_offset_startoff = 0;
        if (bidx > 0) {
            data_offset_startoff += bin_read_uint16(font->font_file->data + 100 + (2 * (bidx - 1)));
        }
        uint16_t data_offset_endoff = bin_read_uint16(font->font_file->data + 100 + (2 * bidx));
        bin_font_reset(font_data_base_offset + data_offset_startoff, font_data_base_offset + data_offset_endoff);
        bin_font_skip(boff * data_buffer_size);
        uint8_t *buffer_end = buffer + data_buffer_size;
        while (buffer < buffer_end)
        {
            bin_font_read1(buffer);
            buffer ++;
        }
        return true;
    }
    return false;
}

static bmf_BitmapFontFile vfile_obj;
static bmf_BitmapFont font_comp_obj = {
    .get_char_image = compf_get_char_data,
    .font_file = &vfile_obj,
    .char_width = 0,
    .char_height = 0,
    .ascii_width = NULL,
};
bmf_BitmapFont *font_comp = &font_comp_obj;

void init_comp_font(const uint8_t *FONT_BIN_DATA, uint16_t NOVAL_FONT_DSIZE) {
    vfile_obj.data = (void *) FONT_BIN_DATA;
    *((uint8_t *)&(font_comp->char_width)) = FONT_BIN_DATA[0];
    *((uint8_t *)&(font_comp->char_height)) = FONT_BIN_DATA[1];
    font_comp->ascii_width = FONT_BIN_DATA + 2;
}

