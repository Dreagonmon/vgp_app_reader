#pragma once
#include <stdint.h>

#define FONT_COMPRESSED 0

uint16_t bin_read_uint16(const uint8_t *p);
void init_bin_data(void);
uint16_t bin_get_text_length(void);
void bin_text_reset(void);
void bin_text_skip(uint16_t len);
uint8_t bin_text_read1(uint8_t *buffer);
uint16_t bin_text_read(uint8_t *buffer, uint16_t len);
void bin_font_reset(const uint8_t *source, const uint8_t *source_end);
void bin_font_skip(uint16_t len);
uint8_t bin_font_read1(uint8_t *buffer);
