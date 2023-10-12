#include "sys_clock.h"
#include "env.h"

bool is_rtc_supported(void) {
    return get_feature(VFEATURE_RTC_SUPPORT) > 0;
}

uint64_t rtc_time(void) {
    uint64_t high = ((uint64_t) rtc_get_h32()) << 32;
    uint64_t low = (uint64_t) rtc_get_l32();
    return high | low;
}

void rtc_set(uint64_t value) {
    uint64_t high = (value >> 32) & UINT32_MAX;
    uint64_t low = value & UINT32_MAX;
    rtc_set_h32((uint32_t) high);
    rtc_set_l32((uint32_t) low);
}

int32_t ticks_ms(void) {
    return cpu_ticks_ms() & INT32_MAX;
}

int32_t ticks_add(int32_t t1, int32_t delta) {
    return (t1 + delta) & INT32_MAX;
}

int32_t ticks_diff(int32_t t1, int32_t t2) {
    int32_t half = (INT32_MAX / 2) + 1;
    return ((t1 - t2 + half) & INT32_MAX) - half;
}
