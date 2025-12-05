#include "../head/universe_data.h"

typedef struct vector {
    float amplitude;
    float angle;
} vector_t;

typedef struct planet {
    float x, y;
    float mass;
} planet_t;

typedef struct trash {
    float x, y;
    vector_t velocity;
    vector_t acceleration;
    float mass;
} trash_t;

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

int get_trash_cords(int trash_index, trash_t* trash, float* x, float* y) {
    if (trash == NULL || x == NULL || y == NULL) {
        return -1; // Error: null pointer
    }
    *x = trash[trash_index].x;
    *y = trash[trash_index].y;
    return 0; // Success
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

int get_planet_cords(int planet_index, planet_t *planets, float *x, float *y)
{
    if (planets == NULL || x == NULL || y == NULL)
    {
        return -1; // Error: null pointer
    }
    *x = planets[planet_index].x;
    *y = planets[planet_index].y;
    return 0; // Success
}