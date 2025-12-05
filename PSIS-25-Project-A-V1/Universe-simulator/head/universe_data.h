#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

#include <stdio.h>
#include <stdlib.h>

// Define vector structure
typedef struct vector {
    float amplitude;
    float angle;
} vector_t;


typedef struct planet planet_t;
typedef struct trash trash_t;

trash_t* init_trash(int n, int width, int height);
void addTrash(int n_trash, trash_t* trash, int width, int height);
int get_trash_cords(int trash_index, trash_t* trash, float* x, float* y);
int update_trash_cords(int trash_index, trash_t *trash, float added_x, float added_y);
int get_trash_mass(int trash_index, trash_t *trash, int *mass);
int update_trash_acceleration(int trash_index, trash_t *trash, float amplitude, float angle);
int get_trash_acceleration(int trash_index, trash_t *trash, float *amplitude, float *angle);
int get_trash_velocity(int trash_index, trash_t *trash, float *amplitude, float *angle);
void update_trash_velocity(int trash_index, trash_t *trash, float amplitude, float angle);
void correct_position(int trash_index, trash_t *trash, int universe_width, int universe_height);
void friction_in_trash_velocity(int trash_index, trash_t *trash, float friction_coefficient);


planet_t* init_planets(int n, int width, int height);
int get_planet_cords(int planet_index, planet_t* planets, float* x, float* y);
int get_planet_mass(int planet_index, planet_t *planets, int *mass);


#endif
