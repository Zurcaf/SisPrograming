// Simple fuzz/test harness for the Universe server.
// Builds against existing shared Communication helpers so it produces
// the exact same protobuf envelopes as the real client. Run scenarios
// to verify the server rejects bad inputs and stays stable.
//
// Build (from PartB directory):
//   gcc -Wall -Wextra -std=c11 -g -Ishared/head -Ilibconfig \
//       -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -D_THREAD_SAFE \
//       -I/opt/homebrew/Cellar/libconfig/1.8.2/include \
//       -I/opt/homebrew/Cellar/protobuf-c/1.5.2_8/include \
//       -I/opt/homebrew/Cellar/zeromq/4.3.5_2/include \
//       -I/opt/homebrew/Cellar/libsodium/1.0.20/include \
//       tools/fuzz_client.c shared/obj/libshared.a libconfig/config.o \
//       -L/opt/homebrew/lib -lSDL2 -L/opt/homebrew/Cellar/libconfig/1.8.2/lib -lconfig++ \
//       -L/opt/homebrew/Cellar/protobuf-c/1.5.2_8/lib -lprotobuf-c \
//       -L/opt/homebrew/Cellar/zeromq/4.3.5_2/lib -lzmq -lm -pthread \
//       -o tools/fuzz_client
//
// Usage (server running on localhost):
//   ./tools/fuzz_client
//   ./tools/fuzz_client 127.0.0.1
//
// Scenarios covered:
//  1) valid connect + thrust
//  2) connect with wrong password after a valid one is set (should fail)
//  3) thrust with wrong password after valid connect (should fail)
//  4) empty password on connect (should fail)
//  5) oversized password (should fail)
//  6) invalid direction on thrust (should fail)
//  7) burst of 200 thrusts to trigger rate limit

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "../shared/head/Communication.h"
#include "../shared/head/validation.h"
#include "../shared/head/ship_movement.pb-c.h"

static bool maybe_response(void *fd, const char *label)
{
    zmq_msg_t zmq_msg;
    for (int attempt = 0; attempt < 30; attempt++) // up to ~300ms
    {
        zmq_msg_init(&zmq_msg);
        int n = zmq_msg_recv(&zmq_msg, fd, ZMQ_DONTWAIT);
        if (n == -1)
        {
            zmq_msg_close(&zmq_msg);
            usleep(10000); // 10ms
            continue;
        }

        ServerResponse *resp = server_response__unpack(NULL, n, zmq_msg_data(&zmq_msg));
        if (resp != NULL)
        {
            printf("[%s] response: %s\n", label, resp->message);
            server_response__free_unpacked(resp, NULL);
        }
        zmq_msg_close(&zmq_msg);
        return true;
    }

    printf("[%s] no response (expected if server drops bad msg)\n", label);
    return false;
}

static void scenario_valid_then_thrust(void *fd)
{
    printf("\n[Scenario 1] valid connect + thrust\n");
    send_connection_message(fd, 'a', "p1");
    maybe_response(fd, "connect");
    send_thrust_message(fd, 'a', 'u', true, "p1");
    maybe_response(fd, "thrust");
}

static void scenario_connect_wrong_password(void *fd)
{
    printf("\n[Scenario 2] wrong password on reconnect (should fail)\n");
    send_connection_message(fd, 'b', "goodpass");
    maybe_response(fd, "connect-good");
    // Attempt to reuse id with different password
    send_connection_message(fd, 'b', "badpass");
    maybe_response(fd, "connect-bad");
}

static void scenario_thrust_wrong_password(void *fd)
{
    printf("\n[Scenario 3] wrong password on thrust (should fail)\n");
    send_connection_message(fd, 'c', "p3");
    maybe_response(fd, "connect");
    send_thrust_message(fd, 'c', 'r', true, "wrong");
    maybe_response(fd, "thrust-bad");
}

static void scenario_empty_password(void *fd)
{
    printf("\n[Scenario 4] empty password connect (should fail)\n");
    send_connection_message(fd, 'd', "");
    maybe_response(fd, "connect-empty");
}

static void scenario_oversized_password(void *fd)
{
    printf("\n[Scenario 5] oversized password connect (should fail)\n");
    char longpw[256];
    memset(longpw, 'x', sizeof(longpw));
    longpw[sizeof(longpw) - 1] = '\0';
    send_connection_message(fd, 'e', longpw);
    maybe_response(fd, "connect-long");
}

static void scenario_invalid_direction(void *fd)
{
    printf("\n[Scenario 6] invalid direction on thrust (should fail)\n");
    send_connection_message(fd, 'f', "p6");
    maybe_response(fd, "connect");
    send_thrust_message(fd, 'f', 'Z', true, "p6");
    maybe_response(fd, "thrust-invalid-dir");
}

static void scenario_rate_limit(void *fd)
{
    printf("\n[Scenario 7] burst thrust to trigger rate limit\n");
    send_connection_message(fd, 'g', "p7");
    maybe_response(fd, "connect");
    for (int i = 0; i < 200; i++)
    {
        send_thrust_message(fd, 'g', 'u', true, "p7");
        maybe_response(fd, "thrust-burst");
        usleep(1000); // 1ms to keep burst tight
    }
}

int main(int argc, char **argv)
{
    const char *server_ip = (argc > 1) ? argv[1] : "localhost";
    int server_port = (argc > 2) ? atoi(argv[2]) : 45007;
    void *fd = create_client_channel(server_ip, server_port);

    scenario_valid_then_thrust(fd);
    scenario_connect_wrong_password(fd);
    scenario_thrust_wrong_password(fd);
    scenario_empty_password(fd);
    scenario_oversized_password(fd);
    scenario_invalid_direction(fd);
    scenario_rate_limit(fd);

    zmq_close(fd);
    return 0;
}
