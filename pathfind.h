#ifndef PATHFIND_H
#define PATHFIND_H
#include "mapgen.h"
#include "queue.h"

void compute_distance_maps(map *m, point_t pc_position);
int movement_cost(map *m, int x, int y, char who);

#endif