#include "../head/Communication.h"
#include <stdbool.h>

void *create_server_channel()
{
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    int response = zmq_bind(responder, "tcp://*:45007");
    if (response != 0)
    {
        printf("Failed to bind server socket\n");
        exit(1);
    }
    return responder;
}

// Modified to use zmq_msg_t and Protocol Buffers
int read_message(void *fd, char *message_type, char *id, char *direction, bool *thrust_active)
{
    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg); // [cite: 1010]

    // Use zmq_msg_recv (or zmq_recvmsg) instead of raw zmq_recv [cite: 1011]
    int n = zmq_msg_recv(&zmq_msg, fd, ZMQ_DONTWAIT);

    if (n == -1)
    {
        // Handle error or EAGAIN
        zmq_msg_close(&zmq_msg);
        return -1;
    }

    void *msg_data = zmq_msg_data(&zmq_msg); // [cite: 1012]
    size_t msg_len = zmq_msg_size(&zmq_msg);

    // ATTEMPT 1: Try unpacking as Thrust (keydown/keyup)
    // Attempt to deserialize as Thrust message (ship movement)
    // Protocol Buffers allows different message types to be sent
    // on the same socket, but doesn't include type identifier automatically.
    // So we try to deserialize as each possible type until we succeed.
    Thrust *thrust_msg = thrust__unpack(NULL, msg_len, msg_data);

    if (thrust_msg != NULL)
    {
        strcpy(message_type, "THRUST");
        *id = thrust_msg->client_id[0];
        *direction = thrust_msg->direction[0];
        if (thrust_active)
        {
            *thrust_active = thrust_msg->active;
        }
        thrust__free_unpacked(thrust_msg, NULL);
        zmq_msg_close(&zmq_msg);
        return 0;
    }

    // ATTEMPT 2: Try unpacking as Connect
    // If not Thrust, try as new client connection message
    Connect *con_msg = connect__unpack(NULL, msg_len, msg_data);

    if (con_msg != NULL)
    {
        strcpy(message_type, "CONNECT");
        *id = con_msg->client_id[0];
        *direction = ' '; // No direction in connect
        connect__free_unpacked(con_msg, NULL);
        zmq_msg_close(&zmq_msg);
        return 0;
    }

    zmq_msg_close(&zmq_msg);
    return -1; // Failed to unpack known messages
}

void send_response(void *fd, char *message_text)
{
    ServerResponse resp = SERVER_RESPONSE__INIT;
    resp.message = message_text;

    size_t len = server_response__get_packed_size(&resp);
    void *buf = malloc(len);
    server_response__pack(&resp, buf);

    zmq_send(fd, buf, len, 0);
    free(buf);
}

void *create_client_channel(char *server_ip_addr)
{
    char server_zmq_addr[100];
    sprintf(server_zmq_addr, "tcp://%s:45007", server_ip_addr);
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    if (zmq_connect(requester, server_zmq_addr) != 0)
    {
        printf("Failed to connect to server at %s\n", server_zmq_addr);
        exit(1);
    }
    return requester;
}

void send_connection_message(void *fd, char ch)
{
    // 1. Initialize the struct [cite: 940]
    Connect msg = CONNECT__INIT;

    // 2. Populate fields (Convert char to string for protobuf)
    char id_str[2] = {ch, '\0'};
    msg.client_id = id_str;

    // 3. Calculate size and allocate buffer [cite: 941]
    size_t len = connect__get_packed_size(&msg);
    void *buf = malloc(len);

    // 4. Pack the message [cite: 941]
    connect__pack(&msg, buf);

    // 5. Send using ZeroMQ [cite: 1025]
    // Note: You might need to send a message TYPE identifier first
    // if handling multiple message types on the server.
    zmq_send(fd, buf, len, 0);

    free(buf);
}

void send_thrust_message(void *fd, char ch, char d, bool active)
{
    Thrust msg = THRUST__INIT;

    char id_str[2] = {ch, '\0'};
    char dir_str[2] = {d, '\0'};

    msg.client_id = id_str;
    msg.direction = dir_str;
    msg.active = active;

    size_t len = thrust__get_packed_size(&msg);
    void *buf = malloc(len);

    thrust__pack(&msg, buf);

    zmq_send(fd, buf, len, 0);
    free(buf);
}

void receive_response(void *fd, char *buffer)
{
    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    int n = zmq_msg_recv(&zmq_msg, fd, 0);
    if (n == -1)
        exit(1);

    ServerResponse *resp = server_response__unpack(NULL, n, zmq_msg_data(&zmq_msg));

    if (resp != NULL)
    {
        strcpy(buffer, resp->message);
        server_response__free_unpacked(resp, NULL);
    }

    zmq_msg_close(&zmq_msg);
}