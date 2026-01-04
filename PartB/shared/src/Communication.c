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

// Now uses Envelope wrapper to identify message type explicitly
int read_message(void *fd, char *message_type, char *id, char *direction, bool *thrust_active)
{
    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    int n = zmq_msg_recv(&zmq_msg, fd, ZMQ_DONTWAIT);

    if (n == -1)
    {
        zmq_msg_close(&zmq_msg);
        return -1;
    }

    Envelope *env = envelope__unpack(NULL, n, zmq_msg_data(&zmq_msg));
    if (env == NULL)
    {
        zmq_msg_close(&zmq_msg);
        return -1;
    }

    if (env->type == ENVELOPE__TYPE__THRUST && env->thrust != NULL)
    {
        strcpy(message_type, "THRUST");
        *id = env->thrust->client_id[0];
        *direction = env->thrust->direction[0];
        if (thrust_active)
        {
            *thrust_active = env->thrust->active;
        }
        envelope__free_unpacked(env, NULL);
        zmq_msg_close(&zmq_msg);
        return 0;
    }

    if (env->type == ENVELOPE__TYPE__CONNECT && env->connect != NULL)
    {
        strcpy(message_type, "CONNECT");
        *id = env->connect->client_id[0];
        *direction = ' ';
        envelope__free_unpacked(env, NULL);
        zmq_msg_close(&zmq_msg);
        return 0;
    }

    envelope__free_unpacked(env, NULL);
    zmq_msg_close(&zmq_msg);
    return -1;
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
    Connect connect = CONNECT__INIT;
    Envelope env = ENVELOPE__INIT;

    char id_str[2] = {ch, '\0'};
    connect.client_id = id_str;

    env.type = ENVELOPE__TYPE__CONNECT;
    env.connect = &connect;

    size_t len = envelope__get_packed_size(&env);
    void *buf = malloc(len);
    envelope__pack(&env, buf);

    zmq_send(fd, buf, len, 0);

    free(buf);
}

void send_thrust_message(void *fd, char ch, char d, bool active)
{
    Thrust thrust = THRUST__INIT;
    Envelope env = ENVELOPE__INIT;

    char id_str[2] = {ch, '\0'};
    char dir_str[2] = {d, '\0'};

    thrust.client_id = id_str;
    thrust.direction = dir_str;
    thrust.active = active;

    env.type = ENVELOPE__TYPE__THRUST;
    env.thrust = &thrust;

    size_t len = envelope__get_packed_size(&env);
    void *buf = malloc(len);
    envelope__pack(&env, buf);

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