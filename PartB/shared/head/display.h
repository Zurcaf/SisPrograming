#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>

#include "universe_data.h"
#include "ship_movement.pb-c.h"

int init_display(const char *title, int width, int height, SDL_Window **window, SDL_Renderer **renderer, SDL_Color *background_color);
void render_frame(SDL_Renderer *renderer, SDL_Color *background_color,
                  planet_t *planets, int n_planets,
                  trash_t *trash, int n_trash, ship_t *ship);
void draw_circle(SDL_Renderer *renderer, int cx, int cy, int radius);
// int update_display(SDL_Window* window, SDL_Renderer* renderer, int width, int height);
int destroy_display(SDL_Window *window, SDL_Renderer *renderer);
void render_client_frame(SDL_Renderer *renderer, SDL_Color *background_color, char direction, int width, int height);

// Render from protobuf snapshot (client mirror of server view)
void render_snapshot_frame(SDL_Renderer *renderer, SDL_Color *background_color, const StateSnapshot *state);

#endif
