#ifndef CURSOR_PROCESSING_H
#define CURSOR_PROCESSING_H

#include <SDL2/SDL.h>
#include "ship_movement.pb-c.h"

char get_direction_from_key(SDL_Keycode key);

int process_keyboard_input(SDL_Event *event, void *zmq_fd, char client_id,
                           const char *password, ServerResponse **out_response);

int is_directional_key(SDL_Keycode key);

int is_quit_key(SDL_Keycode key);

#endif