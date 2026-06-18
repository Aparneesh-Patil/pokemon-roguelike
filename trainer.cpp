#include <cstdlib>
#include <climits>
#include "trainer.h"
#include "mapgen.h"
#include "pathfind.h"

static int is_border(int y, int x) {
    return (y == 0 || y == MAP_H - 1 || x == 0 || x == MAP_W - 1);
}

static int is_exit_cell(map *m, int y, int x) {
    return is_border(y, x) && m->grid[y][x] == PATH;
}

static int valid_spawn_cell(map *m, int y, int x, char t) {
    if (is_border(y, x)) return 0;
    if (is_exit_cell(m, y, x)) return 0;
    if (m->character_map[y][x] != ' ') return 0;
    if (movement_cost(m, x, y, t) == INT_MAX) return 0;
    return 1;
}

static char random_trainer_type(void) {
    int r = std::rand() % 6;
    if (r == 0) return 'p';
    if (r == 1) return 'w';
    if (r == 2) return 's';
    if (r == 3) return 'e';
    if (r == 4) return 'h';
    return 'r';
}

int spawn_trainers(map *m, npc *out, int num, point_t blocked_pos) {
    int attempts, x, y, placed = 0;
    char t;

    int hikers = 0;
    int rivals = 0;
    int max_hikers = 2;
    int max_rivals = 2;

    for (attempts = 0; placed < num; attempts++) {
        if (attempts > 200000) return -1;

        // keep picking until valid under limits
        do {
            t = random_trainer_type();
        } while ((t == 'h' && hikers >= max_hikers) ||
                 (t == 'r' && rivals >= max_rivals));

        y = 1 + std::rand() % (MAP_H - 2);
        x = 1 + std::rand() % (MAP_W - 2);

        if (x == blocked_pos.x && y == blocked_pos.y) continue;
        if (!valid_spawn_cell(m, y, x, t)) continue;

        out[placed].id = placed + 1;
        out[placed].type = t;
        out[placed].pos.x = x;
        out[placed].pos.y = y;
        out[placed].next_time = 0;
        out[placed].dir = std::rand() % 8;
        out[placed].spawn_terrain = m->grid[y][x];
        out[placed].defeated = 0;

        m->character_map[y][x] = t;

        // update counters
        if (t == 'h') hikers++;
        if (t == 'r') rivals++;

        placed++;
    }

    return placed;
}