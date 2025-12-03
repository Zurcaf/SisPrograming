#ifndef PHYSICS_RULES_H
#define PHYSICS_RULES_H

#include <math.h>
#include "universe_data.h"


void new_trash_acceleration(planet_t *planets, int total_planets, trash_t *trash, int total_trash);
void new_trash_velocity(trash_t *trash, int total_trash);
void new_trash_position( trash_t *trash, int total_trash, int universe_width, int universe_height);
void correct_position(float *position, int edge_size);

vector_t make_vector(float x, float y);
vector_t add_vectors(vector_t v1, vector_t v2);



#endif


