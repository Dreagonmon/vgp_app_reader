#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <env.h>
#include <_malloc.h>

#include "screen.h"
#include "framebuf.h"
#include "bin_data.h"
#include "reader.h"
#include "gamepad.h"

static ReaderContext *ctx;

void init(void) {
    size_t mems = _init_memory(1);
    printf("Total Memory Available: %lu\n", mems);
    screen_init();
    gfb_clear(get_frame_buffer(), COLOR_CLEAR);
    screen_flush();
    // init data
    init_bin_data();
    ctx = reader_init();
    reader_render_current_page(ctx);
    puts("inited.\n");
}

WASM_EXPORT("vinit") void vinit() {
    init();
    for (int i=0; i<308; i++) {
        reader_goto_next_page(ctx);
    }
    reader_render_current_page(ctx);
    screen_flush();
}

WASM_EXPORT("vloop") void vloop() {
    uint32_t evt = gamepad_query_event();
    if (k_action(evt) == KACT_DOWN) {
        switch (k_value(evt))
        {
        case K_DOWN:
        case K_RIGHT:
            reader_goto_next_page(ctx);
            reader_render_current_page(ctx);
            screen_flush();
            break;
        case K_UP:
        case K_LEFT:
            reader_goto_last_page(ctx);
            reader_render_current_page(ctx);
            screen_flush();
            break;
        
        default:
            break;
        }
    }
}
