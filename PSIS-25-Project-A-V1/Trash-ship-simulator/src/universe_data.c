#include "../head/universe_data.h"

trash_t* init_trash(int n, int max, int width, int height) {
    trash_t* trash = malloc(max * sizeof(trash_t));
    int cnt=0;
    if (trash == NULL) {
        fprintf(stderr, "Memory allocation failed for trash.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < max; i++) {
        if(cnt < n){ //init trash
            trash[i].mass = 1;   //1 mass unit
            cnt++;
        }else{  //not init trash
            trash[i].mass = -1;   //0 mass unit
        }
        trash[i].x = rand() % width;
        trash[i].y = rand() % height;
        trash[i].velocity.amplitude = 0;
        trash[i].velocity.angle = 0;
        trash[i].acceleration.amplitude = 0;
        trash[i].acceleration.angle = 0;
    }
    return trash;
}

planet_t* init_planets(int n, int width, int height) {
    planet_t* planets = malloc(n * sizeof(planet_t));
    if (planets == NULL) {
        fprintf(stderr, "Memory allocation failed for planets.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        planets[i].x = rand() % width;
        planets[i].y = rand() % height;
        planets[i].mass = 10;   //10 mass units
    }
    return planets;
}

ship_t* init_ship(int capacity) {
    ship_t* ship = malloc(sizeof(ship_t));
    if (ship == NULL) {
        fprintf(stderr, "Memory allocation failed for ship.\n");
        exit(EXIT_FAILURE);
    }
    ship->x = 0;
    ship->y = 0;
    ship->capacity = capacity;
    ship->current_load = 0; // Initialize current load to 0
    return ship;
}

void handle_data(ship_t* ship, char direction, trash_t* trash, planet_t* planets,
     int width, int height, int n_trash, int n_planets) {
    switch (direction) {
        case 'u':
            ship->y-=3;
            correct_position(&ship->y, height);
            break;
        case 'd':
            ship->y+=3;
            correct_position(&ship->y, height);
            break;
        case 'r':
            ship->x+=3;
            correct_position(&ship->x, width);
            break;
        case 'l':
            ship->x-=3;
            correct_position(&ship->x, width);
        break;
    }
    if(ship->current_load < ship->capacity){        //if ship collides with trash
        for (int i = 0; i < n_trash; i++) {
            if (contact_made(ship->x, ship->y, trash[i].x, trash[i].y, 7) && trash[i].mass > 0) {
                trash[i].mass = 0; // Collect the trash
                ship->current_load++;
            }
        }
    }
    
    if(ship->current_load > 0){                 //if ship collides with planet
        for (int i = 0; i < n_planets; i++) {
            if (contact_made(ship->x, ship->y, planets[i].x, planets[i].y, 14) ) {
                ship->current_load = 0;
                for(int i = 0; i < n_trash; i++) {
                    if (trash[i].mass == 0) {
                        trash[i].mass = 1; // Reset the trash mass
                    }
                }
            }
        }
    }
    return;
}


bool contact_made(float src_x, float src_y, float dest_x, float dest_y, int radius) {
    float distance_x = src_x - dest_x;
    float distance_y = src_y - dest_y;
    float distance = sqrt(distance_x * distance_x + distance_y * distance_y);
    return distance < radius; // Assuming contact is made if distance is less than 1 unit
}

void correct_position(float *position, int edge_size){
    if (*position < 0){
        *position = edge_size; //universe width/height
    }
    if (*position > edge_size){
        *position = 0;
    }
    
}
