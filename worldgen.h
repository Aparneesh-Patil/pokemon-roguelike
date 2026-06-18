#ifndef WORLDGEN_H
#define WORLDGEN_H

#include "queue.h"
#include "mapgen.h"
#include "trainer.h"
#include "csv_parser.h"

#define WORLD_W_H 401
#define INIT_COORD 200

class world {
public:
    map *curr[WORLD_W_H][WORLD_W_H];
    point_t coords;
    int default_num_trainers;
    PokedexData *db;

    world() : default_num_trainers(10), db(nullptr) {
        for (int i = 0; i < WORLD_W_H; i++) {
            for (int j = 0; j < WORLD_W_H; j++) {
                curr[i][j] = nullptr;
            }
        }
        coords.x = INIT_COORD;
        coords.y = INIT_COORD;
    }
};

void gate_creation(world *w, point_t target);
map *visit_map(world *w, point_t target);
map *current_map(world *w);

void clear_pc_from_map(map *m, point_t pos);
void place_pc_on_map(map *m, pc *player, point_t pos);

int move_pc_to_neighbor(world *w, pc *player, int move_result);
int fly_pc_to(world *w, pc *player, int world_x, int world_y);

void destroy_world(world *w);

#endif