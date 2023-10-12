#include <stdint.h>

extern const uint8_t *FONT_BIN_DATA;
extern uint16_t NOVAL_FONT_DSIZE; 

uint16_t bin_read_uint16(const uint8_t *p);
void init_bin_data(void);
void bin_text_reset(void);
void bin_text_skip(uint16_t len);
uint8_t bin_text_read1(uint8_t *buffer);
void bin_font_reset(const uint8_t *source, uint16_t len);
void bin_font_skip(uint16_t len);
uint8_t bin_font_read1(uint8_t *buffer);
