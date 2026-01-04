#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <zmq.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ship_movement.pb-c.h"

#define UP 'u'
#define DOWN 'd'
#define LEFT 'l'
#define RIGHT 'r'

void *create_client_channel(char *server_addr);
void *create_server_channel();
// Client supplies its own password (no shared global on client side)
void send_connection_message(void *fd, char ch, const char *password);
void send_thrust_message(void *fd, char ch, char direction, bool active, const char *password);
int read_message(void *fd, char *message_type, char *id, char *direction, bool *thrust_active);
void send_response(void *fd, char *message);
void receive_response(void *fd, char *message);

#endif