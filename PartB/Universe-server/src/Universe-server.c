#include "../head/server_functions.h"

int main()
{
    // =========================================================================
    // INITIALIZATION PHASE
    // =========================================================================

    const char *config_path = "server_init.conf";

    // Load configuration
    if (load_config(config_path) != 0)
    {
        fprintf(stderr, "Failed to load config at %s\n", config_path);
        return EXIT_FAILURE;
    }

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
        cleanup_resources(NULL, win, rend, NULL);
        return EXIT_FAILURE;
    }

    // Communication channel (ZMQ)
    void *zmq_fd = create_server_channel(get_server_port_int());
    if (zmq_fd == NULL)
    {
        fprintf(stderr, "Failed to create ZMQ server channel\n");
        cleanup_resources(NULL, win, rend, &thread_sync);
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
        cleanup_resources(&universe, win, rend, &thread_sync);
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
        cleanup_resources(&universe, win, rend, &thread_sync);
        return EXIT_FAILURE;
    }

    if (create_communication_thread(&comm_thread, communication_thread_func, &ctx) != 0)
    {
        fprintf(stderr, "Failed to create communication thread\n");
        ctx.running = false;
        pthread_join(physics_thread, NULL);
        cleanup_resources(&universe, win, rend, &thread_sync);
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
    cleanup_resources(&universe, win, rend, &thread_sync);

    printf("Server shutdown complete\n");
    return 0;
}