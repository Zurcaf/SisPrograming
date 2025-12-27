# Para o makefile funcionar, instale as seguintes dependências no seu sistema:
# sudo apt update
# sudo apt install build-essential libsdl2-dev libzmq3-dev libconfig++-dev protobuf-c-compiler libprotobuf-c-dev pkg-config


# Makefile - builds two executables: universe-simulator (server) and trash-ship-simulator (client)

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
OBJ_DIR = obj
LIBCONFIG_DIR = ../libconfig

# Libconfig object file
LIBCONFIG_OBJ = $(LIBCONFIG_DIR)/config.o

# Gather all .c files inside src/ -> produce obj/*.o
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Protobuf object file
PROTO_OBJ = $(OBJ_DIR)/ship_movement.pb-c.o

# Explicit object lists for each executable
SERVER_OBJS = \
	$(OBJ_DIR)/universe_data.o \
	$(OBJ_DIR)/display.o \
	$(OBJ_DIR)/Universe-server.o \
	$(OBJ_DIR)/Communication.o \
	$(PROTO_OBJ) \
	$(LIBCONFIG_OBJ)

CLIENT_OBJS = \
	$(OBJ_DIR)/universe_data.o \
	$(OBJ_DIR)/display.o \
	$(OBJ_DIR)/Universe-client.o \
	$(OBJ_DIR)/Communication.o \
	$(PROTO_OBJ) \
	$(LIBCONFIG_OBJ)

# Binaries
SERVER_BIN = universe-simulator
CLIENT_BIN = trash-ship-simulator

# -----------------------------
# Compiler and linker flags
# -----------------------------
CFLAGS = -Wall -Wextra -std=c11 -g \
         -Iheader \
         -I../libconfig \
         $(EXTRA_INCLUDES) \
         $(shell sdl2-config --cflags) \
         $(shell pkg-config --cflags libconfig++ libprotobuf-c libzmq)

LDFLAGS = $(shell sdl2-config --libs) \
          $(shell pkg-config --libs libconfig++ libprotobuf-c libzmq) -lm

# -----------------------------
# Default target: build both executables
# -----------------------------
all: $(OBJ_DIR) $(LIBCONFIG_OBJ) $(SERVER_BIN) $(CLIENT_BIN)

# Link server
$(SERVER_BIN): $(SERVER_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Link client
$(CLIENT_BIN): $(CLIENT_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile .c -> .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Build libconfig submodule
$(LIBCONFIG_OBJ):
	$(MAKE) -C $(LIBCONFIG_DIR)

# Clean build artifacts
clean:
	rm -f $(OBJ_DIR)/*.o $(SERVER_BIN) $(CLIENT_BIN)
	$(MAKE) -C $(LIBCONFIG_DIR) clean
