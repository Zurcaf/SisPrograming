UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	# Homebrew default install prefix on macOS
	LIBCONFIG_PREFIX ?= /opt/homebrew/opt/libconfig
	SDL_INCLUDE_DIR ?= /opt/homebrew/include
else
	# Typical system prefix on Linux (override with LIBCONFIG_PREFIX if needed)
	LIBCONFIG_PREFIX ?= /usr
	SDL_INCLUDE_DIR ?= /usr/include
endif

CC = gcc
SDL_CFLAGS ?= $(shell sdl2-config --cflags)
SDL_LIBS ?= $(shell sdl2-config --libs)
CFLAGS = -Wall -Wextra -std=c11 -g \
		 -I$(HDR_DIR) \
		 -I../libconfig \
		 -I$(LIBCONFIG_PREFIX)/include \
	 	 -I$(SDL_INCLUDE_DIR) \
		 $(SDL_CFLAGS)

LDFLAGS = $(SDL_LIBS) \
		  -L$(LIBCONFIG_PREFIX)/lib \
		  -lconfig++ -lm

# Directories
SRC_DIR = src
HDR_DIR = head
OBJ_DIR = obj
LIBCONFIG_DIR = ../libconfig

# Libconfig object file
LIBCONFIG_OBJ = $(LIBCONFIG_DIR)/config.o

# Gather all .c files inside src/
SRC = $(wildcard $(SRC_DIR)/*.c)

# Produce corresponding obj/*.o filenames
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

BIN = universe_simulator

all: $(OBJ_DIR) $(LIBCONFIG_OBJ) $(BIN)

# Link the final executable
$(BIN): $(OBJ) $(LIBCONFIG_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile .c → .o into obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Build libconfig
$(LIBCONFIG_OBJ):
	$(MAKE) -C $(LIBCONFIG_DIR)

# Clean
clean:
	rm -f $(OBJ) $(BIN)
	$(MAKE) -C $(LIBCONFIG_DIR) clean
