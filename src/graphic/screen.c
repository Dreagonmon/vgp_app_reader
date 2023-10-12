/**
 * hp-39gii screen virtual framebuffer
*/
#include <stddef.h>
#include "framebuf.h"
#include "screen.h"
#include "env.h"

static gfb_FrameBuffer *screen_frame = NULL;
static int32_t screen_mode = VCOLOR_FORMAT_MVLSB;

gfb_FrameBuffer *get_frame_buffer(void) {
    return screen_frame;
}

void screen_deinit(void) {
    if (screen_frame != NULL) {
        gfb_free(screen_frame);
        screen_frame = NULL;
    }
}

void screen_init(void) {
    screen_mode = get_feature(VFEATURE_SCREEN_COLOR_FORMAT);
    uint16_t w = get_screen_width();
    uint16_t h = get_screen_height();
    screen_deinit();
    if (screen_mode == VCOLOR_FORMAT_MVLSB) {
        screen_frame = gfb_new_mono_frame(w, h, COLOR_SET);
    } else if (screen_mode == VCOLOR_FORMAT_GS8) {
        screen_frame = gfb_new_gray_frame(w, h);
    }
}

void screen_flush(void) {
    if (screen_frame == NULL) {
        return;
    }
    if (screen_mode == VCOLOR_FORMAT_MVLSB) {
        update_screen_buffer((screen_frame->buffer) + 2);
    } else if (screen_mode == VCOLOR_FORMAT_GS8) {
        update_screen_buffer(screen_frame->buffer);
    }
}
