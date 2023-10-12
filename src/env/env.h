#ifndef ENV_H
#define ENV_H

#include <stdint.h>
#include <stdbool.h>

#define WASM_IMPORT(mod,name) __attribute((import_module(mod),import_name(name)))
#define WASM_EXPORT(name) __attribute((export_name(name)))

WASM_IMPORT("env","get_feature")
int32_t get_feature(int32_t feature_id);

WASM_IMPORT("env","call0")
int32_t call0(int32_t fid);

WASM_IMPORT("env","call1")
int32_t call1(int32_t fid, int32_t p1);

WASM_IMPORT("env","call2")
int32_t call2(int32_t fid, int32_t p1, int32_t p2);

WASM_IMPORT("env","call3")
int32_t call3(int32_t fid, int32_t p1, int32_t p2, int32_t p3);

WASM_IMPORT("env","call4")
int32_t call4(int32_t fid, int32_t p1, int32_t p2, int32_t p3, int32_t p4);


#define VFEATURE_SCREEN_SIZE 0x0000
#define VFEATURE_SCREEN_COLOR_FORMAT 0x0001
#define VFEATURE_GAMEPAD_SUPPORT 0x0002
#define VFEATURE_SAVE_CAPACITY 0x0003
#define VFEATURE_RTC_SUPPORT 0x0004

#define VFUNC_UPDATE_SCREEN_BUFFER 0x000100
#define VFUNC_CPU_TICKS_MS 0x000101
#define VFUNC_TRACE_PUT_CHAR 0x000102
#define VFUNC_SYSTEM_EXIT 0x000103
#define VFUNC_GAMEPAD_STATUS 0x000200
#define VFUNC_SAVE_WRITE 0x000300
#define VFUNC_SAVE_FLUSH 0x000301
#define VFUNC_SAVE_READ 0x000302
#define VFUNC_RTC_GET_H32 0x000400
#define VFUNC_RTC_SET_H32 0x000401
#define VFUNC_RTC_GET_L32 0x000402
#define VFUNC_RTC_SET_L32 0x000403

/* screen */
#define VCOLOR_FORMAT_MVLSB 1
#define VCOLOR_FORMAT_GS8 2
/* gamepad */
#define KEY_MASK_UP    (0b1 << 5)
#define KEY_MASK_DOWN  (0b1 << 4)
#define KEY_MASK_LEFT  (0b1 << 3)
#define KEY_MASK_RIGHT (0b1 << 2)
#define KEY_MASK_A     (0b1 << 1)
#define KEY_MASK_B     (0b1 << 0)

int32_t get_screen_format(void);
uint16_t get_screen_width(void);
uint16_t get_screen_height(void);
bool is_gamepad_support(void);
bool is_save_support(void);
int32_t get_save_capacity(void);
bool is_rtc_support(void);

void update_screen_buffer(uint8_t *buffer_p);
int32_t cpu_ticks_ms(void);
void trace_put_char(char ch);
void system_exit(void);

uint32_t gamepad_status(void);

void save_write(uint32_t offset, uint8_t byte);
void save_flush(void);
uint8_t save_read(uint32_t offset);

uint32_t rtc_get_h32(void);
void rtc_set_h32(uint32_t value);
uint32_t rtc_get_l32(void);
void rtc_set_l32(uint32_t value);

#endif