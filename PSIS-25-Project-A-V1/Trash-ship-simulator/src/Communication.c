#include "../head/Communication.h"

void * create_server_channel(){
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int response = zmq_bind (responder, "tcp://*:45007");
    if (response != 0){
        printf ("Failed to bind server socket\n");
        exit(1);
    }
    return responder;
}


int read_message(void *fd, char *message_type, char *id, char *direction)
{
    char buffer[256];

    int n = zmq_recv(fd, buffer, sizeof(buffer) - 1, ZMQ_DONTWAIT);

    if (n == -1) {
        if (zmq_errno() == EAGAIN) {
            // No message available, not an error
            return -1;
        }
        printf("ZMQ error: %s\n", zmq_strerror(zmq_errno()));
        return -1;
    }

    buffer[n] = '\0';  // safe termination

    if (sscanf(buffer, "%s %c %c", message_type, id, direction) < 1) {
        return -1;
    }

    return 0;
}



void send_response (void * fd, char * message){
    zmq_send(fd, message, strlen(message), 0);
}

void *create_client_channel(char *server_ip_addr) {
    char server_zmq_addr[100];
    sprintf(server_zmq_addr, "tcp://%s:45007", server_ip_addr);
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    if (zmq_connect(requester, server_zmq_addr) != 0) {
        printf("Failed to connect to server at %s\n", server_zmq_addr);
        exit(1);
    }
    return requester;  
}


void send_connection_message(void * fd, char ch){
    char message[100];
    sprintf(message, "CONNECT %c", ch);
    zmq_send (fd, message, 100, 0);
}

void send_movement_message(void * fd, char ch, char d){
    char message[100];
    sprintf(message, "MOVE %c %c", ch, d);
    zmq_send (fd, message, 100, 0);
}
void receive_response (void * fd, char * message){
    int n = zmq_recv (fd, message, 100, 0);
    if(n == -1)exit(1);
    message[n] = '\0';  // safe termination
}
