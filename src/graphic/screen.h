/**
 * framebuffer screen
*/
#pragma once

#include "framebuf.h"

/* get screen framebuffer */
gfb_FrameBuffer *get_frame_buffer(void);
void screen_deinit(void);
void screen_init(void);
void screen_flush(void);
