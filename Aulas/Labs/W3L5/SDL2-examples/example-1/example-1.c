#include <SDL2/SDL.h>

SDL_Color next_color(SDL_Color current_color) {
    SDL_Color new_color;

    if (current_color.r == 255 && current_color.g == 0) {
        current_color.b -= 1;
        if (current_color.b == 0) {
            current_color.r = 255;
            current_color.g = 0;
            current_color.b = 255;
        }
    }
    if (current_color.b == 255 && current_color.g == 0) {
        current_color.r -= 1;
        if (current_color.r == 0) {
            current_color.r = 0;
            current_color.g = 255;
            current_color.b = 255;
        }
    }
    if (current_color.g == 255 && current_color.r == 0) {
        current_color.b -= 1;
        if (current_color.b == 0) {
            current_color.r = 255;
            current_color.g = 0;
            current_color.b = 255;
        }
    }
    return current_color;
}

int main(int argc, char *argv[])
{

    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* win = SDL_CreateWindow("EXAMPLE 1", // creates a window
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       1000, 1000, 0);

    // triggers the program that controls
    // your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;

    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);

    SDL_Color backgroud_color;
    backgroud_color.r = 255;
    backgroud_color.g = 0;
    backgroud_color.b = 255;
    backgroud_color.a = 255;

    while(1) {
        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type== SDL_QUIT){
            // handling of close button
            break;
        }

        SDL_SetRenderDrawColor(rend, 
            backgroud_color.r, backgroud_color.g, backgroud_color.b, 
            backgroud_color.a);
        SDL_RenderClear(rend);

        SDL_RenderPresent(rend);
        backgroud_color = next_color(backgroud_color);
        SDL_Delay(10);
    }
    // clears the screen

    // destroy renderer
    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);
    
    // close SDL
    SDL_Quit();

    return 0;
}