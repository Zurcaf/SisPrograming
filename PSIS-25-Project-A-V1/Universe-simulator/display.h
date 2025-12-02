#ifndef DISPLAY_H
#define DISPLAY_H

#include "universe_data.h"
#include <SDL2/SDL.h>


int init_display(const char *title, int width, int height, SDL_Window* win, SDL_Renderer* rend);
int update_display(SDL_Renderer* rend);
int destroy_display(SDL_Window* window, SDL_Renderer* renderer);

#endif
