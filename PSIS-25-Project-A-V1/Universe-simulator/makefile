CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g \
         -Iheader \
         -I../libconfig \
         -I/opt/homebrew/Cellar/libconfig/1.8.1/include \
         `sdl2-config --cflags`

LDFLAGS = `sdl2-config --libs` \
          -L/opt/homebrew/Cellar/libconfig/1.8.1/lib \
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
