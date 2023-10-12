#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <env.h>
#include <_malloc.h>

#include "screen.h"
#include "framebuf.h"

#include "bin_data.h"
#include "compfont.h"
#include "ui_utils.h"

void init(void) {
    size_t mems = _init_memory(1);
    printf("Total Memory Available: %lu\n", mems);
    screen_init();
    gfb_clear(get_frame_buffer(), COLOR_CLEAR);
    screen_flush();
    // init data
    init_bin_data();
    puts("inited.\n");
}

WASM_EXPORT("vinit") void vinit() {
    init();
    uint8_t *text = malloc(6);
    bin_text_read1(text + 0);
    bin_text_read1(text + 1);
    bin_text_read1(text + 2);
    bin_text_read1(text + 3);
    bin_text_read1(text + 4);
    text[5] = '\0';
    ui_text_area(
        font_comp, (const char *) text, get_frame_buffer(),
        0, 0, get_screen_width(), get_screen_height() / 2,
        ui_ALIGN_HCENTER | ui_ALIGN_VCENTER,
        COLOR_SET, COLOR_CLEAR
    );
    ui_text_area(
        font_comp, "123abc", get_frame_buffer(),
        0, get_screen_height() / 2, get_screen_width(), get_screen_height() / 2,
        ui_ALIGN_HCENTER | ui_ALIGN_VCENTER,
        COLOR_SET, COLOR_CLEAR
    );
    screen_flush();
}

WASM_EXPORT("vloop") void vloop() {
}
