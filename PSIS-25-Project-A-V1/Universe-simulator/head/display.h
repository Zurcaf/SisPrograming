#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>

#include "../head/universe_data.h"

int init_display(const char *title, int width, int height, SDL_Window **window, SDL_Renderer **renderer, SDL_Color* background_color);
void render_frame(SDL_Renderer *renderer, SDL_Color *background_color,
                  planet_t *planets, int n_planets,
                  trash_t *trash, int n_trash);
void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius);                  
// int update_display(SDL_Window* window, SDL_Renderer* renderer, int width, int height);
int destroy_display(SDL_Window* window, SDL_Renderer* renderer);

#endif
