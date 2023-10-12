BUILD = build
TARGET_NAME = reader
TARGET_WAT = $(BUILD)/$(TARGET_NAME).wat
TARGET_WASM = $(BUILD)/$(TARGET_NAME).wasm

CC = clang
LD = wasm-ld
WASM2WAT = wasm2wat
SIZE = llvm-size
ECHO = echo

# SRC += $(wildcard *.c littlefs/*.c)
SRC += $(wildcard src/*.c)
SRC += $(wildcard src/env/*.c)
SRC += $(wildcard src/libc/*.c)
SRC += $(wildcard src/libc/math/*.c)
SRC += $(wildcard src/libc/ctype/*.c)
SRC += $(wildcard src/libc/string/*.c)
SRC += $(wildcard src/libc/stdlib/*.c)
SRC += $(wildcard src/libc/umm_malloc/*.c)
SRC += $(wildcard src/libc/xprintf/*.c)
SRC += $(wildcard src/time/*.c)
SRC += $(wildcard src/gamepad/*.c)
SRC += $(wildcard src/graphic/*.c)
SRC += $(wildcard src/utils/*.c)
SRC += $(wildcard src/utils/uzlib/*.c)
OBJ := $(SRC:%.c=$(BUILD)/%.o)

# target
CFLAGS += --target=wasm32
CFLAGS += -std=gnu17 -Wall
CFLAGS += -nostdlib
CFLAGS += -fvisibility=hidden
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -flto
CFLAGS += -foptimize-sibling-calls
# wasm3 support features
CFLAGS += -mmutable-globals
CFLAGS += -mnontrapping-fptoint
CFLAGS += -msign-ext
CFLAGS += -mmultivalue
CFLAGS += -mbulk-memory
# opt
ifdef DEBUG
CFLAGS += -O0 -g3
else
CFLAGS += -Os
endif
# include header
CFLAGS += -Isrc/env
CFLAGS += -Isrc/libc
CFLAGS += -Isrc/time
CFLAGS += -Isrc/gamepad
CFLAGS += -Isrc/graphic
CFLAGS += -Isrc/utils

# link
LFLAGS += --no-entry
LFLAGS += --strip-all
LFLAGS += --gc-sections
LFLAGS += --lto-O3
# stack and memory size
LFLAGS += -z stack-size=4096
# LFLAGS += --initial-memory=65536
# LFLAGS += --import-memory


all: $(TARGET_WASM)

$(TARGET_WASM): $(OBJ)
	@$(ECHO) Linking...
	@$(LD) $^ $(LFLAGS) -o $@
	@$(SIZE) $(TARGET_WASM)
	@$(ECHO) done.

$(TARGET_WAT): $(TARGET_WASM)
	@$(WASM2WAT) -o $(TARGET_WAT) $(TARGET_WASM)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) -c -MMD $(CFLAGS) $< -o $@

clean:
	@$(ECHO) Cleaning...
	@rm -f $(TARGET_WASM)
	@rm -f $(TARGET_WAT)
	@rm -f $(OBJ)
	@$(ECHO) done.

run: all
	@$(ECHO) ================================
	../vgp/build/vgp_main $(TARGET_WASM)
