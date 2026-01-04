#include "config.h"
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* configuration parameters with default values */
static int width_universe_int; /* sensible defaults */
static int height_universe_int;
static int max_n_trash_int;
static int init_n_trash_int;
static int capacity_ship_int;
static int n_planets_int;

/* socket configuration */
static int server_port_int = 45007;                /* default port */
static char server_address_str[256] = "localhost"; /* default address */

/* Validate all loaded configuration values */
static int validate_config(void)
{
    int valid = 1;

    /* Dimension validation */
    if (width_universe_int <= 0)
    {
        fprintf(stderr, "[Config] ERROR: width_universe_int must be > 0, got %d\n", width_universe_int);
        valid = 0;
    }
    if (height_universe_int <= 0)
    {
        fprintf(stderr, "[Config] ERROR: height_universe_int must be > 0, got %d\n", height_universe_int);
        valid = 0;
    }

    /* Trash configuration validation */
    if (max_n_trash_int <= 0)
    {
        fprintf(stderr, "[Config] ERROR: max_n_trash_int must be > 0, got %d\n", max_n_trash_int);
        valid = 0;
    }
    if (init_n_trash_int < 0 || init_n_trash_int > max_n_trash_int)
    {
        fprintf(stderr, "[Config] ERROR: init_n_trash_int (%d) must be between 0 and max_n_trash_int (%d)\n",
                init_n_trash_int, max_n_trash_int);
        valid = 0;
    }

    /* Ship capacity validation */
    if (capacity_ship_int <= 0)
    {
        fprintf(stderr, "[Config] ERROR: capacity_ship_int must be > 0, got %d\n", capacity_ship_int);
        valid = 0;
    }

    /* Planet validation */
    if (n_planets_int <= 0)
    {
        fprintf(stderr, "[Config] ERROR: n_planets_int must be > 0, got %d\n", n_planets_int);
        valid = 0;
    }

    /* Port validation */
    if (server_port_int < 1 || server_port_int > 65535)
    {
        fprintf(stderr, "[Config] ERROR: server_port_int must be between 1 and 65535, got %d\n", server_port_int);
        valid = 0;
    }

    /* Server address validation */
    if (strlen(server_address_str) == 0)
    {
        fprintf(stderr, "[Config] ERROR: server_address_str is empty\n");
        valid = 0;
    }

    return valid;
}

/* load once from file; leaves defaults if key missing */
int load_config(const char *path)
{
    config_t cfg;
    config_init(&cfg);

    if (!config_read_file(&cfg, path))
    {
        fprintf(stderr, "Config file error: %s:%d - %s\n",
                config_error_file(&cfg) ? config_error_file(&cfg) : path,
                config_error_line(&cfg),
                config_error_text(&cfg));
        config_destroy(&cfg);
        return 1;
    }

    /* only overwrite defaults when lookup succeeds */
    (void)config_lookup_int(&cfg, "width_universe_int", &width_universe_int);
    (void)config_lookup_int(&cfg, "height_universe_int", &height_universe_int);
    (void)config_lookup_int(&cfg, "max_n_trash_int", &max_n_trash_int);
    (void)config_lookup_int(&cfg, "init_n_trash_int", &init_n_trash_int);
    (void)config_lookup_int(&cfg, "capacity_ship_int", &capacity_ship_int);
    (void)config_lookup_int(&cfg, "n_planets_int", &n_planets_int);

    /* socket configuration */
    (void)config_lookup_int(&cfg, "server_port_int", &server_port_int);
    const char *addr_temp = NULL;
    if (config_lookup_string(&cfg, "server_address_str", &addr_temp))
    {
        strncpy(server_address_str, addr_temp, sizeof(server_address_str) - 1);
        server_address_str[sizeof(server_address_str) - 1] = '\0';
    }

    config_destroy(&cfg);

    /* Validate all parameters */
    if (!validate_config())
    {
        return 1;
    }

    printf("[Config] Configuration validated successfully\n");
    return 0;
}

/* getters (read-only access for other modules) */
int get_width_universe_int(void) { return width_universe_int; }
int get_height_universe_int(void) { return height_universe_int; }
int get_max_n_trash_int(void) { return max_n_trash_int; }
int get_init_n_trash_int(void) { return init_n_trash_int; }
int get_capacity_ship_int(void) { return capacity_ship_int; }
int get_n_planets_int(void) { return n_planets_int; }

/* socket configuration getters */
int get_server_port_int(void) { return server_port_int; }
const char *get_server_address_str(void) { return server_address_str; }