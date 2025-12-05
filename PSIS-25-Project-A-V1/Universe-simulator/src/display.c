#include "../head/display.h"

int init_display(const char *title, int width, int height, SDL_Window** window, SDL_Renderer** renderer, SDL_Color* background_color)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window*  win;
    SDL_Renderer* rend;

    win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED,
                           width, height, 0);
    if (!win) return -1;

    rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!rend) return -1;
    
    // define background color
    // branco
    background_color->r = 255;
    background_color->g = 255;
    background_color->b = 255;
    background_color->a = 255;

    *window = win;
    *renderer = rend;

    return 0;
}


void render_frame(SDL_Renderer* renderer, SDL_Color *background_color,
                  planet_t* planets, int n_planets,
                  trash_t* trash, int n_trash)
{
    // limpar fundo
    SDL_SetRenderDrawColor(renderer, 
        background_color->r, background_color->g, background_color->b, 
        background_color->a);
    SDL_RenderClear(renderer);



    // ---- desenhar planetas ----
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // preto
    for (int i = 0; i < n_planets; i++) {
        float x, y;
        get_planet_cords(i, planets, &x, &y);

        draw_circle(renderer, (int)x, (int)y, 20);
    }

    // ---- desenhar lixo espacial ----
    SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255); // vermelho escuro
    for (int i = 0; i < n_trash; i++) {
        float x, y;
        get_trash_cords(i, trash, &x, &y);
    
        draw_circle(renderer, (int)x, (int)y, 4);
    }

    // apresentar frame
    SDL_RenderPresent(renderer);
}

void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius)
{
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= radius * radius) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}


// int update_display(SDL_Renderer* rend, planet_t* planets, int total_planets,
//                     trash_t* trash, int total_trash)
// {
//     for(int i = 0; i < total_planets; i++){
//         SDL_SetRenderDrawColor(rend, 0, 0, 255, 255); // Blue for planets
//         SDL_Rect planet_rect = { (int)planets[i].x - 5, (int)planets[i].y - 5, 10, 10 };
//         SDL_RenderFillRect(rend, &planet_rect);
//     }

//     SDL_RenderPresent(rend);

//     return 0;
// }

int destroy_display(SDL_Window* win, SDL_Renderer* rend)
{
    if (rend) SDL_DestroyRenderer(rend);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}


