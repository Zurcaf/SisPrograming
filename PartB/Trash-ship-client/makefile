# =====================================================================
# Makefile for Trash Ship Client
# =====================================================================

CC = gcc

# Optional AddressSanitizer (enable with ASAN=1)
ASAN_FLAGS :=
ifeq ($(ASAN),1)
    ASAN_FLAGS := -fsanitize=address -fno-omit-frame-pointer
endif

# Detect OS and set PKG_CONFIG_PATH accordingly
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS with Homebrew
    export PKG_CONFIG_PATH := /opt/homebrew/lib/pkgconfig:$(PKG_CONFIG_PATH)
    EXTRA_INCLUDES := -I/opt/homebrew/include
else
    # Linux/WSL - uses system paths
    EXTRA_INCLUDES :=
endif

# Directories
SHARED_DIR = ../shared
LIBCONFIG_DIR = ../libconfig
OBJ_DIR = obj
BIN_DIR = ../0Executaveis

# Compiler flags (includes shared and libconfig headers)
CFLAGS = -Wall -Wextra -std=c11 -g \
         -Ihead \
         -I$(SHARED_DIR)/head \
         -I$(LIBCONFIG_DIR) \
         $(EXTRA_INCLUDES) \
         $(shell sdl2-config --cflags) \
         $(shell pkg-config --cflags libconfig++ libprotobuf-c libzmq) \
         $(ASAN_FLAGS)

LDFLAGS = $(shell sdl2-config --libs) \
          $(shell pkg-config --libs libconfig++ libprotobuf-c libzmq) -lm \
          $(ASAN_FLAGS)

# Executable name
CLIENT_BIN = $(BIN_DIR)/trash-ship-client

# Object files
CLIENT_OBJ = $(OBJ_DIR)/Universe-client.o
KEYBOARD_OBJ = $(OBJ_DIR)/keyboard-processing.o
SHARED_LIB = $(SHARED_DIR)/obj/libshared.a
LIBCONFIG_OBJ = $(LIBCONFIG_DIR)/config.o

# All dependencies
ALL_OBJS = $(CLIENT_OBJ) $(KEYBOARD_OBJ) $(SHARED_LIB) $(LIBCONFIG_OBJ)

# Default target
all: $(OBJ_DIR) $(BIN_DIR) $(CLIENT_BIN)

# Link executable
$(CLIENT_BIN): $(ALL_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Trash ship client built: $@"

# Compile main source
$(OBJ_DIR)/Universe-client.o: src/Universe-client.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile keyboard processing module
$(OBJ_DIR)/keyboard-processing.o: src/keyboard-processing.c head/keyboard-processing.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Ensure bin directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(CLIENT_BIN)
	@echo "Trash ship client cleaned"

# Clean everything including binary
clean-all: clean
	@echo "Trash ship client fully cleaned"

.PHONY: all clean clean-all
