#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "reader.h"
#include "bin_data.h"
#include "screen.h"
#include "compfont.h"

#define BUFFER_WINDOW_SIZE 256
#define BUFFER_SIZE (BUFFER_WINDOW_SIZE * 4)
#define MAX_LINES 16
#define MIN(x,y) ( x > y ? y : x)
#define MAX(x,y) ( x < y ? y : x)

static uint8_t _text_buffer[BUFFER_SIZE];
// static uint32_t _line_start_ring_buffer[MAX_LINES];
// static uint8_t _line_start_head_p = 0;

static ReaderContext _ctx;

static void _ctx_seek (uint32_t abs_offset) {
    bin_text_reset();
    bin_text_skip((uint16_t) abs_offset);
}

static uint32_t _ctx_read (uint8_t *buffer, uint32_t len) {
    return bin_text_read(buffer, (uint16_t) len);
}

ReaderContext *reader_init(void) {
    _ctx.seek = _ctx_seek;
    _ctx.read = _ctx_read;
    // _ctx.text_file_size = 0;
    _ctx.text_file_size = bin_get_text_length();
    _ctx.text_buffer = _text_buffer;
    _ctx.buffer_start = 0;
    _ctx.buffer_end = BUFFER_SIZE;
    _ctx.buffer_current = 0;
    _ctx.buffer_page_end = 0;
    _ctx.frame = get_frame_buffer();
    _ctx.font = font_comp;
    // init buffer
    _ctx.seek(0);
    uint32_t read_count = _ctx.read(_ctx.text_buffer, BUFFER_SIZE);
    if (read_count < BUFFER_SIZE) {
        _ctx.text_file_size = read_count;
    }
    return &_ctx;
}

void reader_update_buffer_current(ReaderContext *ctx, uint32_t new_pos) {
    ctx->buffer_current = new_pos;
    if (new_pos - ctx->buffer_start < BUFFER_WINDOW_SIZE) {
        // read last data
        if (ctx->buffer_start == 0) {
            return;
        }
        uint32_t size_to_read = MIN(ctx->buffer_start, BUFFER_WINDOW_SIZE);
        uint32_t offset_to_read = ctx->buffer_start - size_to_read;
        memmove(ctx->text_buffer + size_to_read, ctx->text_buffer, (ctx->buffer_end - ctx->buffer_start - size_to_read));
        ctx->buffer_start -= size_to_read;
        ctx->buffer_end -= size_to_read;
        ctx->seek(offset_to_read);
        ctx->read(ctx->text_buffer, size_to_read); // FIXME: check read result
    } else if (ctx->buffer_end - new_pos < BUFFER_WINDOW_SIZE) {
        // read next data
        if (ctx->text_file_size > 0 && ctx->buffer_end >= ctx->text_file_size) {
            return; // end of file, do not read.
        }
        uint32_t size_to_read = MIN((ctx->text_file_size ? ctx->text_file_size : UINT32_MAX) - ctx->buffer_end, BUFFER_WINDOW_SIZE);
        uint32_t offset_to_read = ctx->buffer_end;
        memmove(ctx->text_buffer, ctx->text_buffer + size_to_read, (ctx->buffer_end - ctx->buffer_start - size_to_read));
        ctx->buffer_start += size_to_read;
        ctx->buffer_end += size_to_read;
        ctx->seek(offset_to_read);
        uint32_t read_count = ctx->read(ctx->text_buffer + (ctx->buffer_end - ctx->buffer_start - size_to_read), size_to_read);
        if (read_count < size_to_read) {
            // reach the end of the file
            ctx->text_file_size = offset_to_read + read_count;
            printf("Text Size: %u\n", ctx->text_file_size);
            printf("Buffer End: %u\n", ctx->buffer_end);
            for (int i=ctx->text_file_size-ctx->buffer_start-1; i>=ctx->text_file_size-ctx->buffer_start-16; i--) {
                printf("0x%02X\n", ctx->text_buffer[i]);
            }
            memset(ctx->text_buffer + (ctx->buffer_end - ctx->buffer_start - size_to_read + read_count), 0, size_to_read - read_count);
        }
    }
}

void reader_render_current_page(ReaderContext *ctx) {
    uint32_t text_limit = MIN(ctx->buffer_end, (ctx->text_file_size ? ctx->text_file_size : UINT32_MAX)) - ctx->buffer_current;
    uint8_t *text_start = ctx->text_buffer + (ctx->buffer_current - ctx->buffer_start);
    gfb_clear(ctx->frame, COLOR_CLEAR);
    uint32_t offset = bmf_draw_text(ctx->font, (char *)text_start, text_limit, ctx->frame, 0, 0, ctx->frame->width, ctx->frame->height, COLOR_SET);
    ctx->buffer_page_end = ctx->buffer_current + offset;
}

void reader_goto_next_page(ReaderContext *ctx) {
    if (ctx->buffer_page_end <= ctx->buffer_current) {
        uint32_t text_limit = MIN(ctx->buffer_end, ctx->text_file_size) - ctx->buffer_current;
        uint8_t *text_start = ctx->text_buffer + (ctx->buffer_current - ctx->buffer_start);
        uint32_t offset = bmf_get_text_offset(ctx->font, (char *)text_start, text_limit, ctx->frame->width, ctx->frame->height);
        ctx->buffer_page_end = ctx->buffer_current + offset;
    }
    reader_update_buffer_current(ctx, ctx->buffer_page_end);
}

void reader_goto_last_page(ReaderContext *ctx) {
    puts("start\n");
    uint8_t required_lines = ctx->frame->height / ctx->font->char_height;
    printf("req lines: %u\n", required_lines);
}
