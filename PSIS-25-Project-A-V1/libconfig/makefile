# libconfig Makefile

CC = gcc

# Detect OS and set paths accordingly
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS with Homebrew
    export PKG_CONFIG_PATH := /opt/homebrew/lib/pkgconfig:$(PKG_CONFIG_PATH)
    CFLAGS = -Wall -Wextra -std=c11 -g -I/opt/homebrew/include
    LDFLAGS = -L/opt/homebrew/lib -lconfig++
else
    # Linux/WSL - uses system paths and pkg-config
    CFLAGS = -Wall -Wextra -std=c11 -g $(shell pkg-config --cflags libconfig++ 2>/dev/null)
    LDFLAGS = $(shell pkg-config --libs libconfig++ 2>/dev/null)
endif

OBJ = config.o

all: $(OBJ)

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c -o config.o

clean:
	rm -f $(OBJ)
