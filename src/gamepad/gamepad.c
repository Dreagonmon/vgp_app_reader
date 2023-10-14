#include <stdint.h>
#include "sys_clock.h"
#include "gamepad.h"
#include "env.h"

static uint8_t key_status = 0;
static int32_t pressed_at[] = {
    -1,
    -1,
    -1,
    -1,
    -1,
    -1
};
static const uint8_t keys[] = {
    K_B,
    K_A,
    K_RIGHT,
    K_LEFT,
    K_DOWN,
    K_UP,
};

#define k_LONG_PRESS_TIMEOUT_MS 300
#define K_GET(x) (((0b1 << x) & key_status) >> x)
#define K_SET0(x) (key_status = (~(0b1 << x)) & key_status)
#define K_SET1(x) (key_status = (0b1 << x) | key_status)
#define K_HGET(p,x) ((p & keys[x]) > 0)
#define K_MAKE_EVENT(event, value) ((value << 16) | event)

uint32_t gamepad_query_event(void) {
    // Scan keypad
    int32_t now = ticks_ms();
    uint32_t pad = gamepad_status();
    uint8_t state, mstate, key_n;
    for (key_n = 0; key_n < sizeof(keys); key_n ++) {
        state = K_HGET(pad, key_n);
        mstate = K_GET(key_n);
        if (state && (!mstate)) {
            K_SET1(key_n);
            pressed_at[key_n] = now;
            return K_MAKE_EVENT(KACT_DOWN, keys[key_n]);
        } else if ((!state) && mstate) {
            if (pressed_at[key_n] >= 0) {
                pressed_at[key_n] = -1;
                return K_MAKE_EVENT(KACT_SHORT_CLICK, keys[key_n]);
            }
            K_SET0(key_n);
            return K_MAKE_EVENT(KACT_UP, keys[key_n]);
        }
        // ticks pressed time
        if (pressed_at[key_n] >= 0 && state) {
            if (ticks_diff(now, pressed_at[key_n]) > k_LONG_PRESS_TIMEOUT_MS) {
                pressed_at[key_n] = -1;
                return K_MAKE_EVENT(KACT_LONG_PRESS, keys[key_n]);
            }
        }
    }
    return K_MAKE_EVENT(KACT_NOP, 0);
}
