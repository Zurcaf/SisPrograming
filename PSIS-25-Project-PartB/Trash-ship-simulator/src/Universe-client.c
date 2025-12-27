#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#include "../libconfig/config.h"
#include "../head/universe_data.h"
#include "../head/display.h"
#include "../head/Communication.h"


int main() {
    
    void * fd = create_client_channel("localhost");

    char ch = '\0';
    char message[100];

    while(1){
    while(!isalpha(ch)){
        printf("what is your character(a..z)?: ");
        ch = getchar();
        ch = tolower(ch);
    }
    send_connection_message(fd, ch);
    receive_response (fd, message);
    if (strcmp(message, "NOT OK") ==0){
        continue;
    }else{break;}
    }
    printf("Connected successfully as '%c'\n", ch);
    SDL_Color background_color_c;
    SDL_Window *win_c = NULL;
    SDL_Renderer *rend_c = NULL;
    
    char buffer[256];
    strcpy(buffer, "Universe Client");
    strcat(buffer, " ");
    strncat(buffer, &ch, 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination

    if (init_display(buffer, 400, 400, &win_c, &rend_c, &background_color_c) != 0)
    {
        printf("Failed to initialize display.\n");
        exit(1);
    }
    

    int running = 1;
    SDL_Event event;

    while (running)
    {
        // Events management
        SDL_WaitEvent(&event);

        switch (event.type)
        {

        case SDL_QUIT:
            running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
                case SDLK_UP:
                    send_movement_message(fd, ch, UP);
                    receive_response (fd, message);
                    if(strcmp(message, "OK") ==0){
                        render_client_frame(rend_c, &background_color_c, UP);
                    }
                    break;

                case SDLK_DOWN:
                    send_movement_message(fd, ch, DOWN);
                    receive_response (fd, message);
                    if(strcmp(message, "OK") ==0){
                        render_client_frame(rend_c, &background_color_c, DOWN);
                    }
                    break;

                case SDLK_LEFT:
                    send_movement_message(fd, ch, LEFT);
                    receive_response (fd, message);
                    if(strcmp(message, "OK") ==0){
                        render_client_frame(rend_c, &background_color_c, LEFT);
                    }
                    break;

                case SDLK_RIGHT:
                    send_movement_message(fd, ch, RIGHT);
                    receive_response (fd, message);
                    if(strcmp(message, "OK") ==0){
                        render_client_frame(rend_c, &background_color_c, RIGHT);
                    }
                    break;
            }
            break;
        }
    }
    destroy_display(win_c, rend_c);
    zmq_close(fd);

    return EXIT_SUCCESS;
}