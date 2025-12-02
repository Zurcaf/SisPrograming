#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

typedef struct vector vector_t;   
typedef struct trash  trash_t;
typedef struct planet planet_t;

trash_t* init_trash(int n, int width, int height);
planet_t* init_planets(int n, int width, int height);


#endif
