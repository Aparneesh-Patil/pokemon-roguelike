#include <cstdlib>
#include <climits>
#include <cstdio>
#include <cstring>
#include "movement.h"
#include "pathfind.h"
#include "worldgen.h"
#include "pokemon.h"

static const int dirs8[8][2] = {
  {-1,-1},{-1,0},{-1,1},
  { 0,-1},       { 0,1},
  { 1,-1},{ 1,0},{ 1,1}
};

static int in_bounds(int y, int x);
static int is_border(int y, int x);
static int is_exit_cell(map *m, int y, int x);
static int cell_occupied(map *m, int y, int x);
static int legal_destination(map *m, int y, int x, char who);
static void apply_move(map *m, character *c, int ny, int nx);
static int rand_dir8(void);
static int move_by_gradient(map *m, character *c, int dist[MAP_H][MAP_W], point_t *pc_pos);
static int opposite_dir8(int d);
static int move_pacer(map *m, character *c, point_t *pc_pos);
static int move_wanderer(map *m, character *c, point_t *pc_pos);
static int move_explorer(map *m, character *c, point_t *pc_pos);
static int trainer_can_enter_pc(character *c);

// pc movement
int attempt_pc_move(world *w, map *m, character *pc, int dy, int dx, int *battle_index, PokemonInstance *wild_out, char *msg, size_t msg_len)
{
  int ny = pc->pos.y + dy;
  int nx = pc->pos.x + dx;
  int i;
  char dest_tile;

  if (battle_index) {
    *battle_index = -1;
  }

  if (!in_bounds(ny, nx)) {
    std::snprintf(msg, msg_len, "You cannot move there.");
    return PC_MOVE_BLOCKED;
  }

  if (is_exit_cell(m, ny, nx)) {
    if (nx == 0) {
      std::snprintf(msg, msg_len, "You travel west.");
      return PC_MOVE_GATE_W;
    }
    if (nx == MAP_W - 1) {
      std::snprintf(msg, msg_len, "You travel east.");
      return PC_MOVE_GATE_E;
    }
    if (ny == 0) {
      std::snprintf(msg, msg_len, "You travel north.");
      return PC_MOVE_GATE_N;
    }
    if (ny == MAP_H - 1) {
      std::snprintf(msg, msg_len, "You travel south.");
      return PC_MOVE_GATE_S;
    }
  }

  if (is_border(ny, nx)) {
    std::snprintf(msg, msg_len, "You cannot move there.");
    return PC_MOVE_BLOCKED;
  }

  if (movement_cost(m, nx, ny, pc->type) == INT_MAX) {
    std::snprintf(msg, msg_len, "There's something in the way.");
    return PC_MOVE_BLOCKED;
  }

  for (i = 0; i < m->num_trainers; i++) {
    if (m->trainers[i].pos.y == ny && m->trainers[i].pos.x == nx) {
      if (m->trainers[i].defeated) {
        std::snprintf(msg, msg_len, "You already defeated that trainer.");
        return PC_MOVE_BLOCKED;
      }

      if (battle_index) {
        *battle_index = i;
      }

      std::snprintf(msg, msg_len, "A battle begins with %c!", m->trainers[i].type);
      return PC_MOVE_BATTLE;
    }
  }

  dest_tile = m->grid[ny][nx];
  apply_move(m, pc, ny, nx);

  if (dest_tile == TALL_GRASS && w && w->db) {
    if ((std::rand() % 100) < 10) {
      if (wild_out) {
        generate_random_pokemon_for_world(*w->db, w->coords.x - 200, w->coords.y- 200, wild_out);
      }
      std::snprintf(msg, msg_len, "A wild Pokemon appeared!");
      return PC_MOVE_WILD;
    }
  }

  std::snprintf(msg, msg_len, "PC moved.");
  return PC_MOVE_OK;
}

// trainer turn logic
int take_turn(map *m, character *c, point_t *pc_pos) {
  if (!c || c->defeated) {
    return 0;
  }

  if (c->type == 'h') return move_by_gradient(m, c, m->hiker_dist, pc_pos);
  if (c->type == 'r') return move_by_gradient(m, c, m->rival_dist, pc_pos);
  if (c->type == 'p') return move_pacer(m, c, pc_pos);
  if (c->type == 'w') return move_wanderer(m, c, pc_pos);
  if (c->type == 'e') return move_explorer(m, c, pc_pos);
  if (c->type == 's') return 0;

  return 0;
}

static int in_bounds(int y, int x) {
  return (y >= 0 && y < MAP_H && x >= 0 && x < MAP_W);
}

static int is_border(int y, int x) {
  return (y == 0 || y == MAP_H - 1 || x == 0 || x == MAP_W - 1);
}

static int is_exit_cell(map *m, int y, int x) {
  return is_border(y, x) && (m->grid[y][x] == PATH);
}

static int cell_occupied(map *m, int y, int x) {
  return (m->character_map[y][x] != ' ');
}

static int legal_destination(map *m, int y, int x, char who) {
  if (!in_bounds(y, x)) return 0;
  if (is_border(y, x)) return 0;
  if (is_exit_cell(m, y, x)) return 0;
  if (cell_occupied(m, y, x)) return 0;
  if (movement_cost(m, x, y, who) == INT_MAX) return 0;
  return 1;
}

static void apply_move(map *m, character *c, int ny, int nx) {
  m->character_map[c->pos.y][c->pos.x] = ' ';
  c->pos.y = ny;
  c->pos.x = nx;
  m->character_map[ny][nx] = c->type;
}

static int rand_dir8(void) {
  return std::rand() % 8;
}

static int trainer_can_enter_pc(character *c) {
  return c->type == 'h' || c->type == 'r' || c->type == 'p' || c->type == 'w' || c->type == 'e';
}

static int move_by_gradient(map *m, character *c, int dist[MAP_H][MAP_W], point_t *pc_pos) {
  int i, ny, nx, best_i = -1, best_d = INT_MAX;

  for (i = 0; i < 8; i++) {
    ny = c->pos.y + dirs8[i][0];
    nx = c->pos.x + dirs8[i][1];

    if (pc_pos && ny == pc_pos->y && nx == pc_pos->x && trainer_can_enter_pc(c)) {
      return 2;
    }

    if (!legal_destination(m, ny, nx, c->type)) continue;

    if (dist[ny][nx] < best_d) {
      best_d = dist[ny][nx];
      best_i = i;
    } else if (dist[ny][nx] == best_d && best_i != -1) {
      if ((std::rand() & 1) == 0) best_i = i;
    }
  }

  if (best_i == -1 || best_d == INT_MAX) return 0;

  ny = c->pos.y + dirs8[best_i][0];
  nx = c->pos.x + dirs8[best_i][1];
  apply_move(m, c, ny, nx);
  return 1;
}

static int opposite_dir8(int d) {
  static const int opp[8] = {7,6,5,4,3,2,1,0};
  if (d < 0 || d > 7) return 6;
  return opp[d];
}

static int move_pacer(map *m, character *c, point_t *pc_pos) {
  int d = c->dir, ny, nx;
  if (d < 0 || d > 7) d = 6;

  ny = c->pos.y + dirs8[d][0];
  nx = c->pos.x + dirs8[d][1];

  if (pc_pos && ny == pc_pos->y && nx == pc_pos->x && trainer_can_enter_pc(c)) {
    return 2;
  }

  if (!legal_destination(m, ny, nx, c->type)) {
    d = opposite_dir8(d);
    c->dir = d;

    ny = c->pos.y + dirs8[d][0];
    nx = c->pos.x + dirs8[d][1];

    if (pc_pos && ny == pc_pos->y && nx == pc_pos->x && trainer_can_enter_pc(c)) {
      return 2;
    }

    if (!legal_destination(m, ny, nx, c->type)) return 0;
  }

  apply_move(m, c, ny, nx);
  return 1;
}

static int move_wanderer(map *m, character *c, point_t *pc_pos) {
  int tries = 0, d, ny, nx;

  while (tries < 8) {
    d = c->dir;
    if (d < 0 || d > 7) d = rand_dir8();

    ny = c->pos.y + dirs8[d][0];
    nx = c->pos.x + dirs8[d][1];

    if (pc_pos && ny == pc_pos->y && nx == pc_pos->x && trainer_can_enter_pc(c)) {
      return 2;
    }

    if (legal_destination(m, ny, nx, c->type) &&
        m->grid[ny][nx] == c->spawn_terrain) {
      apply_move(m, c, ny, nx);
      c->dir = d;
      return 1;
    }

    c->dir = rand_dir8();
    tries++;
  }

  return 0;
}

static int move_explorer(map *m, character *c, point_t *pc_pos) {
  int d = c->dir, ny, nx;
  if (d < 0 || d > 7) d = rand_dir8();

  ny = c->pos.y + dirs8[d][0];
  nx = c->pos.x + dirs8[d][1];

  if (pc_pos && ny == pc_pos->y && nx == pc_pos->x && trainer_can_enter_pc(c)) {
    return 2;
  }

  if (!legal_destination(m, ny, nx, c->type)) {
    c->dir = rand_dir8();
    return 0;
  }

  apply_move(m, c, ny, nx);
  c->dir = d;
  return 1;
}