all: human-client server

server: server.c comm-fifo.h comm-fifo.h comm-fifo.c
	gcc -g -o server server.c comm-fifo.c -lncurses

human-client: human-client.c comm-fifo.h comm-fifo.c
	gcc -g -o human-client human-client.c comm-fifo.c -lncurses