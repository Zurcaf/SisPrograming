# =====================================================================
# Makefile for Dashboard (Placeholder)
# =====================================================================
# This makefile will be populated when the Dashboard component is implemented

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

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -g \
         -I$(SHARED_DIR)/head \
         -I$(LIBCONFIG_DIR) \
         $(EXTRA_INCLUDES) \
         $(shell sdl2-config --cflags) \
         $(shell pkg-config --cflags libconfig++ libprotobuf-c libzmq)

LDFLAGS = $(shell sdl2-config --libs) \
          $(shell pkg-config --libs libconfig++ libprotobuf-c libzmq) -lm

# Executable name
DASHBOARD_BIN = dashboard

# Placeholder - no sources yet
SOURCES = 
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))

# Default target
all:
	@echo "Dashboard component not yet implemented"
	@echo "Add source files and update this makefile"

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR)
	@echo "Dashboard cleaned"

.PHONY: all clean
