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
#include "../../shared/head/universe_data.h"
#include "../../shared/head/display.h"
#include "../../shared/head/Communication.h"
#include "../head/physics-rules.h"
#include "../../shared/head/thread_pool.h"
#include "../../shared/head/validation.h"

// Global client password storage
client_password_t client_passwords[MAX_CLIENTS];

// Server-only message reader (moved from shared Communication.c)
// Now uses Envelope wrapper to identify message type explicitly
// Returns 0 on success, -1 on invalid input (but always sends a response)
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
        char client_id = env->thrust->client_id[0];
        char direction_char = env->thrust->direction[0];
        const char *password = env->thrust->password;

        // Validate thrust inputs: client_id, password, and direction
        if (!is_valid_client_id(client_id) ||
            !is_valid_direction(direction_char) ||
            !is_valid_client_password(client_id, password))
        {
            printf("[Security] Invalid THRUST: id=%c, dir=%c, auth=%s\n",
                   client_id, direction_char,
                   is_valid_client_password(client_id, password) ? "ok" : "fail");
            send_response(fd, "INVALID");
            envelope__free_unpacked(env, NULL);
            zmq_msg_close(&zmq_msg);
            return -1;
        }

        strcpy(message_type, "THRUST");
        *id = client_id;
        *direction = direction_char;
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
        char client_id = env->connect->client_id[0];
        const char *password = env->connect->password;
        int idx = get_client_index(client_id);

        // Validate basic inputs
        if (!is_valid_client_id(client_id) || !is_valid_password_format(password) || idx < 0)
        {
            printf("[Security] Invalid CONNECT inputs for client %c\n", client_id);
            send_response(fd, "INVALID");
            envelope__free_unpacked(env, NULL);
            zmq_msg_close(&zmq_msg);
            return -1;
        }

        // First time: set password; otherwise, verify it matches the stored one
        if (!client_passwords[idx].has_password)
        {
            if (!set_client_password_if_empty(client_id, password))
            {
                printf("[Security] Failed to set password for client %c\n", client_id);
                send_response(fd, "INVALID");
                envelope__free_unpacked(env, NULL);
                zmq_msg_close(&zmq_msg);
                return -1;
            }
        }
        else if (!is_valid_client_password(client_id, password))
        {
            printf("[Security] Invalid CONNECT credentials for client %c\n", client_id);
            send_response(fd, "INVALID");
            envelope__free_unpacked(env, NULL);
            zmq_msg_close(&zmq_msg);
            return -1;
        }

        // Mark client as authenticated
        mark_client_authenticated(client_id);

        strcpy(message_type, "CONNECT");
        *id = client_id;
        *direction = ' ';
        envelope__free_unpacked(env, NULL);
        zmq_msg_close(&zmq_msg);
        return 0;
    }

    envelope__free_unpacked(env, NULL);
    zmq_msg_close(&zmq_msg);
    send_response(fd, "INVALID");
    return -1;
}

/**
 * Shared data structures between threads
 * Protected by mutexes from thread_sync_t
 *
 * This structure contains all data shared between threads:
 * - planets, trash, ships: arrays of universe objects
 * - n_trash, n_planets, max_n_trash: counters and limits
 * - width, height: universe dimensions
 * - zmq_fd: ZeroMQ socket for client-server communication
 *
 * IMPORTANT: Any access to this data must be protected by mutexes
 * to avoid race conditions between physics and communication threads
 */
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

typedef struct
{
    thread_sync_t *sync;
    universe_data_t *universe;
    volatile bool running;
    volatile bool game_over;
    uint64_t last_message_time[52]; // Rate limiting: last message timestamp per ship
    int message_count[52];          // Rate limiting: message counter per ship
} server_context_t;

/**
 * Physics thread: updates trash and ships every 10ms
 * This thread does NOT access SDL
 */
void *physics_thread_func(void *arg)
{
    server_context_t *ctx = (server_context_t *)arg;
    uint64_t last_physics_ms = get_time_ms();
    const uint64_t physics_interval_ms = 10;

    printf("[Physics] Thread started\n");

    while (ctx->running)
    {
        uint64_t now_ms = get_time_ms();

        if (now_ms - last_physics_ms >= physics_interval_ms)
        {
            // Lock universe data
            lock_universe(ctx->sync);

            // Update physics for trash and ships
            update_physics(ctx->universe->trash, ctx->universe->n_trash,
                           ctx->universe->planets, ctx->universe->n_planets,
                           ctx->universe->ships, MAX_SHIPS,
                           ctx->universe->width, ctx->universe->height);

            unlock_universe(ctx->sync);

            last_physics_ms = now_ms;
        }

        // Sleep briefly to avoid spinning (max ~100Hz)
        usleep(1000); // 1ms
    }

    printf("[Physics] Thread exiting\n");
    return NULL;
}

/**
 * Communication thread: handles ZMQ messaging and periodic operations
 * Receives commands from clients, manages trash spawning and planet rotation
 */
void *communication_thread_func(void *arg)
{
    server_context_t *ctx = (server_context_t *)arg;
    uint64_t last_spawn_ms = get_time_ms();
    uint64_t last_recycle_ms = get_time_ms();
    const uint64_t trash_spawn_interval_ms = 10000;    // 10s
    const uint64_t recycle_rotate_interval_ms = 30000; // 30s
    char message_type[1024];
    char ship_id;
    char direction;
    bool thrust_active = false;

    printf("[Communication] Thread started\n");

    while (ctx->running)
    {
        // Try to read message (ZMQ_DONTWAIT is non-blocking)
        int result = read_message(ctx->universe->zmq_fd, message_type, &ship_id, &direction, &thrust_active);

        if (result != -1)
        {
            int index = ship_index(ship_id);

            // Input validation
            if (index == -1 ||
                (strcmp("CONNECT", message_type) != 0 && strcmp("THRUST", message_type) != 0))
            {
                printf("[Security] Invalid message type or ship ID\n");
                send_response(ctx->universe->zmq_fd, "INVALID");
                continue;
            }

            // Rate limiting: max 100 messages per second per ship
            uint64_t now_ms = get_time_ms();
            if (now_ms - ctx->last_message_time[index] < 10) // 10ms = 100 msg/s
            {
                ctx->message_count[index]++;
                if (ctx->message_count[index] > 10)
                {
                    printf("[Security] Rate limit exceeded for ship %c\n", ship_id);
                    send_response(ctx->universe->zmq_fd, "RATELIMIT");
                    continue;
                }
            }
            else
            {
                ctx->last_message_time[index] = now_ms;
                ctx->message_count[index] = 1;
            }

            lock_universe(ctx->sync);

            if (strcmp("CONNECT", message_type) == 0 && ship_get_load_at(ctx->universe->ships, index) == -1)
            {
                if (index != -1)
                {
                    ship_set_load_at(ctx->universe->ships, index, 0);
                    ctx->last_message_time[index] = now_ms;
                    ctx->message_count[index] = 0;
                    printf("[Comm] Ship %c connected\n", ship_id);
                }
            }
            else if (strcmp("THRUST", message_type) == 0 && ship_get_load_at(ctx->universe->ships, index) != -1)
            {
                // Validate thrust inputs
                if (!is_valid_direction(direction))
                {
                    printf("[Security] Invalid direction from ship %c: %c\n", ship_id, direction);
                    unlock_universe(ctx->sync);
                    send_response(ctx->universe->zmq_fd, "INVALID");
                    continue;
                }
                apply_thrust(ctx->universe->ships, index, direction, thrust_active);
            }

            unlock_universe(ctx->sync);
            send_response(ctx->universe->zmq_fd, "OK");
        }

        // Check for ship presence
        bool has_ship = false;
        lock_universe(ctx->sync);
        for (int si = 0; si < MAX_SHIPS; si++)
        {
            if (ship_get_load_at(ctx->universe->ships, si) >= 0)
            {
                has_ship = true;
                break;
            }
        }
        unlock_universe(ctx->sync);

        // Periodic operations (get time for periodic tasks)
        uint64_t now_periodic = get_time_ms();
        // Collision-based trash generation
        // When trash collides with planets, it generates more trash (cascade effect)
        // This increases game difficulty over time
        lock_universe(ctx->sync);
        if (has_ship && check4collisions(ctx->universe->trash, &ctx->universe->n_trash,
                                         ctx->universe->planets, ctx->universe->n_planets))
        {
            if (addTrash(ctx->universe->trash, &ctx->universe->n_trash,
                         ctx->universe->max_n_trash, ctx->universe->width, ctx->universe->height))
            {
                printf("[Comm] Collision! New trash created. Total: %d\n", ctx->universe->n_trash);
            }
        }
        unlock_universe(ctx->sync);

        // Periodic trash spawn every 10s
        if (has_ship && (now_periodic - last_spawn_ms) >= trash_spawn_interval_ms)
        {
            lock_universe(ctx->sync);
            if (addTrash(ctx->universe->trash, &ctx->universe->n_trash,
                         ctx->universe->max_n_trash, ctx->universe->width, ctx->universe->height))
            {
                printf("[Comm] Periodic spawn. Total trash: %d\n", ctx->universe->n_trash);
            }
            unlock_universe(ctx->sync);
            last_spawn_ms = now_periodic;
        }

        // Rotate recycling planet every 30s
        // The recycling planet marker changes every 30 seconds
        // This forces players to dynamically adapt their strategy
        // 1. Find current recycling planet (flagged)
        // 2. Clear current flag
        // 3. Set next planet (circular) as new recycling planet (flagged)
        if ((now_periodic - last_recycle_ms) >= recycle_rotate_interval_ms)
        {
            lock_universe(ctx->sync);
            int current = -1;
            for (int pi = 0; pi < ctx->universe->n_planets; pi++)
            {
                if (planet_is_recycling_at(ctx->universe->planets, pi))
                {
                    current = pi;
                    break;
                }
            }
            if (current >= 0)
            {
                int next = (current + 1) % ctx->universe->n_planets;
                planet_set_recycling_at(ctx->universe->planets, current, false);
                planet_set_recycling_at(ctx->universe->planets, next, true);
                printf("[Comm] Recycling planet rotated to index %d\n", next);
            }
            unlock_universe(ctx->sync);
            last_recycle_ms = now_periodic;
        }

        // Check for game over (only set flag, don't exit - let main thread display message)
        lock_game_state(ctx->sync);
        if (ctx->universe->n_trash >= ctx->universe->max_n_trash && !ctx->game_over)
        {
            ctx->game_over = true;
            printf("[Comm] GAME OVER - Max trash reached!\n");
        }
        unlock_game_state(ctx->sync);

        usleep(10000); // 10ms sleep to avoid busy waiting
    }

    printf("[Communication] Thread exiting\n");
    return NULL;
}

int main()
{
    // =========================================================================
    // INITIALIZATION PHASE
    // =========================================================================

    const char *config_path = "init.conf";

    // Load configuration
    load_config(config_path);
    if (get_width_universe_int() <= 0)
    {
        fprintf(stderr, "Failed to load config at %s\n", config_path);
        return EXIT_FAILURE;
    }
    printf("Config loaded from: %s\n", config_path);

    // Display initialization
    SDL_Window *win = NULL;
    SDL_Renderer *rend = NULL;
    SDL_Color background_color = {0, 0, 0, 255};

    if (init_display("Universe Simulator", get_width_universe_int(), get_height_universe_int(),
                     &win, &rend, &background_color) != 0)
    {
        fprintf(stderr, "Failed to initialize display\n");
        return EXIT_FAILURE;
    }

    // Thread synchronization initialization
    thread_sync_t thread_sync;
    if (thread_sync_init(&thread_sync) != 0)
    {
        fprintf(stderr, "Failed to initialize thread synchronization\n");
        destroy_display(win, rend);
        return EXIT_FAILURE;
    }

    // Communication channel (ZMQ)
    void *zmq_fd = create_server_channel();
    if (zmq_fd == NULL)
    {
        fprintf(stderr, "Failed to create ZMQ server channel\n");
        destroy_display(win, rend);
        thread_sync_cleanup(&thread_sync);
        return EXIT_FAILURE;
    }

    // =========================================================================
    // UNIVERSE DATA SETUP
    // =========================================================================

    // Initialize universe components
    int width = get_width_universe_int();
    int height = get_height_universe_int();

    universe_data_t universe = {
        .planets = init_planets(get_n_planets_int(), width, height),
        .trash = init_trash(get_init_n_trash_int(), get_max_n_trash_int(), width, height),
        .ships = init_ship(get_capacity_ship_int()),
        .n_trash = get_init_n_trash_int(),
        .n_planets = get_n_planets_int(),
        .max_n_trash = get_max_n_trash_int(),
        .width = width,
        .height = height,
        .zmq_fd = zmq_fd};

    // Validate universe initialization
    if (universe.planets == NULL || universe.trash == NULL || universe.ships == NULL)
    {
        fprintf(stderr, "Failed to initialize universe data\n");
        if (universe.planets)
            free(universe.planets);
        if (universe.trash)
            free(universe.trash);
        if (universe.ships)
            free(universe.ships);
        zmq_close(universe.zmq_fd);
        destroy_display(win, rend);
        thread_sync_cleanup(&thread_sync);
        return EXIT_FAILURE;
    }

    // Initialize client passwords
    init_client_passwords();
    printf("[Server] Client passwords initialized\n");

    // =========================================================================
    // CONTEXT AND THREAD SETUP
    // =========================================================================

    // Create server context
    server_context_t ctx = {
        .sync = &thread_sync,
        .universe = &universe,
        .running = true,
        .game_over = false};

    // Initialize rate limiting arrays
    for (int i = 0; i < 52; i++)
    {
        ctx.last_message_time[i] = 0;
        ctx.message_count[i] = 0;
    }

    // Create and start worker threads
    pthread_t physics_thread, comm_thread;
    if (create_physics_thread(&physics_thread, physics_thread_func, &ctx) != 0)
    {
        fprintf(stderr, "Failed to create physics thread\n");
        ctx.running = false;
        free(universe.planets);
        free(universe.trash);
        free(universe.ships);
        zmq_close(universe.zmq_fd);
        destroy_display(win, rend);
        thread_sync_cleanup(&thread_sync);
        return EXIT_FAILURE;
    }

    if (create_communication_thread(&comm_thread, communication_thread_func, &ctx) != 0)
    {
        fprintf(stderr, "Failed to create communication thread\n");
        ctx.running = false;
        pthread_join(physics_thread, NULL);
        free(universe.planets);
        free(universe.trash);
        free(universe.ships);
        zmq_close(universe.zmq_fd);
        destroy_display(win, rend);
        thread_sync_cleanup(&thread_sync);
        return EXIT_FAILURE;
    }

    printf("[Main] Physics and Communication threads started\n");

    // =========================================================================
    // MAIN EVENT LOOP
    // =========================================================================

    // Main thread: SDL event handling and display (30 FPS = 33ms)
    uint64_t last_display_ms = get_time_ms();
    const uint64_t display_interval_ms = 33;
    SDL_Event event;

    printf("[Main] Starting main event loop\n");

    while (ctx.running)
    {
        // Process SDL events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                ctx.running = false;
                break;
            }
        }

        // Check for game over (every iteration, not just on render)
        lock_game_state(&thread_sync);
        if (ctx.game_over)
        {
            unlock_game_state(&thread_sync);

            // Display final frame
            lock_universe(&thread_sync);
            render_frame(rend, &background_color,
                         universe.planets, universe.n_planets,
                         universe.trash, universe.n_trash,
                         universe.ships);
            unlock_universe(&thread_sync);

            // Show game over message
            SDL_MessageBoxColorScheme scheme = {
                .colors = {
                    {255, 0, 0}, // background red
                    {0, 0, 0},   // text black
                    {0, 0, 0},   // button border black
                    {0, 0, 0},   // button background black
                    {255, 0, 0}  // button selected (red)
                }};

            SDL_MessageBoxData msgbox = {0};
            msgbox.flags = SDL_MESSAGEBOX_ERROR;
            msgbox.window = win;
            msgbox.title = "Game Over";
            msgbox.message = "Game Over\nThe Universe is Full Off Trash! Humanity is Doomed!";
            msgbox.colorScheme = &scheme;

            SDL_ShowMessageBox(&msgbox, NULL);
            ctx.running = false;
            break;
        }
        unlock_game_state(&thread_sync);

        // Display update at 30Hz
        uint64_t now_ms = get_time_ms();
        if (now_ms - last_display_ms >= display_interval_ms)
        {
            lock_universe(&thread_sync);

            render_frame(rend, &background_color,
                         universe.planets, universe.n_planets,
                         universe.trash, universe.n_trash,
                         universe.ships);

            unlock_universe(&thread_sync);

            last_display_ms = now_ms;
        }

        usleep(1000); // 1ms sleep to avoid busy waiting
    }

    printf("[Main] Waiting for worker threads to finish\n");

    // =========================================================================
    // CLEANUP PHASE
    // =========================================================================

    // Wait for worker threads to complete
    pthread_join(physics_thread, NULL);
    pthread_join(comm_thread, NULL);

    printf("[Main] All threads completed\n");

    // Release resources
    destroy_display(win, rend);
    free(universe.planets);
    free(universe.trash);
    free(universe.ships);
    zmq_close(universe.zmq_fd);
    thread_sync_cleanup(&thread_sync);

    printf("Server shutdown complete\n");
    return 0;
}