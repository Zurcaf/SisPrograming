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

CFLAGS = -Wall -Wextra -std=c11 -g \
         -Iheader \
         -I../libconfig \
         $(EXTRA_INCLUDES) \
         $(shell sdl2-config --cflags) \
         $(shell pkg-config --cflags libconfig++ 2>/dev/null)

LDFLAGS = $(shell sdl2-config --libs) \
          $(shell pkg-config --libs libconfig++ 2>/dev/null) -lm

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
