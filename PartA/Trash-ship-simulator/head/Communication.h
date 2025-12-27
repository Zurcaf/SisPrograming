#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <zmq.h>
#include <stdlib.h>
#include <string.h>
#include "ship_movement.pb-c.h"


#define UP 'u'
#define DOWN 'd'
#define LEFT 'l'
#define RIGHT 'r'


void * create_client_channel(char * server_addr);
void * create_server_channel();
void send_connection_message(void * fd, char ch);
void send_movement_message(void * fd, char ch, char direction);
int read_message (void * fd, char * message_type, char *id, char * direction);
void send_response (void * fd, char * message);
void receive_response (void * fd, char * message);

#endif