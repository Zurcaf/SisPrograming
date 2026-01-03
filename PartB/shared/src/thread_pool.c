#include "../head/thread_pool.h"
#include <sys/time.h>
#include <stdio.h>

int thread_sync_init(thread_sync_t *sync)
{
    if (pthread_mutex_init(&sync->universe_mutex, NULL) != 0)
    {
        fprintf(stderr, "Failed to initialize universe_mutex\n");
        return -1;
    }

    if (pthread_mutex_init(&sync->game_state_mutex, NULL) != 0)
    {
        fprintf(stderr, "Failed to initialize game_state_mutex\n");
        pthread_mutex_destroy(&sync->universe_mutex);
        return -1;
    }

    sync->physics_ready = false;
    sync->comm_ready = false;
    sync->main_ready = false;

    return 0;
}

void thread_sync_cleanup(thread_sync_t *sync)
{
    pthread_mutex_destroy(&sync->universe_mutex);
    pthread_mutex_destroy(&sync->game_state_mutex);
}

void lock_universe(thread_sync_t *sync)
{
    pthread_mutex_lock(&sync->universe_mutex);
}

void unlock_universe(thread_sync_t *sync)
{
    pthread_mutex_unlock(&sync->universe_mutex);
}

void lock_game_state(thread_sync_t *sync)
{
    pthread_mutex_lock(&sync->game_state_mutex);
}

void unlock_game_state(thread_sync_t *sync)
{
    pthread_mutex_unlock(&sync->game_state_mutex);
}

int create_physics_thread(pthread_t *thread_id, void *(*func)(void *), void *arg)
{
    if (pthread_create(thread_id, NULL, func, arg) != 0)
    {
        fprintf(stderr, "Failed to create physics thread\n");
        return -1;
    }
    return 0;
}

int create_communication_thread(pthread_t *thread_id, void *(*func)(void *), void *arg)
{
    if (pthread_create(thread_id, NULL, func, arg) != 0)
    {
        fprintf(stderr, "Failed to create communication thread\n");
        return -1;
    }
    return 0;
}

uint64_t get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}
