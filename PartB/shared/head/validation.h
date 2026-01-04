#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#define MAX_PASSWORD_LEN 64
#define MAX_CLIENT_ID_LEN 2
#define MAX_CLIENTS 52

/**
 * Per-client password storage
 */
typedef struct {
    char client_id;
    char password[MAX_PASSWORD_LEN];
    bool has_password;
    bool authenticated;
} client_password_t;

extern client_password_t client_passwords[MAX_CLIENTS];

/**
 * Initialize client password table (server startup)
 * No password is set initially; it will be set on first CONNECT
 */
static inline void init_client_passwords(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (i < 26)
        {
            client_passwords[i].client_id = 'A' + i;
        }
        else
        {
            client_passwords[i].client_id = 'a' + (i - 26);
        }
        client_passwords[i].password[0] = '\0';
        client_passwords[i].has_password = false;
        client_passwords[i].authenticated = false;
    }
}

/**
 * Get password for a specific client ID
 */
static inline const char *get_client_password(char client_id)
{
    int idx = -1;
    if (client_id >= 'A' && client_id <= 'Z')
        idx = client_id - 'A';
    else if (client_id >= 'a' && client_id <= 'z')
        idx = 26 + (client_id - 'a');
    
    if (idx >= 0 && idx < MAX_CLIENTS)
        return client_passwords[idx].password;
    return NULL;
}

/**
 * Get array index for a client ID
 */
static inline int get_client_index(char client_id)
{
    if (client_id >= 'A' && client_id <= 'Z')
        return client_id - 'A';
    else if (client_id >= 'a' && client_id <= 'z')
        return 26 + (client_id - 'a');
    return -1;
}

/**
 * Validate client ID (single character: a-z or A-Z)
 */
static inline bool is_valid_client_id(char id)
{
    return (id >= 'a' && id <= 'z') || (id >= 'A' && id <= 'Z');
}

/**
 * Validate direction (u/d/l/r)
 */
static inline bool is_valid_direction(char direction)
{
    return direction == 'u' || direction == 'd' || direction == 'l' || direction == 'r';
}

/**
 * Basic password format validation (length only)
 */
static inline bool is_valid_password_format(const char *password)
{
    if (!password || strlen(password) == 0 || strlen(password) > MAX_PASSWORD_LEN)
        return false;
    return true;
}

/**
 * Set password for client if not already set
 */
static inline bool set_client_password_if_empty(char client_id, const char *password)
{
    int idx = get_client_index(client_id);
    if (idx < 0 || idx >= MAX_CLIENTS || !is_valid_password_format(password))
        return false;
    if (client_passwords[idx].has_password)
        return false;
    strncpy(client_passwords[idx].password, password, MAX_PASSWORD_LEN - 1);
    client_passwords[idx].password[MAX_PASSWORD_LEN - 1] = '\0';
    client_passwords[idx].has_password = true;
    return true;
}

/**
 * Validate password for a specific client (must already be set)
 */
static inline bool is_valid_client_password(char client_id, const char *password)
{
    if (!is_valid_password_format(password))
        return false;

    int idx = get_client_index(client_id);
    if (idx < 0 || idx >= MAX_CLIENTS)
        return false;
    if (!client_passwords[idx].has_password)
        return false;

    return strcmp(password, client_passwords[idx].password) == 0;
}

/**
 * Mark client as authenticated after successful CONNECT
 */
static inline void mark_client_authenticated(char client_id)
{
    int idx = get_client_index(client_id);
    if (idx >= 0 && idx < MAX_CLIENTS)
        client_passwords[idx].authenticated = true;
}

/**
 * Check if client is authenticated
 */
static inline bool is_client_authenticated(char client_id)
{
    int idx = get_client_index(client_id);
    if (idx >= 0 && idx < MAX_CLIENTS)
        return client_passwords[idx].authenticated;
    return false;
}

#endif
