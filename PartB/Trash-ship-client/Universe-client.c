#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include "../libconfig/config.h"
#include "../head/universe_data.h"
#include "../head/display.h"
#include "../head/Communication.h"

int main()
{

    void *fd = create_client_channel("localhost");

    char ch = '\0';
    char message[100];

    while (1)
    {
        while (!isalpha(ch))
        {
            printf("what is your character(a..z)?: ");
            ch = getchar();
            ch = tolower(ch);
        }
        send_connection_message(fd, ch);
        receive_response(fd, message);
        if (strcmp(message, "NOT OK") == 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    printf("Connected successfully as '%c'\n", ch);
    SDL_Color background_color_c;
    SDL_Window *win_c = NULL;
    SDL_Renderer *rend_c = NULL;

    char buffer[256];
    strcpy(buffer, "Universe Client");
    strcat(buffer, " ");
    strncat(buffer, &ch, 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination

    if (init_display(buffer, 400, 400, &win_c, &rend_c, &background_color_c) != 0)
    {
        printf("Failed to initialize display.\n");
        exit(1);
    }

    int running = 1;
    SDL_Event event;

    // Enable key repeat for continuous movement
    const Uint8 *keystate;

    // Timer for 30 FPS
    Uint32 frame_start = 0;
    const Uint32 frame_delay = 33; // milliseconds (1000/30 ≈ 33ms)

    while (running)
    {
        frame_start = SDL_GetTicks();

        // Process all pending events (non-blocking)
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = 0;
                    break;
                }
                break;
            }
        }

        // Get current keyboard state for continuous movement
        keystate = SDL_GetKeyboardState(NULL);

        // Send movement based on held keys (not just on keydown)
        if (keystate[SDL_SCANCODE_UP])
        {
            send_movement_message(fd, ch, 'u');
            receive_response(fd, message);
            if (strcmp(message, "OK") == 0)
            {
                render_client_frame(rend_c, &background_color_c, 'u');
            }
        }
        else if (keystate[SDL_SCANCODE_DOWN])
        {
            send_movement_message(fd, ch, 'd');
            receive_response(fd, message);
            if (strcmp(message, "OK") == 0)
            {
                render_client_frame(rend_c, &background_color_c, 'd');
            }
        }
        else if (keystate[SDL_SCANCODE_LEFT])
        {
            send_movement_message(fd, ch, 'l');
            receive_response(fd, message);
            if (strcmp(message, "OK") == 0)
            {
                render_client_frame(rend_c, &background_color_c, 'l');
            }
        }
        else if (keystate[SDL_SCANCODE_RIGHT])
        {
            send_movement_message(fd, ch, 'r');
            receive_response(fd, message);
            if (strcmp(message, "OK") == 0)
            {
                render_client_frame(rend_c, &background_color_c, 'r');
            }
        }

        // Frame rate limiting (maintain 30 FPS)
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay)
        {
            SDL_Delay(frame_delay - frame_time);
        }
    }
    destroy_display(win_c, rend_c);
    zmq_close(fd);

    return EXIT_SUCCESS;
}