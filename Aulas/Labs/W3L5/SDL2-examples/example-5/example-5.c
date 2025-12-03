#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL2/SDL_pixels.h"
#include <stdlib.h>


int main(int argc, char *argv[])
{

    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }

    SDL_Window* win = SDL_CreateWindow("EXAMPLE 5", // creates a window
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

    SDL_Rect imageDestination;
    // connects our texture with dest to control position and size
    SDL_QueryTexture(imageTexture, NULL, NULL, &imageDestination.w, &imageDestination.h);
    imageDestination.x = 0;
    imageDestination.y = 0;
    //the image will 1/6 the original size
    imageDestination.w /= 3;
    imageDestination.h /= 3;

    int imageAngle = 0;
    int close = 0;
    while (!close) {
        SDL_Event event;

        // Events management
        SDL_WaitEvent(&event);
        switch (event.type) {

            case SDL_QUIT: 
                // handling of close button
                close = 1;
                break;

            case SDL_KEYDOWN:
                // keyboard API for key pressed
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_Q:
                case SDL_SCANCODE_UP:
                    imageAngle = 180;
                    imageDestination.y -= 10;
                    break;
                case SDL_SCANCODE_O:
                case SDL_SCANCODE_LEFT:
                    imageAngle = 90;
                    imageDestination.x -= 10;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_DOWN:
                    imageAngle = 0;
                    imageDestination.y += 10;
                    break;
                case SDL_SCANCODE_P:
                case SDL_SCANCODE_RIGHT:
                    imageAngle = -90;
                    imageDestination.x += 10;
                    break;
                default:
                    break;
                }
            }

        // right boundary
        if (imageDestination.x + imageDestination.w > 1000)
            imageDestination.x = 1000 - imageDestination.w;

        // left boundary
        if (imageDestination.x < 0)
            imageDestination.x = 0;

        // bottom boundary
        if (imageDestination.y + imageDestination.h > 1000)
            imageDestination.y = 1000 - imageDestination.h;

        // upper boundary
        if (imageDestination.y < 0)
            imageDestination.y = 0;

        // clears the screen
        SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
        SDL_RenderClear(rend);

        SDL_RenderCopyEx(rend, imageTexture, NULL, &imageDestination, imageAngle, NULL,  SDL_FLIP_NONE);

        // triggers the double buffers
        // for multiple rendering
        SDL_RenderPresent(rend);
    }


    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);
    
    // close SDL
    SDL_Quit();

    return 0;
}