#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

#include <stdio.h>
#include <stdlib.h>

// Define vector structure

typedef struct vector vector_t;
typedef struct planet planet_t;
typedef struct trash trash_t;

trash_t* init_trash(int n, int width, int height);
void addTrash(int n_trash, trash_t* trash, int width, int height);
int get_trash_cords(int trash_index, trash_t* trash, float* x, float* y);

planet_t* init_planets(int n, int width, int height);
int get_planet_cords(int planet_index, planet_t* planets, float* x, float* y);

#endif
