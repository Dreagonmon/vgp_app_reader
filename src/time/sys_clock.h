#pragma once

#include <stdint.h>
#include <stdbool.h>

bool is_rtc_supported(void);
uint64_t rtc_time(void);
void rtc_set(uint64_t value);
int32_t ticks_ms(void);
int32_t ticks_add(int32_t t1, int32_t delta);
int32_t ticks_diff(int32_t t1, int32_t t2);
