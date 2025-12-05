#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>

#include "../../libconfig/config.h"
#include "../head/universe_data.h"
#include "../head/display.h"
#include "../head/physics-rules.h"

Uint32 timer_callback(Uint32 interval, void *param)
{
    SDL_Event timer_event;

    (void)param;            // explicitly marked as unused

    SDL_zero(timer_event); /* SDL will copy this entire struct! Initialize to keep memory checkers happy. */
    timer_event.type = SDL_USEREVENT;
    timer_event.user.code = 2;
    timer_event.user.data1 = NULL;
    timer_event.user.data2 = NULL;
    SDL_PushEvent(&timer_event);
    return interval; // to continue the timer
}

int main()
{
    if (load_config("../libconfig/init.conf") != 0)
    {
        printf("Failed to load configuration file.\n");
        exit(1);
    }

    int width = get_width_universe_int();
    int height = get_height_universe_int();
    int max_trash = get_max_n_trash_int();
    int n_trash = get_init_n_trash_int();
    int n_planets = get_n_planets_int();

    bool running = true;

    // initialize universe data
    planet_t *planets = init_planets(n_planets, width, height);
    trash_t *trash = init_trash(max_trash, width, height);

    // initialize display
    SDL_Color background_color;
    SDL_Window *win = NULL;
    SDL_Renderer *rend = NULL;

    if (init_display("Universe Simulator", width, height, &win, &rend, &background_color) != 0)
    {
        printf("Failed to initialize display.\n");
        exit(1);
    }

    SDL_AddTimer(10,
                 (SDL_TimerCallback)timer_callback, NULL);

    while (running)
    {
        SDL_Event event;

        // Events management
        SDL_WaitEvent(&event);

        switch (event.type)
        {

        case SDL_QUIT:
            running = false;
            break;

        case SDL_USEREVENT:
            if (event.user.code == 2)
            {
                render_frame(rend, &background_color, planets, n_planets, trash, n_trash);
                if (check4collisions(trash, &n_trash, planets, n_planets))
                {
                    if (n_trash >= max_trash) {
                        running = false;
                        printf("Max trash capacity reached! Ending simulation.\n");
                    }
                    else
                    {
                        addTrash(n_trash, trash, width, height);
                        n_trash++;
                        printf("Collision detected! New trash added. Total trash: %d\n", n_trash);
                    }
                }

                update_physics(trash, n_trash, planets, n_planets, width, height);
            }
            break;
        }
    }

    destroy_display(win, rend);
    return 0;
}
