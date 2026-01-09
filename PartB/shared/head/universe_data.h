#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct vector
{
    float amplitude;
    float angle;
} vector_t;

// Opaque types to enforce data abstraction
typedef struct planet planet_t;
typedef struct trash trash_t;
typedef struct ship ship_t;

trash_t *init_trash(int n, int max, int width, int height);
planet_t *init_planets(int n, int width, int height);
// Reuse freed trash slots (mass <= 0) before growing count. Updates n_trash when a new slot is used.
bool addTrash(trash_t *trash, int *n_trash, int max, int width, int height);
ship_t *init_ship(int capacity);
void correct_position(float *position, int edge_size);
int ship_index(char id);

// --- Planet accessors ---
float planet_get_x(const planet_t *p);
float planet_get_y(const planet_t *p);
float planet_get_mass(const planet_t *p);
void planet_set_mass(planet_t *p, float mass);
float planet_get_x_at(const planet_t *list, int idx);
float planet_get_y_at(const planet_t *list, int idx);
float planet_get_mass_at(const planet_t *list, int idx);
void planet_set_mass_at(planet_t *list, int idx, float mass);
bool planet_is_recycling(const planet_t *p);
bool planet_is_recycling_at(const planet_t *list, int idx);
void planet_set_recycling_at(planet_t *list, int idx, bool is_recycling);
int planet_get_recycled_count_at(const planet_t *list, int idx);
void planet_increment_recycled_count_at(planet_t *list, int idx);

// --- Trash accessors ---
float trash_get_x(const trash_t *t);
float trash_get_y(const trash_t *t);
float trash_get_mass(const trash_t *t);
vector_t trash_get_velocity(const trash_t *t);
vector_t trash_get_acceleration(const trash_t *t);
void trash_set_position(trash_t *t, float x, float y);
void trash_set_mass(trash_t *t, float mass);
void trash_set_velocity(trash_t *t, vector_t v);
void trash_set_acceleration(trash_t *t, vector_t a);
float trash_get_x_at(const trash_t *list, int idx);
float trash_get_y_at(const trash_t *list, int idx);
float trash_get_mass_at(const trash_t *list, int idx);
vector_t trash_get_velocity_at(const trash_t *list, int idx);
vector_t trash_get_acceleration_at(const trash_t *list, int idx);
void trash_set_position_at(trash_t *list, int idx, float x, float y);
void trash_set_mass_at(trash_t *list, int idx, float mass);
void trash_set_velocity_at(trash_t *list, int idx, vector_t v);
void trash_set_acceleration_at(trash_t *list, int idx, vector_t a);

// --- Ship accessors ---
float ship_get_x(const ship_t *s);
float ship_get_y(const ship_t *s);
int ship_get_capacity(const ship_t *s);
int ship_get_load(const ship_t *s);
void ship_set_position(ship_t *s, float x, float y);
void ship_set_load(ship_t *s, int load);
void ship_increment_load(ship_t *s);
void ship_reset_load(ship_t *s);
float ship_get_x_at(const ship_t *list, int idx);
float ship_get_y_at(const ship_t *list, int idx);
int ship_get_capacity_at(const ship_t *list, int idx);
int ship_get_load_at(const ship_t *list, int idx);
void ship_set_position_at(ship_t *list, int idx, float x, float y);
void ship_set_load_at(ship_t *list, int idx, int load);
void ship_increment_load_at(ship_t *list, int idx);
void ship_reset_load_at(ship_t *list, int idx);
vector_t ship_get_velocity_at(const ship_t *list, int idx);
vector_t ship_get_acceleration_at(const ship_t *list, int idx);
void ship_set_velocity_at(ship_t *list, int idx, vector_t v);
void ship_set_acceleration_at(ship_t *list, int idx, vector_t a);
vector_t ship_get_thrust_at(const ship_t *list, int idx);
void ship_set_thrust_at(ship_t *list, int idx, vector_t v);

// Apply thrust direction (up/down/left/right) with a fixed magnitude; active=false clears thrust
void apply_thrust(ship_t *ships, int idx, char direction, bool active);

// Ship constants
#define SHIP_MASS 1.0f
#define MAX_SHIPS 52

#endif
