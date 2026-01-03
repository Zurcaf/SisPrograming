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
                        if (strcmp("CONNECT", message_type) == 0 && ship[index].current_load == -1)
                        {
                            if (index != -1)
                            {
                                ship[index].current_load = 0;
                            }
                        }
                        else
                        {
                            if (strcmp("MOVE", message_type) == 0 && ship[index].current_load != -1)
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
        update_physics(trash, n_trash, planets, n_planets, width, height);

        // Check for collisions and generate new trash if collision detected
        if (check4collisions(trash, &n_trash, planets, n_planets))
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