#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL2/SDL_pixels.h"
#include <stdlib.h>


int main(int argc, char *argv[]){

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
    backgroud_color.g = 255;
    backgroud_color.b = 255;
    backgroud_color.a = 255;
    SDL_SetRenderDrawColor(rend, 
        backgroud_color.r, backgroud_color.g, backgroud_color.b, 
        backgroud_color.a);
    SDL_RenderClear(rend);
    SDL_RenderPresent(rend);

    SDL_Surface* imageSurface = IMG_Load("ist.png");
    // loads image to our graphics hardware memory.
    SDL_Texture* imageTexture = SDL_CreateTextureFromSurface(rend, imageSurface);

    // clears main-memory of original image
    SDL_FreeSurface(imageSurface);

    int close = 0;
    int mouse_down = 0;
    while (!close) {
        SDL_Event event;

        // Events management
        SDL_WaitEvent(&event);
        //SDL_PollEvent(&event);
        
        switch (event.type) {

            case SDL_QUIT: 
                // handling of close button
                close = 1;
                break;
            case SDL_MOUSEBUTTONDOWN: 
                // handling of close button
                printf("mouse down\n");
                mouse_down = 1;
                break;
            case SDL_MOUSEBUTTONUP: 
                // handling of close button
                printf("mouse up\n");
                mouse_down = 0;
                break;
            case SDL_MOUSEMOTION: 
                if (mouse_down) {
                    printf("mouse move %d %d\n", event.motion.x, event.motion.y);
                    SDL_Rect dst;
                    dst.x = event.motion.x;
                    dst.y = event.motion.y;
                    dst.w = 200;
                    dst.h = 200;
                    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
                    SDL_RenderClear(rend);
                    SDL_RenderCopy(rend, imageTexture, NULL, &dst);
                    SDL_RenderPresent(rend);
                }
                break;

        }

    }
    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);
    
    // close SDL
    SDL_Quit();

    return 0;
}