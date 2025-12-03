#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

#include <stdio.h>
#include <stdlib.h>

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

trash_t* init_trash(int n, int width, int height);

planet_t* init_planets(int n, int width, int height);


#endif
