#ifndef MAPGEN_H
#define MAPGEN_H

#include "queue.h"
#include "trainer.h"
#include "turnpq.h"

#define MAP_W 80
#define MAP_H 21

#define BOULDER '%'
#define MOUNTAIN '&'
#define PATH '#'
#define POKEMON_CENTER 'C'
#define POKEMART 'M'
#define TALL_GRASS ':'
#define CLEARING '.'
#define WATER '~'
#define FOREST '^'
#define PC '@'

class map {
public:
    char grid[MAP_H][MAP_W];
    int direction[4];
    char character_map[MAP_H][MAP_W];
    int hiker_dist[MAP_H][MAP_W];
    int rival_dist[MAP_H][MAP_W];

    npc trainers[MAX_TRAINERS];
    int num_trainers;
    int trainers_initialized;

    turnpq pq;
    int pq_initialized;

    map() : num_trainers(0), trainers_initialized(0), pq_initialized(0) {
        for (int i = 0; i < 4; i++) {
            direction[i] = -2;
        }
    }
};

void init_map(map *c);
void print_map(map *c);
void terrain_growth(map *c);
void path_generation(map *c);
int place_buildings(map *c, point_t world_coords);
point_t player_spawn(map *c);

#endif