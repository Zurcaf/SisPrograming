#ifndef CONFIG_H
#define CONFIG_H

int load_config(const char *path);

/* Getters for other modules */
int get_width_universe_int(void);
int get_n_planets_int(void);
int get_height_universe_int(void);
int get_max_n_trash_int(void);
int get_init_n_trash_int(void);
int get_capacity_ship_int(void);

/* Socket configuration getters */
int get_server_port_int(void);
const char *get_server_address_str(void);

#endif /* CONFIG_H */
