#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <uzlib/uzlib.h>
#include "_compressed_data.h"
#include "bin_data.h"

#define READ_WINDOW_SIZE (1 << 9)
#define READ_BUFFER_SIZE (READ_WINDOW_SIZE * 2)

const uint8_t *FONT_BIN_DATA = NULL;
uint16_t NOVAL_FONT_DSIZE = 0;
static struct uzlib_uncomp noval_text_uncomp;
static struct uzlib_uncomp noval_font_uncomp;

inline uint16_t bin_read_uint16(const uint8_t *p) {
    return (*p << 8) | (*(p + 1));
}

void bin_text_reset(void) {
    uzlib_uncompress_init(&noval_text_uncomp, NULL, 0);
    noval_text_uncomp.dest = noval_text_uncomp.dest_start;
    int res = uzlib_zlib_parse_header(&noval_text_uncomp);
    if (res < 0) {
        printf("Error parsing header: %d\n", res);
    }
}

uint8_t bin_ulibz_read1(struct uzlib_uncomp *ump) {
    uint8_t *dest = ump->dest;
    if (dest - ump->dest_start >= READ_BUFFER_SIZE) {
        memmove(ump->dest_start, dest - READ_WINDOW_SIZE, READ_WINDOW_SIZE);
        dest = ump->dest_start + READ_WINDOW_SIZE;
    }
    int res = uzlib_uncompress(ump);
    ump->dest_limit = dest + 1;
    if (res < 0) {
        if (ump->eof == 1) {
            return 0;
        }
        printf("Error uncompress: %d\n", res);
        abort();
    }
    return *dest;
}

void bin_text_skip(uint16_t len) {
    while (len > 0) {
        bin_ulibz_read1(&noval_text_uncomp);
        len --;
    }
}

uint8_t bin_text_read1(uint8_t *buffer) {
    uint8_t data = bin_ulibz_read1(&noval_text_uncomp);
    if (noval_text_uncomp.eof) {
        return 0;
    }
    if (buffer != NULL) {
        *buffer = data;
    }
    return 1;
}

void bin_font_reset(const uint8_t *source, uint16_t len) {
    noval_font_uncomp.source = source;
    noval_font_uncomp.source_limit = source + len;
    uzlib_uncompress_init(&noval_font_uncomp, NULL, 0);
    noval_font_uncomp.dest = noval_font_uncomp.dest_start;
    int res = uzlib_zlib_parse_header(&noval_font_uncomp);
    if (res < 0) {
        printf("Error parsing header: %d\n", res);
    }
}

void bin_font_skip(uint16_t len) {
    while (len > 0) {
        bin_ulibz_read1(&noval_font_uncomp);
        len --;
    }
}

uint8_t bin_font_read1(uint8_t *buffer) {
    uint8_t data = bin_ulibz_read1(&noval_font_uncomp);
    if (noval_font_uncomp.eof) {
        return 0;
    }
    if (buffer != NULL) {
        *buffer = data;
    }
    return 1;
}

void init_bin_data(void) {
    // text
    const uint8_t *NOVAL_TEXT = DATA_NOVAL + 2;
    uint16_t NOVAL_TEXT_DSIZE = bin_read_uint16(DATA_NOVAL);
    // font, export for external use
    FONT_BIN_DATA = NOVAL_TEXT + NOVAL_TEXT_DSIZE + 2;
    NOVAL_FONT_DSIZE = bin_read_uint16(NOVAL_TEXT + NOVAL_TEXT_DSIZE);
    // init zlib
    uzlib_init();
    noval_text_uncomp.dest_start = malloc(READ_BUFFER_SIZE);
    if (noval_text_uncomp.dest_start == NULL) {
        puts("Failed to init uncomp buffer\n");
    }
    noval_font_uncomp.dest_start = malloc(READ_BUFFER_SIZE);
    if (noval_font_uncomp.dest_start == NULL) {
        puts("Failed to init uncomp buffer\n");
    }
    noval_text_uncomp.source = NOVAL_TEXT;
    noval_text_uncomp.source_limit = NOVAL_TEXT + NOVAL_TEXT_DSIZE;
    noval_text_uncomp.source_read_cb = NULL;
    bin_text_reset();
}
