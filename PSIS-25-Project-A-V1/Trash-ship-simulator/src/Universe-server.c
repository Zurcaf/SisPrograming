#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>

#include "config.h"
#include "../head/universe_data.h"
#include "../head/display.h"
#include "../head/Communication.h"

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

int main() {

    load_config("../libconfig/init.conf");
    int width = get_width_universe_int();
    int height = get_height_universe_int();
    int n_trash = get_init_n_trash_int();
    int n_planets = get_n_planets_int();
    int max_n_trash = get_max_n_trash_int();
    int capacity_ship = get_capacity_ship_int();


    void *fd = create_server_channel();




    SDL_Window* win = NULL;
    SDL_Renderer* rend = NULL;
    SDL_Color background_color = {0, 0, 0, 255}; // Black background

    if (init_display("Universe Simulator", width, height, &win, &rend, &background_color) != 0) {
        return EXIT_FAILURE;
    }

    planet_t* planets = init_planets(n_planets, width, height);
    trash_t* trash = init_trash(n_trash, max_n_trash, width, height);
    ship_t* ship = init_ship(capacity_ship);

    bool running = 1;
    char message_type[1024];
    char ship_id;
    char direction;
    SDL_Event event;
    SDL_AddTimer(10,
                 (SDL_TimerCallback)timer_callback, NULL);

    while (running)
    {
        // Events management
        SDL_WaitEvent(&event);

        switch (event.type)
        {

        case SDL_QUIT:
            running = 0;
            break;

        case SDL_USEREVENT:
            if (event.user.code == 2)
            {
                if(read_message(fd, message_type, &ship_id, &direction) != -1){
                    int index = ship_index(ship_id);
                    if(strcmp("CONNECT", message_type) == 0 && ship[index].current_load == -1) {
                        if(index != -1){
                            ship[index].current_load = 0; //set current load to 0 when ship connects
                        }
                    }else{
                        if(strcmp("MOVE", message_type) == 0 && ship[index].current_load != -1) {
                        handle_data(ship, direction, trash, planets, width, height, n_trash, n_planets, index);
                        }
                    }
                send_response (fd, "OK");
                render_frame(rend, &background_color, planets, n_planets, trash, n_trash, ship);
                }
            }
            break;
        }
    }
    destroy_display(win, rend);
    zmq_close(fd);
    return 0;
}