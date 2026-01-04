#include "../head/Communication.h"
#include "../head/validation.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

/* Common socket tuning: short timeouts + low linger for graceful shutdown */
static void configure_socket(void *socket, int rcv_timeout_ms, int snd_timeout_ms)
{
    int linger_ms = 100; /* don't block long on close */
    zmq_setsockopt(socket, ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &rcv_timeout_ms, sizeof(rcv_timeout_ms));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &snd_timeout_ms, sizeof(snd_timeout_ms));
}

void *create_server_channel(int port)
{
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);

    /* Short timeouts to avoid blocking forever on shutdown */
    configure_socket(responder, 2000, 2000);

    char bind_addr[256];
    snprintf(bind_addr, sizeof(bind_addr), "tcp://*:%d", port);

    int response = zmq_bind(responder, bind_addr);
    if (response != 0)
    {
        printf("Failed to bind server socket to %s (errno=%d)\n", bind_addr, errno);
        exit(1);
    }
    printf("[Server] Listening on port %d\n", port);
    return responder;
}

void send_response(void *fd, const char *message_text)
{
    ServerResponse resp = SERVER_RESPONSE__INIT;
    resp.message = (char *)message_text;

    size_t len = server_response__get_packed_size(&resp);
    void *buf = malloc(len);
    server_response__pack(&resp, buf);

    int rc = zmq_send(fd, buf, len, 0);
    if (rc == -1)
    {
        printf("[Server] Failed to send response (errno=%d)\n", errno);
    }
    free(buf);
}

void send_response_with_state(void *fd, const char *message_text, const StateSnapshot *state)
{
    ServerResponse resp = SERVER_RESPONSE__INIT;
    resp.message = (char *)message_text;
    if (state != NULL)
    {
        resp.state = (StateSnapshot *)state;
    }

    size_t len = server_response__get_packed_size(&resp);
    void *buf = malloc(len);
    server_response__pack(&resp, buf);

    int rc = zmq_send(fd, buf, len, 0);
    if (rc == -1)
    {
        printf("[Server] Failed to send response with state (errno=%d)\n", errno);
    }
    free(buf);
}

void send_disconnect_message(void *fd, char ch, const char *password)
{
    Disconnect disc = DISCONNECT__INIT;
    Envelope env = ENVELOPE__INIT;

    char id_str[2] = {ch, '\0'};

    disc.client_id = id_str;
    disc.password = (char *)password;

    env.type = ENVELOPE__TYPE__DISCONNECT;
    env.disconnect = &disc;

    size_t len = envelope__get_packed_size(&env);
    void *buf = malloc(len);
    envelope__pack(&env, buf);

    int rc = zmq_send(fd, buf, len, 0);
    if (rc == -1)
    {
        printf("[Client] Failed to send disconnect message (errno=%d)\n", errno);
    }

    free(buf);
}

void *create_client_channel(const char *server_addr, int server_port)
{
    char server_zmq_addr[256];
    snprintf(server_zmq_addr, sizeof(server_zmq_addr), "tcp://%s:%d", server_addr, server_port);

    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);

    /* Short timeouts to avoid indefinite block; retries for transient failures */
    configure_socket(requester, 2000, 2000);

    int attempts = 3;
    int connected = 0;
    for (int i = 0; i < attempts; i++)
    {
        if (zmq_connect(requester, server_zmq_addr) == 0)
        {
            connected = 1;
            break;
        }
        printf("[Client] Connect failed (%s), retrying... (%d/%d)\n", server_zmq_addr, i + 1, attempts);
        usleep(200000); /* 200ms backoff */
    }

    if (!connected)
    {
        printf("Failed to connect to server at %s after %d attempts\n", server_zmq_addr, attempts);
        exit(1);
    }

    printf("[Client] Connected to %s\n", server_zmq_addr);
    return requester;
}

void send_connection_message(void *fd, char ch, const char *password)
{
    Connect connect = CONNECT__INIT;
    Envelope env = ENVELOPE__INIT;

    char id_str[2] = {ch, '\0'};

    connect.client_id = id_str;
    connect.password = (char *)password;

    env.type = ENVELOPE__TYPE__CONNECT;
    env.connect = &connect;

    size_t len = envelope__get_packed_size(&env);
    void *buf = malloc(len);
    envelope__pack(&env, buf);

    int rc = zmq_send(fd, buf, len, 0);
    if (rc == -1)
    {
        printf("[Client] Failed to send connection message (errno=%d)\n", errno);
    }

    free(buf);
}

void send_thrust_message(void *fd, char ch, char d, bool active, const char *password)
{
    Thrust thrust = THRUST__INIT;
    Envelope env = ENVELOPE__INIT;

    char id_str[2] = {ch, '\0'};
    char dir_str[2] = {d, '\0'};

    thrust.client_id = id_str;
    thrust.password = (char *)password;
    thrust.direction = dir_str;
    thrust.active = active;

    env.type = ENVELOPE__TYPE__THRUST;
    env.thrust = &thrust;

    size_t len = envelope__get_packed_size(&env);
    void *buf = malloc(len);
    envelope__pack(&env, buf);

    int rc = zmq_send(fd, buf, len, 0);
    if (rc == -1)
    {
        printf("[Client] Failed to send thrust message (errno=%d)\n", errno);
    }
    free(buf);
}

void send_state_request(void *fd)
{
    Envelope env = ENVELOPE__INIT;
    env.type = ENVELOPE__TYPE__STATE_REQUEST;

    size_t len = envelope__get_packed_size(&env);
    void *buf = malloc(len);
    envelope__pack(&env, buf);

    int rc = zmq_send(fd, buf, len, 0);
    if (rc == -1)
    {
        printf("[Client] Failed to send state request (errno=%d)\n", errno);
    }
    free(buf);
}

void receive_response_text(void *fd, char *buffer)
{
    ServerResponse *resp = receive_response_full(fd);
    if (resp)
    {
        strcpy(buffer, resp->message);
        server_response__free_unpacked(resp, NULL);
    }
}

ServerResponse *receive_response_full(void *fd)
{
    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);

    int n = zmq_msg_recv(&zmq_msg, fd, 0);
    if (n == -1)
    {
        zmq_msg_close(&zmq_msg);
        if (errno == EAGAIN)
        {
            /* Timeout */
            return NULL;
        }
        printf("[Comm] Receive failed (errno=%d)\n", errno);
        return NULL;
    }

    ServerResponse *resp = server_response__unpack(NULL, n, zmq_msg_data(&zmq_msg));
    zmq_msg_close(&zmq_msg);
    return resp; // caller frees
}