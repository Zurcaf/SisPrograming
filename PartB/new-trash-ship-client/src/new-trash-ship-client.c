#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include "../libconfig/config.h"
#include "../head/universe_data.h"
#include "../head/display.h"
#include "../head/Communication.h"
#include "../head/cursor_processing.h"
#include "../shared/head/validation.h"

/* Global flag for graceful shutdown */
static volatile sig_atomic_t shutdown_requested = 0;

/* Signal handler for SIGINT and SIGTERM */
static void signal_handler(int signum)
{
    (void)signum; /* unused */
    shutdown_requested = 1;
    printf("\n[Client] Shutdown signal received, disconnecting...\n");
}

int main()
{
    // =========================================================================
    // CONNECTION SETUP
    // =========================================================================

    // Load client-side configuration (only width/height needed here)
    const char *config_path = "client_init.conf";
    if (load_config(config_path) != 0)
    {
        fprintf(stderr, "Failed to load config at %s\n", config_path);
        return EXIT_FAILURE;
    }

    /* Register signal handlers for graceful shutdown */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    printf("[Client] Signal handlers registered\n");

    // Initialize connection with config values
    void *fd = create_client_channel(get_server_address_str(), get_server_port_int());
    if (fd == NULL)
    {
        fprintf(stderr, "Failed to create client channel\n");
        return EXIT_FAILURE;
    }

    // Connection credentials
    char client_id = '\0';
    char password[MAX_PASSWORD_LEN] = {0};
    char message[100];

    // Connection handshake loop
    while (1)
    {
        // Get unique client ID
        while (!isalpha(client_id))
        {
            printf("Enter your character (a-z, A-Z): ");
            printf("(Must be unique among connected players)\n");
            client_id = getchar();
            client_id = tolower(client_id);
        }

        // Get password for this client ID
        printf("Enter password for '%c': ", client_id);
        scanf("%63s", password);

        // Attempt connection
        send_connection_message(fd, client_id, password);
        receive_response_text(fd, message);

        if (strcmp(message, "OK") == 0)
        {
            printf("Connected successfully as '%c'\n", client_id);
            break; // Connection successful
        }
        else if (strcmp(message, "INVALID") == 0)
        {
            printf("Connection failed: Invalid credentials or character in use.\n");
            printf("Please try again with a different character or password.\n\n");
            client_id = '\0';
            memset(password, 0, MAX_PASSWORD_LEN);
            continue;
        }
        else
        {
            printf("Connection failed: Unable to connect '%s'\n", message);
            zmq_close(fd);
            memset(password, 0, MAX_PASSWORD_LEN);
            return EXIT_FAILURE;
        }
    }

    // =========================================================================
    // DISPLAY INITIALIZATION
    // =========================================================================

    SDL_Color background_color = {0, 0, 0, 255};
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    const int width = get_width_universe_int();
    const int height = get_height_universe_int();

    char window_title[256];
    snprintf(window_title, sizeof(window_title), "Universe Client - %c", client_id);

    if (init_display(window_title, width, height, &window, &renderer, &background_color) != 0)
    {
        fprintf(stderr, "Failed to initialize display\n");
        zmq_close(fd);
        memset(password, 0, MAX_PASSWORD_LEN);
        return EXIT_FAILURE;
    }

    // =========================================================================
    // MAIN GAME LOOP
    // =========================================================================

    int running = 1;
    SDL_Event event;
    Uint32 frame_start = 0;
    const Uint32 frame_delay = 33; // 30 FPS ≈ 33ms
    Uint32 last_state_request = 0;
    int server_alive = 1; // Track if server is still responding

    ServerResponse *latest_state = NULL; // cached snapshot to render

    printf("[Client] Starting game loop\n");

    while (running)
    {
        frame_start = SDL_GetTicks();

        /* Check for external shutdown signal */
        if (shutdown_requested)
        {
            running = 0;
            break;
        }

        // Process input events (delegated to keyboard-processing module)
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
                break;
            }

            ServerResponse *resp = NULL;
            int key_result = process_keyboard_input(&event, fd, client_id, password, &resp);

            if (key_result < 0)
            {
                if (resp)
                {
                    server_response__free_unpacked(resp, NULL);
                }
                running = 0;
                break;
            }

            if (resp)
            {
                // Check if server kicked us
                if (resp->message && (strcmp(resp->message, "KICKED") == 0 || strcmp(resp->message, "BYE") == 0))
                {
                    printf("[Client] Server disconnected this client\n");
                    server_response__free_unpacked(resp, NULL);
                    running = 0;
                    break;
                }
                
                if (resp->state)
                {
                    if (latest_state)
                    {
                        server_response__free_unpacked(latest_state, NULL);
                    }
                    latest_state = resp; // keep newest snapshot
                }
                else if (resp->message && strcmp(resp->message, "OK") == 0 && is_directional_key(event.key.keysym.sym))
                {
                    // Fallback feedback when server replies without state
                    char direction = get_direction_from_key(event.key.keysym.sym);
                    render_client_frame(renderer, &background_color, direction, width, height);
                    server_response__free_unpacked(resp, NULL);
                }
                else
                {
                    server_response__free_unpacked(resp, NULL);
                }
            }
        }

        // Periodic state sync to mirror server view (throttled to ~30Hz)
        Uint32 now = SDL_GetTicks();
        if (now - last_state_request >= frame_delay)
        {
            send_state_request(fd);
            ServerResponse *state_resp = receive_response_full(fd);
            last_state_request = now;

            if (state_resp == NULL)
            {
                printf("[Client] Server is not responding - connection lost\n");
                server_alive = 0;
                running = 0;
                break;
            }

            if (state_resp)
            {
                
                // Check if our ship was disconnected by checking the state
                if (state_resp->state && state_resp->state->ships)
                {
                    int my_index = (isalpha(client_id) ? (isupper(client_id) ? client_id - 'A' : client_id - 'a' + 26) : -1);
                    if (my_index >= 0 && my_index < (int)state_resp->state->n_ships)
                    {
                        if (!state_resp->state->ships[my_index]->connected)
                        {
                            printf("[Client] Server disconnected this client\n");
                            server_response__free_unpacked(state_resp, NULL);
                            running = 0;
                            break;
                        }
                    }
                }
                
                if (state_resp->state)
                {
                    if (latest_state)
                    {
                        server_response__free_unpacked(latest_state, NULL);
                    }
                    latest_state = state_resp;
                }
                else
                {
                    server_response__free_unpacked(state_resp, NULL);
                }
            }
        }

        // Render using latest snapshot (or fallback arrow)
        if (latest_state && latest_state->state)
        {
            render_snapshot_frame(renderer, &background_color, latest_state->state);
        }
        else
        {
            render_client_frame(renderer, &background_color, ' ', width, height);
        }

        // Maintain 30 FPS
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay)
        {
            SDL_Delay(frame_delay - frame_time);
        }
    }

    // =========================================================================
    // CLEANUP
    // =========================================================================

    printf("[Client] Shutting down\n");

    // Send a clean disconnect to server (only if server is still alive)
    if (server_alive && fd && isalpha(client_id))
    {
        send_disconnect_message(fd, client_id, password);
    }

    destroy_display(window, renderer);
    zmq_close(fd);
    if (latest_state)
    {
        server_response__free_unpacked(latest_state, NULL);
    }
    memset(password, 0, MAX_PASSWORD_LEN);

    return EXIT_SUCCESS;
}