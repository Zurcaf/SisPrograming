#include "../head/universe_data.h"

// Concrete definitions (hidden from header for data abstraction)
struct planet
{
    float x, y;
    float mass;
};

struct trash
{
    float x, y;
    vector_t velocity;
    vector_t acceleration;
    float mass;
};

struct ship
{
    float x, y;
    vector_t velocity;
    vector_t acceleration;
    vector_t thrust; // persistent thrust from player input
    int capacity;
    int current_load;
};

trash_t *init_trash(int n, int max, int width, int height)
{
    trash_t *trash = malloc(max * sizeof(trash_t));
    int cnt = 0;
    if (trash == NULL)
    {
        fprintf(stderr, "Memory allocation failed for trash.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < max; i++)
    {
        if (cnt < n)
        {                      // init trash
            trash[i].mass = 1; // 1 mass unit
            cnt++;
        }
        else
        {                       // not init trash
            trash[i].mass = -1; // 0 mass unit
        }
        trash[i].x = rand() % width;
        trash[i].y = rand() % height;
        trash[i].velocity.amplitude = 0;
        trash[i].velocity.angle = 0;
        trash[i].acceleration.amplitude = 0;
        trash[i].acceleration.angle = 0;
    }
    return trash;
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

    int random = rand() % n; // select random planet to be recycling planet
    planets[random].mass = 0;

    return planets;
}

bool addTrash(trash_t *trash, int *n_trash, int max, int width, int height)
{
    int idx = -1;

    // Try to reuse an empty slot
    for (int i = 0; i < *n_trash; i++)
    {
        if (trash[i].mass <= 0)
        {
            idx = i;
            break;
        }
    }

    // Otherwise append if there is capacity
    if (idx == -1)
    {
        if (*n_trash >= max)
        {
            return false;
        }
        idx = *n_trash;
        (*n_trash)++;
    }

    trash[idx].x = rand() % width;
    trash[idx].y = rand() % height;
    trash[idx].mass = 1; // 1 mass unit
    trash[idx].velocity.amplitude = 0;
    trash[idx].velocity.angle = 0;
    trash[idx].acceleration.amplitude = 0;
    trash[idx].acceleration.angle = 0;
    return true;
}

ship_t *init_ship(int capacity)
{
    ship_t *ship = malloc(52 * sizeof(ship_t)); // allocate space for 52 ships (A-Z, a-z)
    if (ship == NULL)
    {
        fprintf(stderr, "Memory allocation failed for ship.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 52; i++)
    {
        ship[i].x = 0;
        ship[i].y = 0;
        ship[i].velocity.amplitude = 0;
        ship[i].velocity.angle = 0;
        ship[i].acceleration.amplitude = 0;
        ship[i].acceleration.angle = 0;
        ship[i].thrust.amplitude = 0;
        ship[i].thrust.angle = 0;
        ship[i].capacity = capacity;
        ship[i].current_load = -1; // Initialize current load to -1 (indicating not connected)
    }
    return ship;
}

vector_t ship_get_thrust_at(const ship_t *list, int idx)
{
    return list[idx].thrust;
}

void ship_set_thrust_at(ship_t *list, int idx, vector_t v)
{
    list[idx].thrust = v;
}

static vector_t thrust_vector_from_direction(char direction)
{
    const float THRUST_FORCE = 0.08f; // tuneable thrust magnitude
    vector_t thrust = {0};

    switch (direction)
    {
    case 'u':
        thrust.angle = -M_PI_2;
        break; // up
    case 'd':
        thrust.angle = M_PI_2;
        break; // down
    case 'l':
        thrust.angle = M_PI;
        break; // left
    case 'r':
        thrust.angle = 0.0f;
        break; // right
    default:
        thrust.angle = 0.0f;
        break;
    }
    thrust.amplitude = THRUST_FORCE;
    return thrust;
}

// Convert vector from polar (amplitude, angle) to cartesian (x, y)
static void vector_to_cartesian(const vector_t *polar, float *vx, float *vy)
{
    *vx = polar->amplitude * cosf(polar->angle);
    *vy = polar->amplitude * sinf(polar->angle);
}

// Convert vector from cartesian (x, y) to polar (amplitude, angle)
static void vector_from_cartesian(float vx, float vy, vector_t *polar)
{
    polar->amplitude = sqrtf(vx * vx + vy * vy);
    polar->angle = atan2f(vy, vx);
}

/**
 * Apply or remove directional thrust from a ship
 *
 * MULTI-DIRECTIONAL CONTROL SYSTEM:
 * Allows multiple keys to be pressed simultaneously
 * (e.g., UP+RIGHT for diagonal movement)
 *
 * HOW IT WORKS:
 * 1. Convert ship's current thrust to Cartesian coordinates (vx, vy)
 * 2. If active=true: ADD thrust vector for pressed direction
 * 3. If active=false: SUBTRACT thrust vector for released direction
 * 4. Convert result back to polar coordinates
 *
 * This allows continuous thrust while key is held,
 * and smooth removal when key is released (SDL_KEYUP)
 */
void apply_thrust(ship_t *ships, int idx, char direction, bool active)
{
    if (idx < 0)
    {
        return;
    }

    // Get current thrust as cartesian coordinates
    vector_t current = ship_get_thrust_at(ships, idx);
    float current_vx, current_vy;
    vector_to_cartesian(&current, &current_vx, &current_vy);

    if (!active)
    {
        // Remove thrust for this direction
        vector_t new_dir = thrust_vector_from_direction(direction);
        float dir_vx, dir_vy;
        vector_to_cartesian(&new_dir, &dir_vx, &dir_vy);

        // Subtract this direction's thrust
        current_vx -= dir_vx;
        current_vy -= dir_vy;
    }
    else
    {
        // Add thrust for this direction
        vector_t new_dir = thrust_vector_from_direction(direction);
        float dir_vx, dir_vy;
        vector_to_cartesian(&new_dir, &dir_vx, &dir_vy);

        // Add this direction's thrust
        current_vx += dir_vx;
        current_vy += dir_vy;
    }

    // Convert back to polar and apply
    vector_t result;
    vector_from_cartesian(current_vx, current_vy, &result);
    ship_set_thrust_at(ships, idx, result);
}

/**
 * Convert a character (client ID) to index in ships array
 *
 * IDENTIFICATION SYSTEM:
 * - Supports 52 simultaneous players (A-Z = 26, a-z = 26)
 * - 'A' -> index 0, 'B' -> index 1, ..., 'Z' -> index 25
 * - 'a' -> index 26, 'b' -> index 27, ..., 'z' -> index 51
 *
 * Returns -1 for invalid characters
 * This direct mapping is efficient and avoids complex lookup structures
 */
int ship_index(char id)
{
    if (id >= 'A' && id <= 'Z')
        return id - 'A';
    if (id >= 'a' && id <= 'z')
        return 26 + (id - 'a');
    return -1; // invalid / trash input
}

// ----------------------
// Planet accessors
// ----------------------
float planet_get_x(const planet_t *p) { return p->x; }
float planet_get_y(const planet_t *p) { return p->y; }
float planet_get_mass(const planet_t *p) { return p->mass; }
void planet_set_mass(planet_t *p, float mass) { p->mass = mass; }
float planet_get_x_at(const planet_t *list, int idx) { return list[idx].x; }
float planet_get_y_at(const planet_t *list, int idx) { return list[idx].y; }
float planet_get_mass_at(const planet_t *list, int idx) { return list[idx].mass; }
void planet_set_mass_at(planet_t *list, int idx, float mass) { list[idx].mass = mass; }

// ----------------------
// Trash accessors
// ----------------------
float trash_get_x(const trash_t *t) { return t->x; }
float trash_get_y(const trash_t *t) { return t->y; }
float trash_get_mass(const trash_t *t) { return t->mass; }
vector_t trash_get_velocity(const trash_t *t) { return t->velocity; }
vector_t trash_get_acceleration(const trash_t *t) { return t->acceleration; }
void trash_set_position(trash_t *t, float x, float y)
{
    t->x = x;
    t->y = y;
}
void trash_set_mass(trash_t *t, float mass) { t->mass = mass; }
void trash_set_velocity(trash_t *t, vector_t v) { t->velocity = v; }
void trash_set_acceleration(trash_t *t, vector_t a) { t->acceleration = a; }
float trash_get_x_at(const trash_t *list, int idx) { return list[idx].x; }
float trash_get_y_at(const trash_t *list, int idx) { return list[idx].y; }
float trash_get_mass_at(const trash_t *list, int idx) { return list[idx].mass; }
vector_t trash_get_velocity_at(const trash_t *list, int idx) { return list[idx].velocity; }
vector_t trash_get_acceleration_at(const trash_t *list, int idx) { return list[idx].acceleration; }
void trash_set_position_at(trash_t *list, int idx, float x, float y)
{
    list[idx].x = x;
    list[idx].y = y;
}
void trash_set_mass_at(trash_t *list, int idx, float mass) { list[idx].mass = mass; }
void trash_set_velocity_at(trash_t *list, int idx, vector_t v) { list[idx].velocity = v; }
void trash_set_acceleration_at(trash_t *list, int idx, vector_t a) { list[idx].acceleration = a; }

// ----------------------
// Ship accessors
// ----------------------
float ship_get_x(const ship_t *s) { return s->x; }
float ship_get_y(const ship_t *s) { return s->y; }
int ship_get_capacity(const ship_t *s) { return s->capacity; }
int ship_get_load(const ship_t *s) { return s->current_load; }
void ship_set_position(ship_t *s, float x, float y)
{
    s->x = x;
    s->y = y;
}
void ship_set_load(ship_t *s, int load) { s->current_load = load; }
void ship_increment_load(ship_t *s) { s->current_load++; }
void ship_reset_load(ship_t *s) { s->current_load = 0; }
float ship_get_x_at(const ship_t *list, int idx) { return list[idx].x; }
float ship_get_y_at(const ship_t *list, int idx) { return list[idx].y; }
int ship_get_capacity_at(const ship_t *list, int idx) { return list[idx].capacity; }
int ship_get_load_at(const ship_t *list, int idx) { return list[idx].current_load; }
void ship_set_position_at(ship_t *list, int idx, float x, float y)
{
    list[idx].x = x;
    list[idx].y = y;
}
void ship_set_load_at(ship_t *list, int idx, int load) { list[idx].current_load = load; }
void ship_increment_load_at(ship_t *list, int idx) { list[idx].current_load++; }
void ship_reset_load_at(ship_t *list, int idx) { list[idx].current_load = 0; }
vector_t ship_get_velocity_at(const ship_t *list, int idx) { return list[idx].velocity; }
vector_t ship_get_acceleration_at(const ship_t *list, int idx) { return list[idx].acceleration; }
void ship_set_velocity_at(ship_t *list, int idx, vector_t v) { list[idx].velocity = v; }
void ship_set_acceleration_at(ship_t *list, int idx, vector_t a) { list[idx].acceleration = a; }
