#include <env.h>

inline int32_t get_screen_format(void) {
    return get_feature(VFEATURE_SCREEN_COLOR_FORMAT);
}

inline uint16_t get_screen_width(void) {
    return (get_feature(VFEATURE_SCREEN_SIZE) >> 12) & 0xFFF;
}

inline uint16_t get_screen_height(void) {
    return get_feature(VFEATURE_SCREEN_SIZE) & 0xFFF;
}

inline bool is_gamepad_support(void) {
    return get_feature(VFEATURE_GAMEPAD_SUPPORT) > 0;
}

inline bool is_save_support(void) {
    return get_feature(VFEATURE_SAVE_CAPACITY) > 0;
}

inline int32_t get_save_capacity(void) {
    int32_t cap = get_feature(VFEATURE_SAVE_CAPACITY);
    if (cap > 0) {
        return cap;
    }
    return 0;
}

inline bool is_rtc_support(void) {
    return get_feature(VFEATURE_RTC_SUPPORT) > 0;
}

/* Feature 0x0001 */

inline void update_screen_buffer(uint8_t *buffer_p) {
    call1(VFUNC_UPDATE_SCREEN_BUFFER, (int32_t) buffer_p);
}

inline int32_t cpu_ticks_ms(void) {
    return call0(VFUNC_CPU_TICKS_MS);
}

inline void trace_put_char(char ch) {
    call1(VFUNC_TRACE_PUT_CHAR, ch);
}

inline void system_exit(void) {
    call0(VFUNC_SYSTEM_EXIT);
}

/* Feature 0x0002 */

inline uint32_t gamepad_status(void) {
    return call0(VFUNC_GAMEPAD_STATUS);
}

/* Feature 0x0003 */

inline void save_write(uint32_t offset, uint8_t byte) {
    call2(VFUNC_SAVE_WRITE, offset, byte);
}

inline void save_flush(void) {
    call0(VFUNC_SAVE_FLUSH);
}

inline uint8_t save_read(uint32_t offset) {
    return call1(VFUNC_SAVE_READ, offset);
}

/* Feature 0x0004 */

inline uint32_t rtc_get_h32(void) {
    return call0(VFUNC_RTC_GET_H32);
}

inline void rtc_set_h32(uint32_t value) {
    call1(VFUNC_RTC_SET_H32, value);
}

inline uint32_t rtc_get_l32(void) {
    return call0(VFUNC_RTC_GET_L32);
}

inline void rtc_set_l32(uint32_t value) {
    call1(VFUNC_RTC_SET_L32, value);
}
