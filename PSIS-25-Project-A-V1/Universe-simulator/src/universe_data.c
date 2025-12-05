#include "../head/universe_data.h"

typedef struct planet
{
    float x, y;
    float mass;
} planet_t;

typedef struct trash
{
    float x, y;
    vector_t velocity;
    vector_t acceleration;
    float mass;
} trash_t;

trash_t *init_trash(int n, int width, int height)
{
    trash_t *trash = malloc(n * sizeof(trash_t));
    if (trash == NULL)
    {
        fprintf(stderr, "Memory allocation failed for trash.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++)
    {
        trash[i].x = rand() % width;
        trash[i].y = rand() % height;
        trash[i].mass = 1; // 1 mass unit
        trash[i].velocity.amplitude = 0;
        trash[i].velocity.angle = 0;
        trash[i].acceleration.amplitude = 0;
        trash[i].acceleration.angle = 0;
    }
    return trash;
}

void addTrash(int n_trash, trash_t *trash, int width, int height)
{
    trash[n_trash].x = rand() % width;
    trash[n_trash].y = rand() % height;
    trash[n_trash].mass = 1; // 1 mass unit
    trash[n_trash].velocity.amplitude = 0;
    trash[n_trash].velocity.angle = 0;
    trash[n_trash].acceleration.amplitude = 0;
    trash[n_trash].acceleration.angle = 0;
}

int get_trash_cords(int trash_index, trash_t *trash, float *x, float *y)
{
    if (trash == NULL || x == NULL || y == NULL)
    {
        return -1; // Error: null pointer
    }
    *x = trash[trash_index].x;
    *y = trash[trash_index].y;
    return 0; // Success
}

int update_trash_cords(int trash_index, trash_t *trash, float added_x, float added_y)
{
    if (trash == NULL)
    {
        return -1; // Error: null pointer
    }
    trash[trash_index].x += added_x;
    trash[trash_index].y += added_y;
    return 0; // Success
}

int get_trash_mass(int trash_index, trash_t *trash, int *mass)
{
    if (trash == NULL || mass == NULL)
    {
        return -1; // Error: null pointer
    }
    *mass = trash[trash_index].mass;
    return 0; // Success
}

int update_trash_acceleration(int trash_index, trash_t *trash, float amplitude, float angle)
{
    if (trash == NULL)
    {
        return -1; // Error: null pointer
    }
    trash[trash_index].acceleration.amplitude = amplitude;
    trash[trash_index].acceleration.angle = angle;
    return 0; // Success
}

int get_trash_acceleration(int trash_index, trash_t *trash, float *amplitude, float *angle)
{
    if (trash == NULL || amplitude == NULL || angle == NULL)
    {
        return -1; // Error: null pointer
    }
    *amplitude = trash[trash_index].acceleration.amplitude;
    *angle = trash[trash_index].acceleration.angle;
    return 0; // Success
}

int get_trash_velocity(int trash_index, trash_t *trash, float *amplitude, float *angle)
{
    if (trash == NULL || amplitude == NULL || angle == NULL)
    {
        return -1; // Error: null pointer
    }
    *amplitude = trash[trash_index].velocity.amplitude;
    *angle = trash[trash_index].velocity.angle;
    return 0; // Success
}

void update_trash_velocity(int trash_index, trash_t *trash, float amplitude, float angle)
{
    if (trash == NULL)
    {
        return; // Error: null pointer
    }
    trash[trash_index].velocity.amplitude = amplitude;
    trash[trash_index].velocity.angle = angle;
}

void correct_position(int trash_index, trash_t *trash, int universe_width, int universe_height)
{
    if (trash == NULL)
    {
        return; // Error: null pointer
    }
    if (trash[trash_index].x < 0)
        trash[trash_index].x += universe_width;
    else if (trash[trash_index].x >= universe_width)
        trash[trash_index].x -= universe_width;

    if (trash[trash_index].y < 0)
        trash[trash_index].y += universe_height;
    else if (trash[trash_index].y >= universe_height)
        trash[trash_index].y -= universe_height;
}

void friction_in_trash_velocity(int trash_index, trash_t *trash, float friction_coefficient)
{
    if (trash == NULL)
    {
        return; // Error: null pointer
    }
    trash[trash_index].velocity.amplitude *= friction_coefficient;
}

planet_t *init_planets(int n, int width, int height)
{
    planet_t *planets = malloc(n * sizeof(planet_t));
    if (planets == NULL)
    {
        fprintf(stderr, "Memory allocation failed for planets.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++)
    {
        planets[i].x = rand() % width;
        planets[i].y = rand() % height;
        planets[i].mass = 10; // 10 mass units
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

int get_planet_mass(int planet_index, planet_t *planets, int *mass)
{
    if (planets == NULL || mass == NULL)
    {
        return -1; // Error: null pointer
    }
    *mass = planets[planet_index].mass;
    return 0; // Success
}
