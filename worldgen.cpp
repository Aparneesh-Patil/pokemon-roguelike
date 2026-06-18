#include <cstdlib>
#include <cstdio>
#include "worldgen.h"
#include "mapgen.h"
#include "trainer.h"
#include "turnpq.h"
#include "movement.h"
#include "pokemon.h"

static int in_world_bounds(point_t p) {
    return p.x >= 0 && p.x < WORLD_W_H && p.y >= 0 && p.y < WORLD_W_H;
}

static point_t fallback_open_road(map *m) {
    int y, x;
    point_t p = player_spawn(m);

    if (m->character_map[p.y][p.x] == ' ') {
        return p;
    }

    for (y = 1; y < MAP_H - 1; y++) {
        for (x = 1; x < MAP_W - 1; x++) {
            if (m->grid[y][x] == PATH && m->character_map[y][x] == ' ') {
                p.x = x;
                p.y = y;
                return p;
            }
        }
    }

    for (y = 1; y < MAP_H - 1; y++) {
        for (x = 1; x < MAP_W - 1; x++) {
            if (m->character_map[y][x] == ' ') {
                p.x = x;
                p.y = y;
                return p;
            }
        }
    }

    return p;
}

static int initialize_map_runtime(world *w, map *m) {
    int i;
    point_t blocked;

    m->num_trainers = 0;
    m->trainers_initialized = 0;
    m->pq_initialized = 0;

    if (turnpq_init(&m->pq, MAX_TRAINERS + 4) != 0) {
        return -1;
    }
    m->pq_initialized = 1;

    blocked = player_spawn(m);

    m->num_trainers = spawn_trainers(m, m->trainers, w->default_num_trainers, blocked);
    if (m->num_trainers < 0) {
        turnpq_destroy(&m->pq);
        m->pq_initialized = 0;
        return -1;
    }

    if (w->db) {
        for (i = 0; i < m->num_trainers; i++) {
            generate_npc_party(&m->trainers[i], *w->db, w->coords.x - 200, w->coords.y - 200);;
        }
    }

    for (i = 0; i < m->num_trainers; i++) {
        turnpq_push(&m->pq, &m->trainers[i]);
    }

    m->trainers_initialized = 1;
    return 0;
}

static point_t gate_entry_position(map *m, int move_result) {
    point_t p;

    if (move_result == PC_MOVE_GATE_W) {
        p.x = MAP_W - 2;
        p.y = m->direction[0];
    } else if (move_result == PC_MOVE_GATE_E) {
        p.x = 1;
        p.y = m->direction[1];
    } else if (move_result == PC_MOVE_GATE_N) {
        p.x = m->direction[3];
        p.y = MAP_H - 2;
    } else {
        p.x = m->direction[2];
        p.y = 1;
    }

    if (p.x < 1 || p.x > MAP_W - 2 || p.y < 1 || p.y > MAP_H - 2 ||
        m->character_map[p.y][p.x] != ' ') {
        p = fallback_open_road(m);
    }

    return p;
}

void gate_creation(world *dungeon, point_t target) {
    int k, nx, ny;
    int x = target.x, y = target.y;
    int dirs_world[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };

    if (x == 0)   dungeon->curr[x][y]->direction[1] = -1;
    if (x == 400) dungeon->curr[x][y]->direction[0] = -1;
    if (y == 0)   dungeon->curr[x][y]->direction[2] = -1;
    if (y == 400) dungeon->curr[x][y]->direction[3] = -1;

    for (k = 0; k < 4; k++) {
        nx = x + dirs_world[k][1];
        ny = y + dirs_world[k][0];

        if (nx < 0 || nx > 400 || ny < 0 || ny > 400) continue;

        if (k == 2 && dungeon->curr[x][y]->direction[1] != -1) {
            if (dungeon->curr[nx][ny] != nullptr) {
                dungeon->curr[x][y]->direction[1] = dungeon->curr[nx][ny]->direction[0];
            } else {
                dungeon->curr[x][y]->direction[1] = 1 + (std::rand() % (MAP_H - 2));
            }
        } else if (k == 3 && dungeon->curr[x][y]->direction[0] != -1) {
            if (dungeon->curr[nx][ny] != nullptr) {
                dungeon->curr[x][y]->direction[0] = dungeon->curr[nx][ny]->direction[1];
            } else {
                dungeon->curr[x][y]->direction[0] = 1 + (std::rand() % (MAP_H - 2));
            }
        } else if (k == 0 && dungeon->curr[x][y]->direction[2] != -1) {
            if (dungeon->curr[nx][ny] != nullptr) {
                dungeon->curr[x][y]->direction[2] = dungeon->curr[nx][ny]->direction[3];
            } else {
                dungeon->curr[x][y]->direction[2] = 1 + (std::rand() % (MAP_W - 2));
            }
        } else {
            if (dungeon->curr[nx][ny] != nullptr && dungeon->curr[x][y]->direction[3] != -1) {
                dungeon->curr[x][y]->direction[3] = dungeon->curr[nx][ny]->direction[2];
            } else if (dungeon->curr[x][y]->direction[3] != -1) {
                dungeon->curr[x][y]->direction[3] = 1 + (std::rand() % (MAP_W - 2));
            }
        }
    }
}

map *visit_map(world *w, point_t target) {
    int i;
    map *m;
    int x = target.x, y = target.y;

    if (!in_world_bounds(target)) {
        return nullptr;
    }

    if (w->curr[x][y] == nullptr) {
        m = new map();

        w->curr[x][y] = m;

        for (i = 0; i < 4; i++) {
            m->direction[i] = -2;
        }

        gate_creation(w, target);
        init_map(m);
        place_buildings(m, target);

        if (initialize_map_runtime(w, m) != 0) {
            delete m;
            w->curr[x][y] = nullptr;
            return nullptr;
        }
    }

    return w->curr[x][y];
}

map *current_map(world *w) {
    return w->curr[w->coords.x][w->coords.y];
}

void clear_pc_from_map(map *m, point_t pos) {
    if (!m) return;
    if (pos.x < 0 || pos.x >= MAP_W || pos.y < 0 || pos.y >= MAP_H) return;
    if (m->character_map[pos.y][pos.x] == PC) {
        m->character_map[pos.y][pos.x] = ' ';
    }
}

void place_pc_on_map(map *m, pc *player, point_t pos) {
    if (!m || !player) return;
    player->pos = pos;
    m->character_map[pos.y][pos.x] = PC;
}

int move_pc_to_neighbor(world *w, pc *player, int move_result) {
    point_t next = w->coords;
    map *old_map = current_map(w);
    map *new_map;
    point_t pos;

    if (move_result == PC_MOVE_GATE_W) next.x--;
    else if (move_result == PC_MOVE_GATE_E) next.x++;
    else if (move_result == PC_MOVE_GATE_N) next.y--;
    else if (move_result == PC_MOVE_GATE_S) next.y++;
    else return 0;

    if (!in_world_bounds(next)) {
        return 0;
    }

    clear_pc_from_map(old_map, player->pos);

    w->coords = next;
    new_map = visit_map(w, w->coords);
    if (!new_map) {
        return 0;
    }

    pos = gate_entry_position(new_map, move_result);
    place_pc_on_map(new_map, player, pos);

    {
        character *top = turnpq_peek(&new_map->pq);
        player->next_time = top ? top->next_time : 0;
    }

    return 1;
}

int fly_pc_to(world *w, pc *player, int world_x, int world_y) {
    point_t target;
    map *old_map = current_map(w);
    map *new_map;
    point_t pos;

    if (world_x < -200 || world_x > 200 || world_y < -200 || world_y > 200) {
        return 0;
    }

    target.x = INIT_COORD + world_x;
    target.y = INIT_COORD + world_y;

    if (!in_world_bounds(target)) {
        return 0;
    }

    clear_pc_from_map(old_map, player->pos);

    w->coords = target;
    new_map = visit_map(w, w->coords);
    if (!new_map) {
        return 0;
    }

    pos = fallback_open_road(new_map);
    place_pc_on_map(new_map, player, pos);

    {
        character *top = turnpq_peek(&new_map->pq);
        player->next_time = top ? top->next_time : 0;
    }

    return 1;
}

void destroy_world(world *w) {
    int i, j;
    if (!w) return;

    for (i = 0; i < WORLD_W_H; i++) {
        for (j = 0; j < WORLD_W_H; j++) {
            if (w->curr[i][j]) {
                if (w->curr[i][j]->pq_initialized) {
                    turnpq_destroy(&w->curr[i][j]->pq);
                }
                delete w->curr[i][j];
            }
        }
    }

    delete w;
}