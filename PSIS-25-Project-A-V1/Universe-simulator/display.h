#ifndef DISPLAY_H
#define DISPLAY_H

#include "universe_data.h"
#include <SDL2/SDL.h>


int update_display(SDL_Window* window, SDL_Renderer* renderer, int width, int height);
int destroy_display(SDL_Window* window, SDL_Renderer* renderer);


#endif
