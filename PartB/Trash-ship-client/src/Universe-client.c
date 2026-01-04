#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include "../libconfig/config.h"
#include "../head/universe_data.h"
#include "../head/display.h"
#include "../head/Communication.h"
#include "../shared/head/validation.h"

int main()
{
    // =========================================================================
    // CONNECTION SETUP
    // =========================================================================

    // Initialize connection
    void *fd = create_client_channel("localhost");
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
        receive_response(fd, message);

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
            printf("Connection failed: Unexpected server response '%s'\n", message);
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

    char window_title[256];
    snprintf(window_title, sizeof(window_title), "Universe Client - %c", client_id);

    if (init_display(window_title, 400, 400, &window, &renderer, &background_color) != 0)
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

    printf("[Client] Starting game loop\n");

    while (running)
    {
        frame_start = SDL_GetTicks();

        // Process input events
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;

            case SDL_KEYDOWN:
                // Ignore key repeats (OS automatic repeats)
                if (event.key.repeat)
                    break;

                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = 0;
                    break;

                // Directional inputs: send thrust and render feedback
                case SDLK_UP:
                case SDLK_DOWN:
                case SDLK_LEFT:
                case SDLK_RIGHT:
                {
                    char direction = (event.key.keysym.sym == SDLK_UP) ? 'u' : (event.key.keysym.sym == SDLK_DOWN) ? 'd'
                                                                           : (event.key.keysym.sym == SDLK_LEFT)   ? 'l'
                                                                                                                   : 'r';
                    send_thrust_message(fd, client_id, direction, true, password);
                    receive_response(fd, message);
                    if (strcmp(message, "OK") == 0)
                        render_client_frame(renderer, &background_color, direction);
                    break;
                }
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_UP:
                case SDLK_DOWN:
                case SDLK_LEFT:
                case SDLK_RIGHT:
                {
                    char direction = (event.key.keysym.sym == SDLK_UP) ? 'u' : (event.key.keysym.sym == SDLK_DOWN) ? 'd'
                                                                           : (event.key.keysym.sym == SDLK_LEFT)   ? 'l'
                                                                                                                   : 'r';
                    send_thrust_message(fd, client_id, direction, false, password);
                    receive_response(fd, message);
                    break;
                }
                }
                break;
            }
        }

        // Render frame
        render_client_frame(renderer, &background_color, ' ');

        // Maintain 30 FPS
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay)
            SDL_Delay(frame_delay - frame_time);
    }

    // =========================================================================
    // CLEANUP
    // =========================================================================

    printf("[Client] Shutting down\n");
    destroy_display(window, renderer);
    zmq_close(fd);
    memset(password, 0, MAX_PASSWORD_LEN);

    return EXIT_SUCCESS;
}