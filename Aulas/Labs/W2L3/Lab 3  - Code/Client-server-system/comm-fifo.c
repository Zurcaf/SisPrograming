#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "comm-fifo.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  


int create_server_channel(){
    int fd;
    unlink(FIFO_NAME);
    if(mkfifo(FIFO_NAME, 0666)!=0){
        printf("problem creating the fifo\n");
        exit(-1);
    }else{
        printf("fifo created\n");
    }

	fd = open(FIFO_NAME, O_RDONLY);
    if(fd== -1){
        printf("problem opening the fifo\n");
        exit(-1);   
	}
	printf("fifo just opened\n");
    return fd;
}

int create_client_channel(){
    int fd;

    while((fd = open(FIFO_NAME, O_WRONLY))== -1){
	  if(mkfifo(FIFO_NAME, 0666)!=0){
			printf("problem creating the fifo\n");
			exit(-1);
	  }else{
		  printf("fifo created\n");
	  }
	}
	printf("fifo just opened\n");
    return fd;
}


void send_connection_message(int fd, char ch){
    char message[100];
    sprintf(message, "CONNECT %c", ch);
    write(fd, message, 100);
}

void send_movement_message(int fd, char ch, direction_t d){
    char message[100];
    sprintf(message, "MOVE %c %c", ch, d);
    write(fd, message, 100);
}

void read_message (int fd, char * message_type, char * c, direction_t *d ){
    char message[100];
    read(fd, message, 100);
    d[0]='\0';
    sscanf(message, "%s %c %c", message_type, c, d);
    return;
    sscanf(message, "%s", message_type);
    if (strcmp(message_type, "CONNECT")==0){
        sscanf(message, "%s", message_type, c);
    }
    if (strcmp(message_type, "MOVE") ==0){
        sscanf(message, "%s", message_type, c, d);
    }

}
