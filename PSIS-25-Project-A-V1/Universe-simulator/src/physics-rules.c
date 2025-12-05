#include "../head/physics-rules.h"

void update_physics(trash_t *trash, int total_trash,
                    planet_t *planets, int total_planets,
                    int universe_width, int universe_height)
{
    new_trash_acceleration(planets, total_planets, trash, total_trash);
    new_trash_velocity(trash, total_trash);
    new_trash_position(trash, total_trash, universe_width, universe_height);
}

void new_trash_acceleration(planet_t *planets, int total_planets,
                            trash_t *trash, int total_trash)
{
    vector_t total_vector_force;

    for (int n_trash = 0; n_trash < total_trash; n_trash++)
    {
        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;
        for (int n_planet = 0; n_planet < total_planets; n_planet++)
        {
            float planet_x, planet_y, trash_x, trash_y;
            get_planet_cords(n_planet, planets, &planet_x, &planet_y);
            get_trash_cords(n_trash, trash, &trash_x, &trash_y);
            float force_vector_x = planet_x - trash_x;
            float force_vector_y = planet_y - trash_y;

            vector_t local_vector_force = make_vector(force_vector_x, force_vector_y);

            float distance = local_vector_force.amplitude;
            if (distance < 0.001)
                distance = 0.001;

            int planet_mass, trash_mass;
            get_trash_mass(n_trash, trash, &trash_mass);
            get_planet_mass(n_planet, planets, &planet_mass);

            local_vector_force.amplitude = (planet_mass * trash_mass) / (distance * distance);

            total_vector_force = add_vectors(local_vector_force, total_vector_force);

            // set new trash acceleration
            update_trash_acceleration(n_trash, trash, total_vector_force.amplitude, total_vector_force.angle);
        }
    }
}

void new_trash_velocity(trash_t *trash, int total_trash)
{
    for (int n_trash = 0; n_trash < total_trash; n_trash++)
    {

        friction_in_trash_velocity(n_trash, trash, 0.99);

        float accelelation_ampl, acceleration_angle, velocity_ampl, velocity_angle;
        get_trash_acceleration(n_trash, trash, &accelelation_ampl, &acceleration_angle);
        get_trash_velocity(n_trash, trash, &velocity_ampl, &velocity_angle);

        vector_t acceleration_vector = make_vector(accelelation_ampl, acceleration_angle);
        vector_t velocity_vector = make_vector(velocity_ampl, velocity_angle);

        // add acceleration to velocity
        vector_t new_velocity_vector = add_vectors(acceleration_vector, velocity_vector);

        update_trash_velocity(n_trash, trash, new_velocity_vector.amplitude, new_velocity_vector.angle);
    }
}

void new_trash_position(trash_t *trash, int total_trash, int universe_width, int universe_height)
{
    for (int n_trash = 0; n_trash < total_trash; n_trash++)
    {
        float velocity_ampl, velocity_angle;
        get_trash_velocity(n_trash, trash, &velocity_ampl, &velocity_angle);

        float delta_x = velocity_ampl * cos(velocity_angle);
        float delta_y = velocity_ampl * sin(velocity_angle);

        update_trash_cords(n_trash, trash, delta_x, delta_y);

        correct_position(n_trash, trash, universe_width, universe_height);
    }
}

vector_t make_vector(float x, float y)
{
    vector_t vector;
    vector.amplitude = sqrt(x * x + y * y);
    vector.angle = atan2(y, x);
    return vector;
}

vector_t add_vectors(vector_t v1, vector_t v2)
{
    float x = v1.amplitude * cos(v1.angle) + v2.amplitude * cos(v2.angle);
    float y = v1.amplitude * sin(v1.angle) + v2.amplitude * sin(v2.angle);
    return make_vector(x, y);
}

bool check4collisions(trash_t *trash, int *n_trash,
                      planet_t *planets, int n_planets)
{
    // add new trash if collision with planet
    for (int i = 0; i < n_planets; i++)
    {
        // compare planet position with all trash positions
        for (int j = 0; j < *n_trash; j++)
        {
            float planet_x, planet_y, trash_x, trash_y;
            get_planet_cords(i, planets, &planet_x, &planet_y);
            get_trash_cords(j, trash, &trash_x, &trash_y);

            float dx = planet_x - trash_x;
            float dy = planet_y - trash_y;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance < 1) // collision threshold
            {
                return true; // collision occurred
            }
        }
    }

    return false; // no collision
}