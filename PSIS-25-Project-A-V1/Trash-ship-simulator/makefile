# Makefile - builds two executables: universe-simulator (server) and trash-ship-simulator (client)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g \
         -Iheader \
         -I../libconfig \
         -I/opt/homebrew/Cellar/libconfig/1.8.1/include \
         `sdl2-config --cflags`

# Note: put libraries used for linking here (libs after objects)
LDFLAGS = `sdl2-config --libs` \
          -L/opt/homebrew/Cellar/libconfig/1.8.1/lib \
          -lconfig++ -lzmq -lm

# Directories
SRC_DIR = src
OBJ_DIR = obj
LIBCONFIG_DIR = ../libconfig

# Libconfig object file
LIBCONFIG_OBJ = $(LIBCONFIG_DIR)/config.o

# Gather all .c files inside src/ -> produce obj/*.o
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Explicit object lists for each executable (adapt names if your sources differ)
# Server executable (universe-simulator) needs Universe-server.o, universe_data.o, display.o, Communication.o
SERVER_OBJS = $(OBJ_DIR)/universe_data.o $(OBJ_DIR)/display.o $(OBJ_DIR)/Universe-server.o $(OBJ_DIR)/Communication.o $(LIBCONFIG_OBJ)

# Client executable (trash-ship-simulator) needs Universe-client.o, universe_data.o, display.o, Communication.o
CLIENT_OBJS = $(OBJ_DIR)/universe_data.o $(OBJ_DIR)/display.o $(OBJ_DIR)/Universe-client.o $(OBJ_DIR)/Communication.o $(LIBCONFIG_OBJ)

# Binaries
SERVER_BIN = universe-simulator
CLIENT_BIN = trash-ship-simulator

# Default: build both
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

# Build libconfig
$(LIBCONFIG_OBJ):
	$(MAKE) -C $(LIBCONFIG_DIR)

# Clean
clean:
	rm -f $(OBJ) $(SERVER_BIN) $(CLIENT_BIN)
	$(MAKE) -C $(LIBCONFIG_DIR) clean
