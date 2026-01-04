#include "../head/display.h"

int init_display(const char *title, int width, int height,
                 SDL_Window **window, SDL_Renderer **renderer,
                 SDL_Color *background_color)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *win = SDL_CreateWindow(title,
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       width, height,
                                       SDL_WINDOW_SHOWN);
    if (!win)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *rend = SDL_CreateRenderer(win, -1,
                                            SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);
    if (!rend)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return -1;
    }

    if (background_color)
    {
        background_color->r = 255;
        background_color->g = 255;
        background_color->b = 255;
        background_color->a = 255;
    }

    *window = win;
    *renderer = rend;
    SDL_SetRenderDrawColor(rend,
                           background_color->r,
                           background_color->g,
                           background_color->b,
                           background_color->a);
    SDL_RenderClear(rend);
    SDL_RenderPresent(rend);

    return 0;
}

void render_frame(SDL_Renderer *renderer, SDL_Color *background_color,
                  planet_t *planets, int n_planets,
                  trash_t *trash, int n_trash, ship_t *ship)
{
    // limpar fundo
    SDL_SetRenderDrawColor(renderer,
                           background_color->r, background_color->g, background_color->b,
                           background_color->a);
    SDL_RenderClear(renderer);

    // ---- desenhar planetas ----
    for (int i = 0; i < n_planets; i++)
    {
        if (planet_is_recycling_at(planets, i))
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // green for recycling planet
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black for normal planets
        }
        draw_circle(renderer, (int)planet_get_x_at(planets, i), (int)planet_get_y_at(planets, i), 20);
    }

    // ---- desenhar lixo espacial ----
    SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255); // vermelho escuro
    for (int i = 0; i < n_trash; i++)
    {
        if (trash_get_mass_at(trash, i) > 0)
        {
            draw_circle(renderer, (int)trash_get_x_at(trash, i), (int)trash_get_y_at(trash, i), 4);
        }
    }

    if (ship != NULL)
    {
        for (int i = 0; i < 52; i++)
        {
            if (ship_get_load_at(ship, i) >= 0)
            {                                                     // only draw connected ships
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blue for ship
                draw_circle(renderer, (int)ship_get_x_at(ship, i), (int)ship_get_y_at(ship, i), 14);
            }
        }
    }

    // apresentar frame
    SDL_RenderPresent(renderer);
}

void draw_circle(SDL_Renderer *renderer, int cx, int cy, int radius)
{
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= radius * radius)
            {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void render_client_frame(SDL_Renderer *renderer, SDL_Color *background_color, char direction, int width, int height)
{
    // force black background white
    SDL_SetRenderDrawColor(renderer, background_color->r, background_color->g, background_color->b, background_color->a);
    SDL_RenderClear(renderer);

    // arrow black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Center arrow based on window size
    const int cx = width > 0 ? width / 2 : 200;
    const int cy = height > 0 ? height / 2 : 200;
    const int size = 60; // overall arrow size
    const int half = size / 2;
    const int head = size / 3; // head size

    switch (direction)
    {
    case 'u':
    {
        // shaft
        SDL_RenderDrawLine(renderer, cx, cy + half, cx, cy - half);
        // head (two diagonals)
        SDL_RenderDrawLine(renderer, cx, cy - half,
                           cx - head, cy - half + head);
        SDL_RenderDrawLine(renderer, cx, cy - half,
                           cx + head, cy - half + head);
        break;
    }
    case 'd':
    {
        SDL_RenderDrawLine(renderer, cx, cy - half, cx, cy + half);
        SDL_RenderDrawLine(renderer, cx, cy + half,
                           cx - head, cy + half - head);
        SDL_RenderDrawLine(renderer, cx, cy + half,
                           cx + head, cy + half - head);
        break;
    }
    case 'l':
    {
        SDL_RenderDrawLine(renderer, cx + half, cy, cx - half, cy);
        SDL_RenderDrawLine(renderer, cx - half, cy,
                           cx - half + head, cy - head);
        SDL_RenderDrawLine(renderer, cx - half, cy,
                           cx - half + head, cy + head);
        break;
    }
    case 'r':
    {
        SDL_RenderDrawLine(renderer, cx - half, cy, cx + half, cy);
        SDL_RenderDrawLine(renderer, cx + half, cy,
                           cx + half - head, cy - head);
        SDL_RenderDrawLine(renderer, cx + half, cy,
                           cx + half - head, cy + head);
        break;
    }
    default:
        // unknown direction: draw nothing (or you can draw a placeholder)
        break;
    }

    SDL_RenderPresent(renderer);
}

// Render full snapshot sent from server (client-side mirror)
void render_snapshot_frame(SDL_Renderer *renderer, SDL_Color *background_color, const StateSnapshot *state)
{
    if (!renderer || !state)
        return;

    SDL_SetRenderDrawColor(renderer,
                           background_color->r, background_color->g, background_color->b,
                           background_color->a);
    SDL_RenderClear(renderer);

    // Planets
    for (size_t i = 0; i < state->n_planets; i++)
    {
        PlanetState *p = state->planets[i];
        if (!p)
            continue;

        if (p->is_recycling)
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        else
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        draw_circle(renderer, (int)p->x, (int)p->y, 20);
    }

    // Trash
    SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255);
    for (size_t i = 0; i < state->n_trash; i++)
    {
        TrashState *t = state->trash[i];
        if (!t || t->mass <= 0)
            continue;
        draw_circle(renderer, (int)t->x, (int)t->y, 4);
    }

    // Ships
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for (size_t i = 0; i < state->n_ships; i++)
    {
        ShipState *s = state->ships[i];
        if (!s || !s->connected)
            continue;
        draw_circle(renderer, (int)s->x, (int)s->y, 14);
    }

    SDL_RenderPresent(renderer);
}

int destroy_display(SDL_Window *win, SDL_Renderer *rend)
{
    if (rend)
        SDL_DestroyRenderer(rend);
    if (win)
        SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
