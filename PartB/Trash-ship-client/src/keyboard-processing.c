#include "keyboard-processing.h"
#include "../head/Communication.h"
#include <stdio.h>
#include <string.h>

/**
 * Convert SDL key event to direction character
 * Returns:
 *   'u' - up
 *   'd' - down
 *   'l' - left
 *   'r' - right
 *   ' ' - no direction (invalid)
 */
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
        return ' '; // Invalid direction
    }
}

/**
 * Check if key is a directional key
 * Returns 1 if true, 0 otherwise
 */
int is_directional_key(SDL_Keycode key)
{
    return (key == SDLK_UP || key == SDLK_DOWN ||
            key == SDLK_LEFT || key == SDLK_RIGHT);
}

/**
 * Check if key is ESC or quit command
 */
int is_quit_key(SDL_Keycode key)
{
    return (key == SDLK_ESCAPE);
}

/**
 * Process keyboard input and send thrust messages to server
 * Handles:
 * - Direction keys: UP, DOWN, LEFT, RIGHT
 * - ESC to quit
 * Returns 0 on success, -1 on error
 */
int process_keyboard_input(SDL_Event *event, void *zmq_fd, char client_id,
                           const char *password, char *response_buffer)
{
    // Validate inputs
    if (event == NULL || zmq_fd == NULL || password == NULL || response_buffer == NULL)
    {
        fprintf(stderr, "[Keyboard] Invalid input parameters\n");
        return -1;
    }

    // Only process key events
    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP)
    {
        return 0; // Not a keyboard event, but valid
    }

    // Ignore key repeats from OS
    if (event->key.repeat)
    {
        return 0;
    }

    SDL_Keycode key = event->key.keysym.sym;

    // Check for quit command
    if (is_quit_key(key))
    {
        return -1; // Signal to quit
    }

    // Check for directional input
    if (is_directional_key(key))
    {
        char direction = get_direction_from_key(key);
        bool thrust_active = (event->type == SDL_KEYDOWN);

        if (direction == ' ')
        {
            fprintf(stderr, "[Keyboard] Invalid direction conversion\n");
            return -1;
        }

        // Send thrust message to server
        send_thrust_message(zmq_fd, client_id, direction, thrust_active, password);

        // Receive server response
        receive_response(zmq_fd, response_buffer);

        // Log the input
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

    // Unhandled key - not an error, just ignore it
    return 0;
}
