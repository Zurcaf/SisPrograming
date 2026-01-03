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
        ship[i].capacity = capacity;
        ship[i].current_load = -1; // Initialize current load to -1 (indicating not connected)
    }
    return ship;
}

void handle_data(ship_t *ship, char direction, trash_t *trash, planet_t *planets,
                 int width, int height, int n_trash, int n_planets, int ship_index)
{
    const int SHIP_SPEED = 10; // Increased from 3 to 10 for faster movement

    switch (direction)
    {
    case 'u':
        ship[ship_index].y -= SHIP_SPEED;
        correct_position(&ship[ship_index].y, height);
        break;
    case 'd':
        ship[ship_index].y += SHIP_SPEED;
        correct_position(&ship[ship_index].y, height);
        break;
    case 'r':
        ship[ship_index].x += SHIP_SPEED;
        correct_position(&ship[ship_index].x, width);
        break;
    case 'l':
        ship[ship_index].x -= SHIP_SPEED;
        correct_position(&ship[ship_index].x, width);
        break;
    }
    if (ship[ship_index].current_load < ship[ship_index].capacity)
    { // if ship collides with trash
        for (int i = 0; i < n_trash; i++)
        {
            if (contact_made(ship[ship_index].x, ship[ship_index].y, trash[i].x, trash[i].y, 7) && trash[i].mass > 0)
            {
                trash[i].mass = 0; // Collect the trash
                ship[ship_index].current_load++;
            }
        }
    }

    if (ship[ship_index].current_load > 0)
    { // if ship collides with planet
        for (int i = 0; i < n_planets; i++)
        {
            if (contact_made(ship[ship_index].x, ship[ship_index].y, planets[i].x, planets[i].y, 14))
            {
                ship[ship_index].current_load = 0;
                if (planets[i].mass == 0)
                { // recycling planet
                    for (int j = 0; j < n_trash; j++)
                    {
                        if (trash[j].mass == 0)
                        {
                            trash[j].mass = -1; // remove trash from universe
                        }
                    }
                }
                else
                {
                    for (int j = 0; j < n_trash; j++)
                    {
                        if (trash[j].mass == 0)
                        {
                            trash[j].mass = 1; // drop trash back to universe
                        }
                    }
                }
            }
        }
    }

    return;
}

bool contact_made(float src_x, float src_y, float dest_x, float dest_y, int radius)
{
    float distance_x = src_x - dest_x;
    float distance_y = src_y - dest_y;
    float distance = sqrt(distance_x * distance_x + distance_y * distance_y);
    return distance < radius; // Assuming contact is made if distance is less than 1 unit
}

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
