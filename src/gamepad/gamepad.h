#pragma once
#include <stdint.h>
#include "env.h"

typedef enum Keys_t {
    K_UP = KEY_MASK_UP,
    K_DOWN = KEY_MASK_DOWN,
    K_LEFT = KEY_MASK_LEFT,
    K_RIGHT = KEY_MASK_RIGHT,
    K_A = KEY_MASK_A,
    K_B = KEY_MASK_B,
} Keys_t;

#define KACT_NOP           0x0
#define KACT_UP            0x1
#define KACT_DOWN          0x2
#define KACT_SHORT_CLICK   0x3
#define KACT_LONG_PRESS    0x4

#define k_action(x) (x & 0xFFFF)
#define k_value(x) ((x >> 16) & 0xFFFF)

uint32_t gamepad_query_event(void);
