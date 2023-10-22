#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "reader.h"
#include "bin_data.h"
#include "screen.h"
#if FONT_COMPRESSED
#include "compfont.h"
#else
#include "rawfont.h"
#endif

#define BUFFER_WINDOW_SIZE 1024
#define BUFFER_SIZE (BUFFER_WINDOW_SIZE * 4)
#define MAX_LINES 16
#define MIN(x,y) ( x > y ? y : x)
#define MAX(x,y) ( x < y ? y : x)

static uint8_t _text_buffer[BUFFER_SIZE];
static const char *_line_start_ring_buffer[MAX_LINES];
static uint8_t _line_start_head_p = 0;

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
    #if FONT_COMPRESSED
    _ctx.font = font_comp;
    #else
    _ctx.font = font_raw;
    #endif
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

void _reader_ring_append(const char *value) {
    _line_start_ring_buffer[_line_start_head_p] = value;
    _line_start_head_p = (_line_start_head_p + 1) % MAX_LINES;
}

const char *_reader_ring_get_reverse(uint8_t r_offset) {
    return _line_start_ring_buffer[(_line_start_head_p + MAX_LINES - (r_offset % MAX_LINES)) % MAX_LINES];
}

void reader_goto_last_page(ReaderContext *ctx) {
    if (ctx->buffer_current <= 0) {
        return;
    }
    uint8_t required_lines = ctx->frame->height / ctx->font->char_height;
    uint8_t filled_lines = 0;
    uint16_t line_width = 0;
    const char *buf_lim = (char *)MAX(ctx->text_buffer, ctx->text_buffer + (ctx->buffer_current - BUFFER_WINDOW_SIZE - ctx->buffer_start));
    const char *next_paragraph = (char *)ctx->text_buffer + (ctx->buffer_current - ctx->buffer_start);
    const char *hard_break = NULL;
    const char *next_pos = (char *)ctx->text_buffer + (ctx->buffer_current - ctx->buffer_start);
    const char *pos = bmf_get_last_char(next_pos, buf_lim);
    // try to calc from a line start.
    // if last line is too far away, calc with char width instead.
    while (pos >= buf_lim) {
        char last_char = pos[0];
        if (last_char == '\n') {
            // find last '\n'
            const char *line_start = next_pos;
            uint8_t count_line = 0;
            // calc start from the line start
            while (line_start < next_paragraph) {
                _reader_ring_append(line_start);
                uint32_t offset = bmf_get_text_offset(ctx->font, line_start, next_paragraph - line_start, ctx->frame->width, ctx->font->char_height);
                line_start += offset;
                if (offset > 0) {
                    count_line ++;
                }
            }
            if (count_line >= required_lines) {
                // found
                pos = _reader_ring_get_reverse(required_lines);
                goto found_line_start;
            } else {
                // search again
                required_lines -= count_line;
                filled_lines = 0;
                line_width = 0;
                next_paragraph = next_pos;
            }
        }
        uint8_t char_width = bmf_get_text_width(ctx->font, pos, next_pos - pos);
        line_width += char_width;
        if (line_width > ctx->frame->width) {
            filled_lines += 1;
            line_width = char_width;
            if (filled_lines == required_lines) {
                hard_break = next_pos;
                while ((hard_break[0] == '\n' || hard_break[0] == '\r') && hard_break < next_paragraph) {
                    hard_break ++;
                }
            }
        }
        next_pos = pos;
        pos --;
    }
    if (hard_break != NULL) {
        pos = hard_break;
    } else {
        pos = buf_lim;
    }

    found_line_start:
    reader_update_buffer_current(ctx, ctx->buffer_start + ((uint8_t *)pos - ctx->text_buffer));
}
