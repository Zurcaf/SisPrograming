# =====================================================================
# Makefile for shared components (communication, display, data structures)
# =====================================================================

CC = gcc

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
SRC_DIR = src
HEADER_DIR = head
OBJ_DIR = obj
LIBCONFIG_DIR = ../libconfig

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -g \
         -I$(HEADER_DIR) \
         -I$(LIBCONFIG_DIR) \
         $(EXTRA_INCLUDES) \
         $(shell sdl2-config --cflags) \
         $(shell pkg-config --cflags libconfig++ libprotobuf-c libzmq)

LDFLAGS = $(shell sdl2-config --libs) \
          $(shell pkg-config --libs libconfig++ libprotobuf-c libzmq) -lm

# Protobuf generation
PROTO_FILE = ship_movement.proto
PROTO_C = $(SRC_DIR)/ship_movement.pb-c.c
PROTO_H = $(HEADER_DIR)/ship_movement.pb-c.h

# Source files (including generated protobuf)
SHARED_SOURCES = \
	Communication.c \
	display.c \
	universe_data.c \
	physics-rules.c \
	ship_movement.pb-c.c

# Object files (all in obj directory)
SHARED_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SHARED_SOURCES))

# Library archive
SHARED_LIB = $(OBJ_DIR)/libshared.a

# Default target: build shared library
# IMPORTANT: Generate protobuf files FIRST before compiling anything
all: $(OBJ_DIR) $(PROTO_C) $(PROTO_H) $(SHARED_LIB)

# Build static library from shared objects
$(SHARED_LIB): $(SHARED_OBJS)
	ar rcs $@ $^
	@echo "Shared library built: $@"

# Compile .c files to .o files (depends on proto files being generated)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(PROTO_H)
	$(CC) $(CFLAGS) -c $< -o $@

# Generate protobuf C code from .proto file
$(PROTO_C) $(PROTO_H): $(PROTO_FILE)
	protoc-c --c_out=. $<
	@mkdir -p $(SRC_DIR) $(HEADER_DIR)
	@mv -f ship_movement.pb-c.c $(SRC_DIR)/ 2>/dev/null || true
	@mv -f ship_movement.pb-c.h $(HEADER_DIR)/ 2>/dev/null || true
	@echo "Generated protobuf files"

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(PROTO_C) $(PROTO_H)
	@echo "Shared components cleaned"

.PHONY: all clean
