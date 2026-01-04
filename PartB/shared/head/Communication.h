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

void *create_client_channel(const char *server_addr, int server_port);
void *create_server_channel(int port);
// Client supplies its own password (no shared global on client side)
void send_connection_message(void *fd, char ch, const char *password);
void send_thrust_message(void *fd, char ch, char direction, bool active, const char *password);
void send_state_request(void *fd);
int read_message(void *fd, char *message_type, char *id, char *direction, bool *thrust_active);

// Server responses (with optional full state snapshot)
void send_response(void *fd, const char *message_text);
void send_response_with_state(void *fd, const char *message_text, const StateSnapshot *state);

// Client receive helpers
// receive_response_text fills message buffer with response->message and discards state
void receive_response_text(void *fd, char *message);
// receive_response_full returns unpacked ServerResponse* (caller must free with server_response__free_unpacked)
ServerResponse *receive_response_full(void *fd);

#endif