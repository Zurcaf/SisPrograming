# =====================================================================
# Makefile for Universe Server
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
         -pthread \
         $(ASAN_FLAGS)

LDFLAGS = $(shell sdl2-config --libs) \
          $(shell pkg-config --libs libconfig++ libprotobuf-c libzmq) -lm \
          -pthread \
          $(ASAN_FLAGS)

# Executable name
SERVER_BIN = $(BIN_DIR)/new-universe-server

# Object files
SERVER_OBJ = $(OBJ_DIR)/new-universe-server.o
SERVER_FUNCS_OBJ = $(OBJ_DIR)/server_functions.o
PHYSICS_OBJ = $(OBJ_DIR)/physics-rules.o
SHARED_LIB = $(SHARED_DIR)/obj/libshared.a
LIBCONFIG_OBJ = $(LIBCONFIG_DIR)/config.o

# All dependencies
ALL_OBJS = $(SERVER_OBJ) $(SERVER_FUNCS_OBJ) $(PHYSICS_OBJ) $(SHARED_LIB) $(LIBCONFIG_OBJ)

# Default target
all: $(OBJ_DIR) $(BIN_DIR) $(SERVER_BIN)

# Link executable
$(SERVER_BIN): $(ALL_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Universe server built: $@"

# Compile main source
$(OBJ_DIR)/new-universe-server.o: src/new-universe-server.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile server functions source
$(OBJ_DIR)/server_functions.o: src/server_functions.c head/server_functions.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile physics source
$(OBJ_DIR)/physics-rules.o: src/physics-rules.c head/physics-rules.h | $(OBJ_DIR)
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
	rm -f $(SERVER_BIN)
	@echo "Universe server cleaned"

# Clean everything including binary
clean-all: clean
	@echo "Universe server fully cleaned"

.PHONY: all clean clean-all
