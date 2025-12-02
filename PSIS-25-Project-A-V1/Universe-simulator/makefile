CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -I../libconfig \
         -I/opt/homebrew/Cellar/libconfig/1.8.1/include \
         `sdl2-config --cflags`

LDFLAGS = `sdl2-config --libs` \
          -L/opt/homebrew/Cellar/libconfig/1.8.1/lib \
          -lconfig++ -lm

LIBCONFIG_DIR = ../libconfig
LIBCONFIG_OBJ = $(LIBCONFIG_DIR)/config.o

SRC = universe_simulator.c #universe_data.c display.c physics-rules.c
OBJ = $(SRC:.c=.o)
BIN = universe_simulator.exe

all: $(LIBCONFIG_OBJ) $(BIN)

$(BIN): $(OBJ) $(LIBCONFIG_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(LIBCONFIG_OBJ):
	$(MAKE) -C $(LIBCONFIG_DIR)

clean:
	rm -f $(OBJ) $(BIN)
	$(MAKE) -C $(LIBCONFIG_DIR) clean
