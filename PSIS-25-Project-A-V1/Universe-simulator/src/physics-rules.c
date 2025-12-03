#include "../head/physics-rules.h"

void update_physics(trash_t *trash, int total_trash,
                    planet_t *planets, int total_planets,
                    int universe_width, int universe_height){
    new_trash_acceleration(planets, total_planets, trash, total_trash);
    new_trash_velocity(trash, total_trash);
    new_trash_position(trash, total_trash, universe_width, universe_height);
}


void new_trash_acceleration(planet_t *planets, int total_planets,
                            trash_t *trash, int total_trash){
    vector_t total_vector_force;

    for (int n_trash = 0; n_trash < total_trash; n_trash ++){
        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;
        for (int n_planet = 0; n_planet < total_planets; n_planet ++){
            float force_vector_x = planets[n_planet].x - trash[n_trash].x;
            float force_vector_y = planets[n_planet].y - trash[n_trash].y;
            vector_t local_vector_force = make_vector(force_vector_x, force_vector_y);
            local_vector_force.amplitude = (planets[n_planet].mass * trash[n_trash].mass)/
                                            pow(local_vector_force.amplitude, 2);
            total_vector_force = add_vectors(local_vector_force, total_vector_force);
        }
        trash[n_trash].acceleration = total_vector_force ; // / trash[n_trash].mass
    }
}

void new_trash_velocity(trash_t *trash, int total_trash){
    for (int n_trash = 0; n_trash < total_trash; n_trash ++){
        trash[n_trash].velocity.amplitude *= 0.99; //friction
        trash[n_trash].velocity = add_vectors(trash[n_trash].velocity, trash[n_trash].acceleration);
    }
}

void new_trash_position( trash_t *trash, int total_trash, int universe_width, int universe_height){
    for (int n_trash = 0; n_trash < total_trash; n_trash ++){
        trash[n_trash].x += trash[n_trash].velocity.amplitude * cos( trash[n_trash].velocity.angle);
        trash[n_trash].y += trash[n_trash].velocity.amplitude * sin( trash[n_trash].velocity.angle);
        correct_position(&trash[n_trash].x, universe_width);
        correct_position(&trash[n_trash].y, universe_height);
    }
}

vector_t make_vector(float x, float y){
    vector_t vector;
    vector.amplitude = sqrt(x*x + y*y);
    vector.angle = atan2(y, x);
    return vector;
}

vector_t add_vectors(vector_t v1, vector_t v2){
    float x = v1.amplitude * cos(v1.angle) + v2.amplitude * cos(v2.angle);
    float y = v1.amplitude * sin(v1.angle) + v2.amplitude * sin(v2.angle);
    return make_vector(x, y);
}

void correct_position(float *position, int edge_size){
    if (*position < 0){
        *position = edge_size; //universe width/height
    }
    if (*position > edge_size){
        *position = 0;
    }
}




