#include <cstdlib>
#include <cstdio>
#include <climits>
#include "mapgen.h"
#include "heap.h"
#include "pathfind.h"

static int in_bounds(int y, int x);
static void dijkstra_all(map *c, point_t pc, int dist[MAP_H][MAP_W], char trainer_type);
static int cost_to_enter_hiker(map *c, int x, int y);
static int cost_to_enter_rival(map *c, int x, int y);
static int cost_to_enter_pc(map *c, int x, int y);
static int is_exit_cell(map *m, int y, int x);

// 8 direction movement
static const int dirs8[8][2] = {
  {-1,-1},{-1,0},{-1,1},
  { 0,-1},       { 0,1},
  { 1,-1},{ 1,0},{ 1,1}
};

// shared cost function used by spawning + scheduler
int movement_cost(map *m, int x, int y, char who) {
  // nobody can go into exits
  if (is_exit_cell(m, y, x)) return INT_MAX;

  if (who == '@') {
    return cost_to_enter_pc(m, x, y);
  } else if (who == 'h') {
    return cost_to_enter_hiker(m, x, y);
  } else if (who == 'r' || who == 'p' || who == 'w' || who == 's' || who == 'e') {
    return cost_to_enter_rival(m, x, y);
  } else {
    return INT_MAX;
  }
}

void compute_distance_maps(map *m, point_t pc_position) {
  dijkstra_all(m, pc_position, m->hiker_dist, 'h');
  dijkstra_all(m, pc_position, m->rival_dist, 'r');
}

// compute dist grid using heap
static void dijkstra_all(map *c, point_t pc, int dist[MAP_H][MAP_W], char trainer_type) {
  int y, x, cy, cx, d, ny, nx, i, w, cand;
  heap h;
  heap_item *node;
  point_t np;

  for (y = 0; y < MAP_H; y++) {
    for (x = 0; x < MAP_W; x++) {
      dist[y][x] = INT_MAX;
    }
  }

  dist[pc.y][pc.x] = 0;

  heap_init(&h);
  heap_insert(&h, pc, 0);

  while (h.size > 0) {
    node = heap_extract_min(&h);
    if (!node) break;

    cy = node->cell.y;
    cx = node->cell.x;
    d  = node->distance;

    if (d != dist[cy][cx]) {
      delete node;
      continue;
    }

    for (i = 0; i < 8; i++) {
      ny = cy + dirs8[i][0];
      nx = cx + dirs8[i][1];
      if (!in_bounds(ny, nx)) continue;

      w = movement_cost(c, nx, ny, trainer_type);
      if (w == INT_MAX) continue;

      if (d > INT_MAX - w) continue;
      cand = d + w;

      if (cand < dist[ny][nx]) {
        dist[ny][nx] = cand;
        np.x = nx;
        np.y = ny;
        heap_insert(&h, np, cand);
      }
    }

    delete node;
  }

  heap_destroy(&h);
}

// cost function for hiker
static int cost_to_enter_hiker(map *c, int x, int y) {
  if (c->grid[y][x] == CLEARING ||
      (c->grid[y][x] == PATH && (y > 0 && y < MAP_H - 1 && x > 0 && x < MAP_W - 1))) {
    return 10;
  } else if (c->grid[y][x] == MOUNTAIN || c->grid[y][x] == FOREST || c->grid[y][x] == TALL_GRASS) {
    return 15;
  } else if (c->grid[y][x] == POKEMART || c->grid[y][x] == POKEMON_CENTER) {
    return 50;
  } else {
    return INT_MAX;
  }
}

// cost function for rival, pacer, wanderer, explorer
static int cost_to_enter_rival(map *c, int x, int y) {
  if (c->grid[y][x] == CLEARING ||
      (c->grid[y][x] == PATH && (y > 0 && y < MAP_H - 1 && x > 0 && x < MAP_W - 1))) {
    return 10;
  } else if (c->grid[y][x] == TALL_GRASS) {
    return 20;
  } else if (c->grid[y][x] == POKEMART || c->grid[y][x] == POKEMON_CENTER) {
    return 50;
  } else {
    return INT_MAX;
  }
}

// cost function for PC
static int cost_to_enter_pc(map *c, int x, int y) {
  if (c->grid[y][x] == CLEARING ||
      (c->grid[y][x] == PATH && (y > 0 && y < MAP_H - 1 && x > 0 && x < MAP_W - 1))) {
    return 10;
  } else if (c->grid[y][x] == TALL_GRASS) {
    return 20;
  } else if (c->grid[y][x] == POKEMART || c->grid[y][x] == POKEMON_CENTER) {
    return 10;
  } else {
    return INT_MAX;
  }
}

static int in_bounds(int y, int x) {
  return (y >= 0 && y < MAP_H && x >= 0 && x < MAP_W);
}

static int is_exit_cell(map *m, int y, int x) {
  return ((y == 0 || y == MAP_H - 1 || x == 0 || x == MAP_W - 1) &&
          (m->grid[y][x] == PATH));
}