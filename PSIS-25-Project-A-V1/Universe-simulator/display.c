#include "display.h"

static SDL_Window* win = NULL;
static SDL_Renderer* rend = NULL;

int init_display(const char *title, int width, int height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED,
                           width, height, 0);

    if (!win) return -1;

    rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!rend) return -1;

    return 0;
}

int destroy_display()
{
    if (rend) SDL_DestroyRenderer(rend);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}


