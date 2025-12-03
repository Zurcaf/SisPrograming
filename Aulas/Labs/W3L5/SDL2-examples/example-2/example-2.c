#include <SDL2/SDL.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"
#include <stdlib.h>

SDL_Color random_color(){
    SDL_Color color;
    color.r = rand() % 256;
    color.g = rand() % 256;
    color.b = rand() % 256;
    color.a = rand() % 255;
    return color;
}

Uint32 SDL_ColorToUint(SDL_Color c){
	return (Uint32)((c.a << 24) + (c.b << 16) + (c.g << 8)+ (c.r << 0));
}

int main(int argc, char *argv[])
{

    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* win = SDL_CreateWindow("EXAMPLE 2", // creates a window
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

    SDL_Color object_color;
    int counter = 0;
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

        object_color = random_color();
        int x = rand() % 1000; 
        int y = rand() % 1000;;
        int heigth = rand() % 300;
        int witdth = rand() % 300;

        switch (rand()%6)
        {
        case 0:
            filledCircleColor(rend, x, y, heigth, 
                                SDL_ColorToUint(object_color));
            break;
        case 1:
            circleColor(rend, x, y, heigth, 
                        SDL_ColorToUint(object_color));
            break;
        case 2:
            boxColor(rend, x, y, x+witdth, y+heigth, 
                        SDL_ColorToUint(object_color));
            break;
        case 3: 
            rectangleColor(rend, x, y, x+witdth, y+heigth, 
                            SDL_ColorToUint(object_color));
            break;
        case 4:
                ellipseColor(rend, x, y, witdth, heigth, 
                            SDL_ColorToUint(object_color));
            break;
        case 5:
                filledEllipseColor(rend, x, y, witdth, heigth, 
                                    SDL_ColorToUint(object_color));
            break;
        default:
            break;
        }
        SDL_RenderPresent(rend);
        SDL_Delay(1000);
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
