#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>

#include <config.h>
#include "../head/universe_data.h"
#include "../head/display.h"
#include "../head/Communication.h"
#include "../head/physics-rules.h"
#include "../head/thread_pool.h"

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
            lock_universe(ctx->sync);

            if (strcmp("CONNECT", message_type) == 0 && ship_get_load_at(ctx->universe->ships, index) == -1)
            {
                if (index != -1)
                {
                    ship_set_load_at(ctx->universe->ships, index, 0);
                    printf("[Comm] Ship %c connected\n", ship_id);
                }
            }
            else if (strcmp("THRUST", message_type) == 0 && ship_get_load_at(ctx->universe->ships, index) != -1)
            {
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

        // Periodic operations
        uint64_t now_ms = get_time_ms();

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
        if (has_ship && (now_ms - last_spawn_ms) >= trash_spawn_interval_ms)
        {
            lock_universe(ctx->sync);
            if (addTrash(ctx->universe->trash, &ctx->universe->n_trash,
                         ctx->universe->max_n_trash, ctx->universe->width, ctx->universe->height))
            {
                printf("[Comm] Periodic spawn. Total trash: %d\n", ctx->universe->n_trash);
            }
            unlock_universe(ctx->sync);
            last_spawn_ms = now_ms;
        }

        // Rotate recycling planet every 30s
        // The recycling planet (mass=0) changes every 30 seconds
        // This forces players to dynamically adapt their strategy
        // 1. Find current recycling planet (the one with mass=0)
        // 2. Restore current planet's mass to 10
        // 3. Set next planet (circular) as new recycling planet (mass=0)
        if ((now_ms - last_recycle_ms) >= recycle_rotate_interval_ms)
        {
            lock_universe(ctx->sync);
            int current = -1;
            for (int pi = 0; pi < ctx->universe->n_planets; pi++)
            {
                if (planet_get_mass_at(ctx->universe->planets, pi) == 0)
                {
                    current = pi;
                    break;
                }
            }
            if (current >= 0)
            {
                int next = (current + 1) % ctx->universe->n_planets;
                planet_set_mass_at(ctx->universe->planets, current, 10);
                planet_set_mass_at(ctx->universe->planets, next, 0);
                printf("[Comm] Recycling planet rotated to index %d\n", next);
            }
            unlock_universe(ctx->sync);
            last_recycle_ms = now_ms;
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
    // Load configuration
    const char *config_paths[] = {
        "libconfig/init.conf",
        "../libconfig/init.conf",
        "../../PartB/libconfig/init.conf"};
    int config_loaded = 0;
    for (int i = 0; i < 3; i++)
    {
        load_config(config_paths[i]);
        if (get_width_universe_int() > 0)
        {
            config_loaded = 1;
            printf("Config loaded from: %s\n", config_paths[i]);
            break;
        }
    }
    if (!config_loaded)
    {
        fprintf(stderr, "Failed to load config from any path\n");
        return EXIT_FAILURE;
    }

    // Initialize SDL
    SDL_Window *win = NULL;
    SDL_Renderer *rend = NULL;
    SDL_Color background_color = {0, 0, 0, 255};

    if (init_display("Universe Simulator", get_width_universe_int(), get_height_universe_int(),
                     &win, &rend, &background_color) != 0)
    {
        return EXIT_FAILURE;
    }

    // Initialize thread synchronization
    thread_sync_t thread_sync;
    if (thread_sync_init(&thread_sync) != 0)
    {
        fprintf(stderr, "Failed to initialize thread synchronization\n");
        destroy_display(win, rend);
        return EXIT_FAILURE;
    }

    // Create universe data
    universe_data_t universe = {
        .planets = init_planets(get_n_planets_int(), get_width_universe_int(), get_height_universe_int()),
        .trash = init_trash(get_init_n_trash_int(), get_max_n_trash_int(),
                            get_width_universe_int(), get_height_universe_int()),
        .ships = init_ship(get_capacity_ship_int()),
        .n_trash = get_init_n_trash_int(),
        .n_planets = get_n_planets_int(),
        .max_n_trash = get_max_n_trash_int(),
        .width = get_width_universe_int(),
        .height = get_height_universe_int(),
        .zmq_fd = create_server_channel()};

    // Create server context
    server_context_t ctx = {
        .sync = &thread_sync,
        .universe = &universe,
        .running = true,
        .game_over = false};

    // Create worker threads
    pthread_t physics_thread, comm_thread;
    if (create_physics_thread(&physics_thread, physics_thread_func, &ctx) != 0)
    {
        fprintf(stderr, "Failed to create physics thread\n");
        destroy_display(win, rend);
        thread_sync_cleanup(&thread_sync);
        return EXIT_FAILURE;
    }

    if (create_communication_thread(&comm_thread, communication_thread_func, &ctx) != 0)
    {
        fprintf(stderr, "Failed to create communication thread\n");
        ctx.running = false;
        pthread_join(physics_thread, NULL);
        destroy_display(win, rend);
        thread_sync_cleanup(&thread_sync);
        return EXIT_FAILURE;
    }

    printf("[Main] Physics and Communication threads started\n");

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

    // Wait for worker threads to complete
    pthread_join(physics_thread, NULL);
    pthread_join(comm_thread, NULL);

    printf("[Main] All threads completed\n");

    // Cleanup
    destroy_display(win, rend);
    free(universe.planets);
    free(universe.trash);
    free(universe.ships);
    zmq_close(universe.zmq_fd);
    thread_sync_cleanup(&thread_sync);

    printf("Server shutdown complete\n");
    return 0;
}