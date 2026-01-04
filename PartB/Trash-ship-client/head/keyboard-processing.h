#ifndef KEYBOARD_PROCESSING_H
#define KEYBOARD_PROCESSING_H

#include <SDL2/SDL.h>
#include "ship_movement.pb-c.h"

/**
 * Convert SDL key event to direction character
 * Returns:
 *   'u' - up
 *   'd' - down
 *   'l' - left
 *   'r' - right
 *   ' ' - no direction (invalid)
 */
char get_direction_from_key(SDL_Keycode key);

/**
 * Process keyboard input and send thrust messages to server
 * Returns 0 on success, -1 on error
 */
int process_keyboard_input(SDL_Event *event, void *zmq_fd, char client_id,
                           const char *password, ServerResponse **out_response);

/**
 * Check if key is a directional key
 * Returns 1 if true, 0 otherwise
 */
int is_directional_key(SDL_Keycode key);

/**
 * Check if key is ESC or quit command
 */
int is_quit_key(SDL_Keycode key);

#endif
