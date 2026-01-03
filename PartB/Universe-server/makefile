# =====================================================================
# Makefile for Universe Server
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
SHARED_DIR = ../shared
LIBCONFIG_DIR = ../libconfig
OBJ_DIR = obj
BIN_DIR = ../0Executaveis

# Compiler flags (includes shared and libconfig headers)
CFLAGS = -Wall -Wextra -std=c11 -g \
         -I$(SHARED_DIR)/head \
         -I$(LIBCONFIG_DIR) \
         $(EXTRA_INCLUDES) \
         $(shell sdl2-config --cflags) \
         $(shell pkg-config --cflags libconfig++ libprotobuf-c libzmq)

LDFLAGS = $(shell sdl2-config --libs) \
          $(shell pkg-config --libs libconfig++ libprotobuf-c libzmq) -lm

# Executable name
SERVER_BIN = $(BIN_DIR)/universe-server

# Object files
SERVER_OBJ = $(OBJ_DIR)/Universe-server.o
SHARED_LIB = $(SHARED_DIR)/obj/libshared.a
LIBCONFIG_OBJ = $(LIBCONFIG_DIR)/config.o

# All dependencies
ALL_OBJS = $(SERVER_OBJ) $(SHARED_LIB) $(LIBCONFIG_OBJ)

# Default target
all: $(OBJ_DIR) $(BIN_DIR) $(SERVER_BIN)

# Link executable
$(SERVER_BIN): $(ALL_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Universe server built: $@"

# Compile main source
$(OBJ_DIR)/Universe-server.o: Universe-server.c
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
