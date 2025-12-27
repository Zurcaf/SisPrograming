# libconfig Makefile

CC = gcc

# Detect Homebrew prefix (arm64: /opt/homebrew, Intel: /usr/local)
BREW_PREFIX := $(shell brew --prefix 2>/dev/null)

CFLAGS = -Wall -Wextra -std=c11 -g \
         $(if $(BREW_PREFIX),-I$(BREW_PREFIX)/include)

LDFLAGS = $(if $(BREW_PREFIX),-L$(BREW_PREFIX)/lib -lconfig++)

OBJ = config.o

all: $(OBJ)

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c -o config.o

clean:
	rm -f $(OBJ)
