#include "display.h"

int update_display(SDL_Window* win, SDL_Renderer* rend, int width, int height)
{
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



    return 0;
}

int update_display(SDL_Renderer* rend, planet_t* planets, int total_planets,
                    trash_t* trash, int total_trash)
{
    for(int i = 0; i < total_planets; i++){
        SDL_SetRenderDrawColor(rend, 0, 0, 255, 255); // Blue for planets
        SDL_Rect planet_rect = { (int)planets[i].x - 5, (int)planets[i].y - 5, 10, 10 };
        SDL_RenderFillRect(rend, &planet_rect);
    }

    SDL_RenderPresent(rend);

    return 0;
}

int destroy_display(SDL_Window* win, SDL_Renderer* rend)
{
    if (rend) SDL_DestroyRenderer(rend);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}


