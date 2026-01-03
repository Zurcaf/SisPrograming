#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>

#include <config.h>
#include "../head/universe_data.h"
#include "../head/display.h"
#include "../head/Communication.h"
#include "../head/physics-rules.h"

Uint32 timer_callback(Uint32 interval, void *param)
{
    SDL_Event timer_event;

    (void)param; // explicitly marked as unused

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

    load_config("../libconfig/init.conf");
    int width = get_width_universe_int();
    int height = get_height_universe_int();
    int n_trash = get_init_n_trash_int();
    int n_planets = get_n_planets_int();
    int max_n_trash = get_max_n_trash_int();
    int capacity_ship = get_capacity_ship_int();

    void *fd = create_server_channel();

    SDL_Window *win = NULL;
    SDL_Renderer *rend = NULL;
    SDL_Color background_color = {0, 0, 0, 255}; // Black background

    if (init_display("Universe Simulator", width, height, &win, &rend, &background_color) != 0)
    {
        return EXIT_FAILURE;
    }

    planet_t *planets = init_planets(n_planets, width, height);
    trash_t *trash = init_trash(n_trash, max_n_trash, width, height);
    ship_t *ship = init_ship(capacity_ship);

    Uint32 last_trash_spawn_ms = SDL_GetTicks();
    Uint32 last_recycle_ms = SDL_GetTicks();
    const Uint32 trash_spawn_interval_ms = 10000;  // 10s
    const Uint32 recycle_rotate_interval_ms = 30000; // 30s

    bool running = 1;
    char message_type[1024];
    char ship_id;
    char direction;
    SDL_Event event;

    // Timer for 30 FPS (33ms per frame)
    Uint32 frame_start = 0;
    const Uint32 frame_delay = 33; // milliseconds (1000/30 ≈ 33ms)

    SDL_AddTimer(33, (SDL_TimerCallback)timer_callback, NULL);

    while (running)
    {
        frame_start = SDL_GetTicks();

        // Process all pending events (non-blocking)
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;

            case SDL_USEREVENT:
                if (event.user.code == 2)
                {
                    if (read_message(fd, message_type, &ship_id, &direction) != -1)
                    {
                        int index = ship_index(ship_id);
                        if (strcmp("CONNECT", message_type) == 0 && ship_get_load_at(ship, index) == -1)
                        {
                            if (index != -1)
                            {
                                ship_set_load_at(ship, index, 0);
                            }
                        }
                        else
                        {
                            if (strcmp("MOVE", message_type) == 0 && ship_get_load_at(ship, index) != -1)
                            {
                                handle_data(ship, direction, trash, planets, width, height, n_trash, n_planets, index);
                            }
                        }
                        send_response(fd, "OK");
                    }
                }
                break;
            }
        }

        // Update physics every frame
        update_physics(trash, n_trash, planets, n_planets, ship, MAX_SHIPS, width, height);

        // Determine if there is any connected ship in the universe
        bool has_ship = false;
        for (int si = 0; si < MAX_SHIPS; si++)
        {
            if (ship_get_load_at(ship, si) >= 0)
            {
                has_ship = true;
                break;
            }
        }

        // Collision-based trash only if there is at least one ship in the universe
        if (has_ship && check4collisions(trash, &n_trash, planets, n_planets))
        {
            if (n_trash < max_n_trash)
            {
                addTrash(n_trash, trash, width, height);
                n_trash++;
                printf("Collision detected! New trash created. Total trash: %d\n", n_trash);
            }
            else
            {
                printf("Max trash capacity reached!\n");
            }
        }

        Uint32 now_ms = SDL_GetTicks();

        // Periodic trash spawn every 10s if ships exist
        if (has_ship && (now_ms - last_trash_spawn_ms) >= trash_spawn_interval_ms)
        {
            if (n_trash < max_n_trash)
            {
                addTrash(n_trash, trash, width, height);
                n_trash++;
                printf("Periodic trash spawn. Total trash: %d\n", n_trash);
            }
            else
            {
                printf("Periodic spawn skipped: max trash reached.\n");
            }
            last_trash_spawn_ms = now_ms;
        }

        // Rotate recycling planet every 30s
        if ((now_ms - last_recycle_ms) >= recycle_rotate_interval_ms)
        {
            // Find current recycling planet
            int current = -1;
            for (int pi = 0; pi < n_planets; pi++)
            {
                if (planet_get_mass_at(planets, pi) == 0)
                {
                    current = pi;
                    break;
                }
            }
            if (current >= 0)
            {
                int next = (current + 1) % n_planets;
                planet_set_mass_at(planets, current, 10);
                planet_set_mass_at(planets, next, 0);
                printf("Recycling planet rotated to index %d\n", next);
            }
            last_recycle_ms = now_ms;
        }

        // Render every frame
        render_frame(rend, &background_color, planets, n_planets, trash, n_trash, ship);

        // Frame rate limiting (maintain 30 FPS)
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay)
        {
            SDL_Delay(frame_delay - frame_time);
        }
    }
    destroy_display(win, rend);
    free(planets);
    free(trash);
    free(ship);
    zmq_close(fd);
    return 0;
}