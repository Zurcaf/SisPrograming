#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>

#include "config.h"
#include "universe_data.h"
#include "display.h"

Uint32 timer_callback(Uint32 interval, void* param){
    SDL_Event timer_event;
    printf("Timer callback function\n");

    SDL_zero(timer_event);  /* SDL will copy this entire struct! Initialize to keep memory checkers happy. */
    timer_event.type = SDL_USEREVENT;
    timer_event.user.code = 2;
    timer_event.user.data1 = NULL;
    timer_event.user.data2 = NULL;
    SDL_PushEvent(&timer_event);
    return interval; // to continue the timer
} 

int main() {
    if(load_config("../libconfig/init.conf") != 0) {
        printf("Failed to load configuration file.\n");
        exit(1);
    }

    int width = get_width_universe_int();
    int height = get_height_universe_int();
    int max_trash = get_max_n_trash_int();
    int n_trash = get_init_n_trash_int();
    int n_planets = get_n_planets();
    bool running = 1;

    trash_t *trash = init_trash(n_trash, width, height);
    planet_t *planets = init_planets(n_planets, width, height);

    init_display("Universe Simulator", width, height);

    
    SDL_TimerID timer_id = 0;    
    timer_id = SDL_AddTimer(10, 
                (SDL_TimerCallback)timer_callback, NULL);
    
    
    while (running) {
        SDL_Event event;
        

        // Events management
        SDL_WaitEvent(&event);

            
        switch (event.type) {

            case SDL_QUIT: 
                running = 0;
                break;   


            case SDL_USEREVENT:
                if (event.user.code == 2){
                //update_physics(trash, n_trash, planets, n_planets, width, height);
                
            }
            break;   
        }
   }
    destroy_display();
    return 0;
}
