CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g \
    -I/opt/homebrew/Cellar/libconfig/1.8.1/include

LDFLAGS = -L/opt/homebrew/Cellar/libconfig/1.8.1/lib -lconfig++

OBJ = config.o

all: $(OBJ)

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c -o config.o

clean:
	rm -f config.o
