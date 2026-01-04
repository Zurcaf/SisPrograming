#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>

#include <config.h>

#include "physics-rules.h"
#include "../../shared/head/display.h"
#include "../../shared/head/thread_pool.h"
#include "../../shared/head/validation.h"
#include "../../shared/head/universe_data.h"
#include "../../shared/head/Communication.h"

// Shared universe data for server threads
typedef struct
{
    planet_t *planets;
    trash_t *trash;
    ship_t *ships;
    int n_trash;
    int n_planets;
    int max_n_trash;
    int width;
    int height;
    void *zmq_fd;
} universe_data_t;

// Context passed to worker threads
typedef struct
{
    thread_sync_t *sync;
    universe_data_t *universe;
    volatile bool running;
    volatile bool game_over;
    uint64_t last_message_time[52]; // Rate limiting: last message timestamp per ship
    int message_count[52];          // Rate limiting: message counter per ship
} server_context_t;

// Message reader (server side)
int read_message(void *fd, char *message_type, char *id, char *direction, bool *thrust_active);

// Resource cleanup
static void cleanup_resources(universe_data_t *universe, SDL_Window *win, SDL_Renderer *rend, thread_sync_t *sync);

// Worker thread entry points
void *physics_thread_func(void *arg);
void *communication_thread_func(void *arg);

#endif
