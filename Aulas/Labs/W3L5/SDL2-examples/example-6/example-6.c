#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_keyboard.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]){

    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }

    SDL_Window* win = SDL_CreateWindow("EXAMPLE 6", // creates a window
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

    SDL_TimerID timer_id = 0;

    int n = 0;
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
                    case SDL_SCANCODE_SPACE:
                        if(timer_id == 0){
                            printf("Timer start (5 second period)\n");
                            // start timer
                            timer_id = SDL_AddTimer(5000, 
                                                    (SDL_TimerCallback)timer_callback, NULL);
                        } else {
                            // stop timer
                            printf("Timer stoped\n");
                            SDL_RemoveTimer(timer_id);
                            timer_id = 0;   
                        }
                        break; 
                }
                break;
            case SDL_USEREVENT:
            if (event.user.code == 2){
                printf("USER EVENT received %dth time\n", n++);
            }
            break;   
        }
   }


    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);
    
    // close SDL
    SDL_Quit();

    return 0;
}
