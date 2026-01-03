#include "../head/physics-rules.h"

/**
 * Main physics update loop: acceleration -> velocity -> position
 */
void update_physics(trash_t *trash, int total_trash,
                    planet_t *planets, int total_planets,
                    int universe_width, int universe_height)
{
    new_trash_acceleration(planets, total_planets, trash, total_trash);
    new_trash_velocity(trash, total_trash);
    new_trash_position(trash, total_trash, universe_width, universe_height);
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
        for (int n_planet = 0; n_planet < total_planets; n_planet++)
        {
            float force_vector_x = planet_get_x_at(planets, n_planet) - trash_get_x_at(trash, n_trash);
            float force_vector_y = planet_get_y_at(planets, n_planet) - trash_get_y_at(trash, n_trash);
            vector_t local_vector_force = make_vector(force_vector_x, force_vector_y);

            // Apply inverse square law
            float distance = local_vector_force.amplitude;
            if (distance < 0.001)
                distance = 0.001;

            // F = (M * m) / r^2
            local_vector_force.amplitude = (planet_get_mass_at(planets, n_planet) * trash_get_mass_at(trash, n_trash)) / (distance * distance);

            // Sum all forces
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
 */
vector_t add_vectors(vector_t v1, vector_t v2)
{
    // Convert to Cartesian
    float x = v1.amplitude * cos(v1.angle) + v2.amplitude * cos(v2.angle);
    float y = v1.amplitude * sin(v1.angle) + v2.amplitude * sin(v2.angle);

    // Convert back to polar
    return make_vector(x, y);
}

/**
 * Wrap position around universe edges
 * Creates toroidal (wrap-around) boundary conditions
 */
void correct_position(float *position, int edge_size)
{
    if (*position < 0)
    {
        *position = edge_size; // Wrap to opposite side
    }
    if (*position > edge_size)
    {
        *position = 0; // Wrap to opposite side
    }
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
