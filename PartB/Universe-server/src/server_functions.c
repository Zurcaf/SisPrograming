#include "../head/server_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Helpers to build and free a state snapshot for clients
static void build_state_snapshot(const universe_data_t *universe, StateSnapshot *snapshot)
{
    state_snapshot__init(snapshot);

    snapshot->width = universe->width;
    snapshot->height = universe->height;

    // Planets
    snapshot->n_planets = universe->n_planets;
    if (snapshot->n_planets > 0)
    {
        snapshot->planets = calloc(snapshot->n_planets, sizeof(PlanetState *));
        for (size_t i = 0; i < snapshot->n_planets; i++)
        {
            snapshot->planets[i] = calloc(1, sizeof(PlanetState));
            planet_state__init(snapshot->planets[i]);
            snapshot->planets[i]->x = planet_get_x_at(universe->planets, (int)i);
            snapshot->planets[i]->y = planet_get_y_at(universe->planets, (int)i);
            snapshot->planets[i]->mass = planet_get_mass_at(universe->planets, (int)i);
            snapshot->planets[i]->is_recycling = planet_is_recycling_at(universe->planets, (int)i);
        }
    }

    // Trash
    snapshot->n_trash = universe->n_trash;
    if (snapshot->n_trash > 0)
    {
        snapshot->trash = calloc(snapshot->n_trash, sizeof(TrashState *));
        for (size_t i = 0; i < snapshot->n_trash; i++)
        {
            snapshot->trash[i] = calloc(1, sizeof(TrashState));
            trash_state__init(snapshot->trash[i]);
            snapshot->trash[i]->x = trash_get_x_at(universe->trash, (int)i);
            snapshot->trash[i]->y = trash_get_y_at(universe->trash, (int)i);
            snapshot->trash[i]->mass = trash_get_mass_at(universe->trash, (int)i);
        }
    }

    // Ships (fixed 52)
    snapshot->n_ships = MAX_SHIPS;
    snapshot->ships = calloc(snapshot->n_ships, sizeof(ShipState *));
    for (size_t i = 0; i < snapshot->n_ships; i++)
    {
        snapshot->ships[i] = calloc(1, sizeof(ShipState));
        ship_state__init(snapshot->ships[i]);
        snapshot->ships[i]->x = ship_get_x_at(universe->ships, (int)i);
        snapshot->ships[i]->y = ship_get_y_at(universe->ships, (int)i);
        snapshot->ships[i]->load = ship_get_load_at(universe->ships, (int)i);
        snapshot->ships[i]->capacity = ship_get_capacity_at(universe->ships, (int)i);
        snapshot->ships[i]->connected = (ship_get_load_at(universe->ships, (int)i) >= 0);
    }
}

static void free_state_snapshot(StateSnapshot *snapshot)
{
    if (snapshot->planets)
    {
        for (size_t i = 0; i < snapshot->n_planets; i++)
        {
            free(snapshot->planets[i]);
        }
        free(snapshot->planets);
    }

    if (snapshot->trash)
    {
        for (size_t i = 0; i < snapshot->n_trash; i++)
        {
            free(snapshot->trash[i]);
        }
        free(snapshot->trash);
    }

    if (snapshot->ships)
    {
        for (size_t i = 0; i < snapshot->n_ships; i++)
        {
            free(snapshot->ships[i]);
        }
        free(snapshot->ships);
    }
}

// Provide global storage for client passwords (declared extern in validation.h)
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

    if (env->type == ENVELOPE__TYPE__STATE_REQUEST)
    {
        strcpy(message_type, "STATE");
        *id = ' ';
        *direction = ' ';
        if (thrust_active)
        {
            *thrust_active = false;
        }
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
            // Handle STATE snapshot requests immediately (no rate limit or ID required)
            if (strcmp("STATE", message_type) == 0)
            {
                lock_universe(ctx->sync);
                StateSnapshot snapshot;
                build_state_snapshot(ctx->universe, &snapshot);
                unlock_universe(ctx->sync);

                send_response_with_state(ctx->universe->zmq_fd, "OK", &snapshot);
                free_state_snapshot(&snapshot);
                continue;
            }

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

            // Build snapshot under lock
            StateSnapshot snapshot;
            build_state_snapshot(ctx->universe, &snapshot);

            unlock_universe(ctx->sync);

            send_response_with_state(ctx->universe->zmq_fd, "OK", &snapshot);
            free_state_snapshot(&snapshot);
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

// Centralized cleanup helper to avoid repeating frees/teardown
static void cleanup_resources(universe_data_t *universe, SDL_Window *win, SDL_Renderer *rend, thread_sync_t *sync)
{
    if (rend || win)
    {
        destroy_display(win, rend);
    }

    if (universe)
    {
        if (universe->planets)
            free(universe->planets);
        if (universe->trash)
            free(universe->trash);
        if (universe->ships)
            free(universe->ships);
        if (universe->zmq_fd)
            zmq_close(universe->zmq_fd);
    }

    if (sync)
    {
        thread_sync_cleanup(sync);
    }
}
