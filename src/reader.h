#pragma once
#include <stdint.h>
#include "framebuf.h"
#include "bmfont.h"

typedef void (*ReaderMethodSeek) (uint32_t abs_offset); // seek to absloute offset
typedef uint32_t (*ReaderMethodRead) (uint8_t *buffer, uint32_t len); // read at most 'len' bytes, return readed count. 0 if failed or EOF.

typedef struct {
    ReaderMethodSeek seek;
    ReaderMethodRead read;
    uint32_t text_file_size;
    uint8_t *text_buffer;
    uint32_t buffer_start;
    uint32_t buffer_end;
    uint32_t buffer_current;
    uint32_t buffer_page_end;
    gfb_FrameBuffer *frame;
    bmf_BitmapFont *font;
} ReaderContext;

ReaderContext *reader_init(void);
void reader_render_current_page(ReaderContext *ctx);
void reader_goto_next_page(ReaderContext *ctx);
void reader_goto_last_page(ReaderContext *ctx);
