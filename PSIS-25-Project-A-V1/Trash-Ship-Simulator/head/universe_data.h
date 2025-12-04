#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

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

typedef struct ship{
    float x, y;
    int capacity;
    int current_load;
} ship_t;

trash_t* init_trash(int n, int max, int width, int height);
planet_t* init_planets(int n, int width, int height);
ship_t* init_ship(int capacity);
void handle_data(ship_t* ship, char direction, trash_t* trash, planet_t* planets, int width, int height, int n_trash, int n_planets);
bool contact_made(float src_x, float src_y, float dest_x, float dest_y, int radius);



#endif
