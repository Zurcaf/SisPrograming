#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "comm-fifo.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  


#include <zmq.h>


void * create_server_channel(){
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int response = zmq_bind (responder, "tcp://*:5555");
    return responder;
}


void read_message (void * fd, char * message_type, char * c, direction_t *d ){
    char message[100];
    zmq_recv (fd, message, 100, 0);
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


void send_response (void * fd, char * message){
    zmq_send(fd, message, strlen(message), 0);
}


void * create_client_channel(char * server_ip_addr){
    char server_zmq_addr[100];
    sprintf(server_zmq_addr, "tcp://%s:5555", server_ip_addr);
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    return requester;
}


void send_connection_message(void * fd, char ch){
    char message[100];
    sprintf(message, "CONNECT %c", ch);
    zmq_send (fd, message, 100, 0);
}

void send_movement_message(void * fd, char ch, direction_t d){
    char message[100];
    sprintf(message, "MOVE %c %c", ch, d);
    zmq_send (fd, message, 100, 0);
}
void receive_response (void * fd, char * message){
    zmq_recv (fd, message, 100, 0);
}
