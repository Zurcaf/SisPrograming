#ifndef PHYSICS_RULES_H
#define PHYSICS_RULES_H

#include <math.h>
#include <stdbool.h>

#include "universe_data.h"

/**
 * Update all physics for trash objects
 * Applies gravitational forces from planets, updates velocities and positions
 */
void update_physics(trash_t *trash, int total_trash,
                    planet_t *planets, int total_planets,
                    int universe_width, int universe_height);

/**
 * Calculate new acceleration for all trash based on planet gravity
 */
void new_trash_acceleration(planet_t *planets, int total_planets,
                            trash_t *trash, int total_trash);

/**
 * Update velocity with acceleration and apply friction
 */
void new_trash_velocity(trash_t *trash, int total_trash);

/**
 * Update position based on velocity
 */
void new_trash_position(trash_t *trash, int total_trash,
                        int universe_width, int universe_height);

/**
 * Wrap position around universe edges
 */
void correct_position(float *position, int edge_size);

/**
 * Check for collisions between trash and planets
 */
bool check4collisions(trash_t *trash, int *n_trash,
                      planet_t *planets, int n_planets);

/**
 * Create vector from x,y components
 */
vector_t make_vector(float x, float y);

/**
 * Add two vectors
 */
vector_t add_vectors(vector_t v1, vector_t v2);

#endif
