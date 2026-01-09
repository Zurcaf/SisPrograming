#include "cursor_processing.h"
#include "../head/Communication.h"
#include "../shared/head/validation.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

char get_direction_from_key(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_UP:
        return 'u';
    case SDLK_DOWN:
        return 'd';
    case SDLK_LEFT:
        return 'l';
    case SDLK_RIGHT:
        return 'r';
    default:
        return ' ';
    }
}

int is_directional_key(SDL_Keycode key)
{
    return (key == SDLK_UP || key == SDLK_DOWN ||
            key == SDLK_LEFT || key == SDLK_RIGHT);
}

int is_quit_key(SDL_Keycode key)
{
    return (key == SDLK_ESCAPE);
}

int process_keyboard_input(SDL_Event *event, void *zmq_fd, char client_id,
                           const char *password, ServerResponse **out_response)
{
    if (out_response)
    {
        *out_response = NULL;
    }

    if (event == NULL || zmq_fd == NULL || password == NULL || out_response == NULL)
    {
        fprintf(stderr, "[Keyboard] Invalid input parameters\n");
        return -1;
    }

    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP)
    {
        return 0;
    }

    if (event->key.repeat)
    {
        return 0;
    }

    SDL_Keycode key = event->key.keysym.sym;

    if (is_quit_key(key))
    {
        return -1;
    }

    if (is_directional_key(key))
    {
        char direction = get_direction_from_key(key);
        bool thrust_active = (event->type == SDL_KEYDOWN);

        if (direction == ' ')
        {
            fprintf(stderr, "[Keyboard] Invalid direction conversion\n");
            return -1;
        }

        send_thrust_message(zmq_fd, client_id, direction, thrust_active, password);

        ServerResponse *resp = receive_response_full(zmq_fd);
        if (resp == NULL)
        {
            fprintf(stderr, "[Keyboard] Failed to receive server response\n");
            return -1;
        }

        *out_response = resp;

        if (thrust_active)
        {
            printf("[Keyboard] Thrust %c activated\n", direction);
        }
        else
        {
            printf("[Keyboard] Thrust %c deactivated\n", direction);
        }

        return 0;
    }

    return 0;
}