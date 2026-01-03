#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Thread management abstraction for the universe server.
 * Handles:
 * - Physics thread (10ms)
 * - Communication thread (ZMQ)
 * - Main thread (SDL + display)
 * - Data synchronization via mutexes
 */

typedef struct
{
    pthread_t thread_id;
    bool running;
    uint64_t last_update_ms;
} thread_info_t;

typedef struct
{
    pthread_mutex_t universe_mutex;   // Protects planets, trash, ships
    pthread_mutex_t game_state_mutex; // Protects running, game_over flags
    bool physics_ready;
    bool comm_ready;
    bool main_ready;
} thread_sync_t;

// Initialize thread synchronization structures
int thread_sync_init(thread_sync_t *sync);

// Cleanup thread synchronization
void thread_sync_cleanup(thread_sync_t *sync);

// Lock/unlock universe data
void lock_universe(thread_sync_t *sync);
void unlock_universe(thread_sync_t *sync);

// Lock/unlock game state
void lock_game_state(thread_sync_t *sync);
void unlock_game_state(thread_sync_t *sync);

// Thread creation helpers
int create_physics_thread(pthread_t *thread_id, void *(*func)(void *), void *arg);
int create_communication_thread(pthread_t *thread_id, void *(*func)(void *), void *arg);

// Helper to get current time in milliseconds
uint64_t get_time_ms(void);

#endif
