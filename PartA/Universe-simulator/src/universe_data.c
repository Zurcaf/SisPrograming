#include "../head/universe_data.h"

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

trash_t* init_trash(int n, int width, int height) {
    trash_t* trash = malloc(n * sizeof(trash_t));
    if (trash == NULL) {
        fprintf(stderr, "Memory allocation failed for trash.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        trash[i].x = rand() % width;
        trash[i].y = rand() % height;
        trash[i].mass = 1;   //1 mass unit
        trash[i].velocity.amplitude = 0;
        trash[i].velocity.angle = 0;
        trash[i].acceleration.amplitude = 0;
        trash[i].acceleration.angle = 0;
    }
    return trash;
}

void addTrash(int n_trash, trash_t* trash, int width, int height) 
{
    trash[n_trash].x = rand() % width;
    trash[n_trash].y = rand() % height;
    trash[n_trash].mass = 1;   //1 mass unit
    trash[n_trash].velocity.amplitude = 0;
    trash[n_trash].velocity.angle = 0;
    trash[n_trash].acceleration.amplitude = 0;
    trash[n_trash].acceleration.angle = 0;
}