#include "../head/physics-rules.h"

/**
 * Main physics update loop: acceleration -> velocity -> position
 */
void update_physics(trash_t *trash, int total_trash,
                    planet_t *planets, int total_planets,
                    ship_t *ships, int total_ships,
                    int universe_width, int universe_height)
{
    new_trash_acceleration(planets, total_planets, trash, total_trash);
    new_trash_velocity(trash, total_trash);
    new_trash_position(trash, total_trash, universe_width, universe_height);

    new_ship_acceleration(planets, total_planets, ships, total_ships);
    new_ship_velocity(ships, total_ships);
    new_ship_position(ships, total_ships, universe_width, universe_height);

    // Check collisions for trash collection and recycling
    check_ship_trash_collisions(ships, total_ships, trash, total_trash);
    check_ship_planet_collisions(ships, total_ships, planets, total_planets, trash, total_trash);
}

/**
 * Calculate gravitational acceleration for each trash object
 * F = (m1 * m2) / r^2
 * Uses inverse square law of gravity
 */
void new_trash_acceleration(planet_t *planets, int total_planets,
                            trash_t *trash, int total_trash)
{
    vector_t total_vector_force;

    for (int n_trash = 0; n_trash < total_trash; n_trash++)
    {
        // Skip trash that has been collected (mass = -1 or 0)
        if (trash_get_mass_at(trash, n_trash) <= 0)
        {
            vector_t zero = {0};
            trash_set_acceleration_at(trash, n_trash, zero);
            continue;
        }

        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;

        // Calculate gravitational force from each planet
        // Implements simplified Newton's Law of Universal Gravitation:
        // F = (M * m) / r²
        // where M is planet mass, m is trash mass, and r is distance
        for (int n_planet = 0; n_planet < total_planets; n_planet++)
        {
            // Calculate force direction vector (from trash to planet)
            float force_vector_x = planet_get_x_at(planets, n_planet) - trash_get_x_at(trash, n_trash);
            float force_vector_y = planet_get_y_at(planets, n_planet) - trash_get_y_at(trash, n_trash);
            vector_t local_vector_force = make_vector(force_vector_x, force_vector_y);

            // Apply inverse square law
            // Use minimum distance of 0.001 to avoid division by zero
            // when objects are very close together
            float distance = local_vector_force.amplitude;
            if (distance < 0.001)
                distance = 0.001;

            // F = (M * m) / r²
            // Force decreases with the square of distance
            local_vector_force.amplitude = (planet_get_mass_at(planets, n_planet) * trash_get_mass_at(trash, n_trash)) / (distance * distance);

            // Sum all forces
            // Vector sum of all gravitational forces from planets
            total_vector_force = add_vectors(local_vector_force, total_vector_force);
        }

        trash_set_acceleration_at(trash, n_trash, total_vector_force);
    }
}

/**
 * Update velocity based on acceleration and apply friction
 * v = v + a
 * v = v * 0.99 (friction/drag)
 */
void new_trash_velocity(trash_t *trash, int total_trash)
{
    for (int n_trash = 0; n_trash < total_trash; n_trash++)
    {
        // Skip collected trash
        if (trash_get_mass_at(trash, n_trash) <= 0)
            continue;

        // Apply friction (damping)
        vector_t vel = trash_get_velocity_at(trash, n_trash);
        vel.amplitude *= 0.99;
        vel = add_vectors(vel, trash_get_acceleration_at(trash, n_trash));
        trash_set_velocity_at(trash, n_trash, vel);
    }
}

/**
 * Update position based on velocity
 * x = x + v * cos(angle)
 * y = y + v * sin(angle)
 */
void new_trash_position(trash_t *trash, int total_trash,
                        int universe_width, int universe_height)
{
    for (int n_trash = 0; n_trash < total_trash; n_trash++)
    {
        // Skip collected trash
        if (trash_get_mass_at(trash, n_trash) <= 0)
            continue;

        // Update position based on velocity vector
        vector_t vel = trash_get_velocity_at(trash, n_trash);
        float new_x = trash_get_x_at(trash, n_trash) + vel.amplitude * cos(vel.angle);
        float new_y = trash_get_y_at(trash, n_trash) + vel.amplitude * sin(vel.angle);
        trash_set_position_at(trash, n_trash, new_x, new_y);

        // Wrap around edges (toroidal topology)
        float adj_x = trash_get_x_at(trash, n_trash);
        float adj_y = trash_get_y_at(trash, n_trash);
        correct_position(&adj_x, universe_width);
        correct_position(&adj_y, universe_height);
        trash_set_position_at(trash, n_trash, adj_x, adj_y);
    }
}

/**
 * Convert x,y Cartesian coordinates to polar (amplitude, angle)
 */
vector_t make_vector(float x, float y)
{
    vector_t vector;
    vector.amplitude = sqrt(x * x + y * y);
    vector.angle = atan2(y, x);
    return vector;
}

/**
 * Add two vectors in polar form by converting to Cartesian, adding, then back to polar
 *
 * Vectors are stored in polar coordinates (amplitude, angle) because:
 * 1. It's more intuitive for physics (direction and magnitude)
 * 2. Makes applying directional forces easier
 *
 * But vector addition is simpler in Cartesian coordinates:
 * 1. Convert both vectors to (x,y) using cos/sin
 * 2. Add x and y components separately
 * 3. Convert result back to polar using atan2 and sqrt
 */
vector_t add_vectors(vector_t v1, vector_t v2)
{
    // Convert to Cartesian
    float x = v1.amplitude * cos(v1.angle) + v2.amplitude * cos(v2.angle);
    float y = v1.amplitude * sin(v1.angle) + v2.amplitude * sin(v2.angle);

    // Convert back to polar
    return make_vector(x, y);
}

// Ship physics mirrors trash rules but always uses SHIP_MASS
void new_ship_acceleration(planet_t *planets, int total_planets,
                           ship_t *ships, int total_ships)
{
    vector_t total_vector_force;

    for (int idx = 0; idx < total_ships; idx++)
    {
        // Skip ships not connected (load < 0)
        if (ship_get_load_at(ships, idx) < 0)
        {
            vector_t zero = {0};
            ship_set_acceleration_at(ships, idx, zero);
            continue;
        }

        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;

        for (int n_planet = 0; n_planet < total_planets; n_planet++)
        {
            float force_vector_x = planet_get_x_at(planets, n_planet) - ship_get_x_at(ships, idx);
            float force_vector_y = planet_get_y_at(planets, n_planet) - ship_get_y_at(ships, idx);
            vector_t local_vector_force = make_vector(force_vector_x, force_vector_y);

            float distance = local_vector_force.amplitude;
            if (distance < 0.001f)
                distance = 0.001f;

            // Ship mass fixed at SHIP_MASS
            local_vector_force.amplitude = (planet_get_mass_at(planets, n_planet) * SHIP_MASS) / (distance * distance);

            total_vector_force = add_vectors(local_vector_force, total_vector_force);
        }
        // Add player thrust if any
        vector_t thrust = ship_get_thrust_at(ships, idx);
        total_vector_force = add_vectors(total_vector_force, thrust);

        ship_set_acceleration_at(ships, idx, total_vector_force);
    }
}

void new_ship_velocity(ship_t *ships, int total_ships)
{
    for (int idx = 0; idx < total_ships; idx++)
    {
        if (ship_get_load_at(ships, idx) < 0)
            continue;

        vector_t vel = ship_get_velocity_at(ships, idx);
        vel.amplitude *= 0.99f;
        vel = add_vectors(vel, ship_get_acceleration_at(ships, idx));
        ship_set_velocity_at(ships, idx, vel);
    }
}

void new_ship_position(ship_t *ships, int total_ships,
                       int universe_width, int universe_height)
{
    for (int idx = 0; idx < total_ships; idx++)
    {
        if (ship_get_load_at(ships, idx) < 0)
            continue;

        vector_t vel = ship_get_velocity_at(ships, idx);
        float new_x = ship_get_x_at(ships, idx) + vel.amplitude * cos(vel.angle);
        float new_y = ship_get_y_at(ships, idx) + vel.amplitude * sin(vel.angle);
        ship_set_position_at(ships, idx, new_x, new_y);

        float adj_x = ship_get_x_at(ships, idx);
        float adj_y = ship_get_y_at(ships, idx);
        correct_position(&adj_x, universe_width);
        correct_position(&adj_y, universe_height);
        ship_set_position_at(ships, idx, adj_x, adj_y);
    }
}

/**
 * Wrap position around universe edges
 * Creates toroidal (wrap-around) boundary conditions
 */
void correct_position(float *position, int edge_size)
{
    // Keep coordinates in [0, edge_size) using modular wrap.
    // Handles large jumps across multiple widths/heights cleanly.
    float wrapped = fmodf(*position, (float)edge_size);
    if (wrapped < 0)
    {
        wrapped += edge_size;
    }
    *position = wrapped;
}

/**
 * Check for collisions between trash and planets
 * Returns true if any collision detected
 */
bool check4collisions(trash_t *trash, int *n_trash,
                      planet_t *planets, int n_planets)
{
    for (int i = 0; i < n_planets; i++)
    {
        for (int j = 0; j < *n_trash; j++)
        {
            float dx = planet_get_x_at(planets, i) - trash_get_x_at(trash, j);
            float dy = planet_get_y_at(planets, i) - trash_get_y_at(trash, j);
            float distance = sqrt(dx * dx + dy * dy);

            // Collision threshold
            if (distance < 1)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * Check ship-trash collisions and collect trash
 * Ships collect trash when within radius if not at capacity
 */
void check_ship_trash_collisions(ship_t *ships, int total_ships,
                                 trash_t *trash, int total_trash)
{
    const float COLLECT_RADIUS = 7.0f;

    for (int si = 0; si < total_ships; si++)
    {
        // Skip disconnected ships
        if (ship_get_load_at(ships, si) < 0)
            continue;

        // Skip if at capacity
        if (ship_get_load_at(ships, si) >= ship_get_capacity_at(ships, si))
            continue;

        float ship_x = ship_get_x_at(ships, si);
        float ship_y = ship_get_y_at(ships, si);

        for (int ti = 0; ti < total_trash; ti++)
        {
            // Skip already collected trash
            if (trash_get_mass_at(trash, ti) <= 0)
                continue;

            float dx = ship_x - trash_get_x_at(trash, ti);
            float dy = ship_y - trash_get_y_at(trash, ti);
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < COLLECT_RADIUS)
            {
                trash_set_mass_at(trash, ti, 0); // Collected (held by ship)
                ship_increment_load_at(ships, si);
            }
        }
    }
}

/**
 * Check ship-planet collisions for recycling/dumping
 * Recycling planet (mass=0) removes trash, others drop it back
 *
 * CRITICAL GAME LOGIC:
 * 1. Recycling planet (mass=0): Removes trash from universe (success!)
 *    - trash_mass = -1 means "destroyed/recycled"
 * 2. Normal planets (mass=10): Trash returns to universe (failure)
 *    - trash_mass = 1 means "active in universe"
 * 3. Collected trash has mass=0 while on ship
 *
 * This mechanic forces strategy: players must go to the correct planet
 */
void check_ship_planet_collisions(ship_t *ships, int total_ships,
                                  planet_t *planets, int total_planets,
                                  trash_t *trash, int total_trash)
{
    const float PLANET_RADIUS = 14.0f;

    for (int si = 0; si < total_ships; si++)
    {
        // Skip disconnected ships
        if (ship_get_load_at(ships, si) < 0)
            continue;

        // Skip if no trash to drop
        if (ship_get_load_at(ships, si) <= 0)
            continue;

        float ship_x = ship_get_x_at(ships, si);
        float ship_y = ship_get_y_at(ships, si);

        for (int pi = 0; pi < total_planets; pi++)
        {
            float dx = ship_x - planet_get_x_at(planets, pi);
            float dy = ship_y - planet_get_y_at(planets, pi);
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < PLANET_RADIUS)
            {
                bool is_recycling = (planet_get_mass_at(planets, pi) == 0);

                // Process all collected trash
                for (int ti = 0; ti < total_trash; ti++)
                {
                    if (trash_get_mass_at(trash, ti) == 0) // Collected trash
                    {
                        if (is_recycling)
                        {
                            trash_set_mass_at(trash, ti, -1); // Remove from universe
                        }
                        else
                        {
                            trash_set_mass_at(trash, ti, 1); // Drop back to universe
                        }
                    }
                }

                ship_reset_load_at(ships, si); // Empty the ship
                break;                         // Only one planet collision per frame
            }
        }
    }
}
