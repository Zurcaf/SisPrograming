#include "../head/universe_data.h"

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

void addTrash(int n_trash, trash_t *trash, int width, int height)
{
    // Initialize a new trash object at the given index
    trash[n_trash].x = rand() % width;
    trash[n_trash].y = rand() % height;
    trash[n_trash].mass = 1; // 1 mass unit
    trash[n_trash].velocity.amplitude = 0;
    trash[n_trash].velocity.angle = 0;
    trash[n_trash].acceleration.amplitude = 0;
    trash[n_trash].acceleration.angle = 0;
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
