#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <climits>
#include "worldgen.h"
#include "queue.h"
#include "mapgen.h"

// helper methods
static void dijkstra(map *c, point_t from, point_t to);
static int cost_to_enter(map *c, int x, int y);
static int can_place_2x2_building(map *c, int y, int x);
static int in_bounds_interior(int y, int x);
static int footprint_touches_path(map *c, int y, int x);
static void stamp_2x2(map *c, int y, int x, char ch);
static int should_spawn_building(point_t world_coo);

int dirs[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

// initialize our map by first filling the borders, then placing random exits on NSEW sides
void init_map(map *c) {
    int i, j;

    // borders on the side of the map and everything the inside filled with clearing for the time being
    for (i = 0; i < MAP_H; i++) {
        for (j = 0; j < MAP_W; j++) {
            if (i == 0 || i == MAP_H - 1 || j == 0 || j == MAP_W - 1) {
                c->grid[i][j] = BOULDER;
            } else {
                c->grid[i][j] = ' ';
            }

            c->character_map[i][j] = ' ';
        }
    }

    for (i = 0; i < 4; i++) {
        if (c->direction[i] != -1) {
            if (i == 0) {
                c->grid[c->direction[0]][MAP_W - 1] = PATH;
            } else if (i == 1) {
                c->grid[c->direction[1]][0] = PATH;
            } else if (i == 2) {
                c->grid[0][c->direction[2]] = PATH;
            } else {
                c->grid[MAP_H - 1][c->direction[3]] = PATH;
            }
        }
    }

    // grows the terrain for the map using a queue
    terrain_growth(c);

    // generates a path using dijkstra's
    path_generation(c);
}

// prints the map
void print_map(map *c) {
    int i, j;

    for (i = 0; i < MAP_H; i++) {
        for (j = 0; j < MAP_W; j++) {
            if (c->character_map[i][j] != ' ') {
                std::printf("%c", c->character_map[i][j]);
            } else {
                std::printf("%c", c->grid[i][j]);
            }

            if (j == MAP_W - 1) {
                std::printf("\n");
            }
        }
    }
}

void terrain_growth(map *c) {
    // x = width, y = height
    int i, x, y, ny, nx;
    point_t t;
    queue q;

    // 14 terrain seeds
    char terrain_types[] = {
        WATER, TALL_GRASS, TALL_GRASS, CLEARING, CLEARING,
        MOUNTAIN, FOREST, TALL_GRASS, TALL_GRASS, CLEARING,
        CLEARING, WATER, FOREST, FOREST
    };

    // initialize the queue
    queue_init(&q);

    // We first add 14 random points into the queue, defining their terrains using terrain_types
    for (i = 0; i < 14; i++) {
        y = 1 + (std::rand() % (MAP_H - 2));
        x = 1 + (std::rand() % (MAP_W - 2));

        t.y = y;
        t.x = x;

        c->grid[y][x] = terrain_types[i];
        queue_push(&q, t);
    }

    // while the queue isn't empty, we pop points and make its neighbors the same terrain type as itself
    while (q.size > 0) {
        queue_pop(&q, &t);

        nx = t.x;
        ny = t.y;

        // checks the down cell
        if (ny - 1 > 0 && ny - 1 < MAP_H - 1 && nx > 0 && nx < MAP_W - 1 &&
            c->grid[ny - 1][nx] == ' ') {
            c->grid[ny - 1][nx] = c->grid[ny][nx];
            t.y = ny - 1;
            queue_push(&q, t);
            t.y = ny;
        }

        // checks the up cell
        if (ny + 1 > 0 && ny + 1 < MAP_H - 1 && nx > 0 && nx < MAP_W - 1 &&
            c->grid[ny + 1][nx] == ' ') {
            c->grid[ny + 1][nx] = c->grid[ny][nx];
            t.y = ny + 1;
            queue_push(&q, t);
            t.y = ny;
        }

        // checks the left cell
        if (ny > 0 && ny < MAP_H - 1 && nx - 1 > 0 && nx - 1 < MAP_W - 1 &&
            c->grid[ny][nx - 1] == ' ') {
            c->grid[ny][nx - 1] = c->grid[ny][nx];
            t.x = nx - 1;
            queue_push(&q, t);
            t.x = nx;
        }

        // checks the right cell
        if (ny > 0 && ny < MAP_H - 1 && nx + 1 > 0 && nx + 1 < MAP_W - 1 &&
            c->grid[ny][nx + 1] == ' ') {
            c->grid[ny][nx + 1] = c->grid[ny][nx];
            t.x = nx + 1;
            queue_push(&q, t);
            t.x = nx;
        }
    }

    // clears the queue from memory after usage
    queue_destroy(&q);
}

void path_generation(map *c) {
    int x, y, i;
    point_t right, left, top, bottom, central;

    // First, we generate a random central point
    x = (MAP_W / 4) + std::rand() % (((3 * MAP_W) / 4) - (MAP_W / 4) + 1);
    y = (MAP_H / 4) + std::rand() % (((3 * MAP_H) / 4) - (MAP_H / 4) + 1);
    central.x = x;
    central.y = y;
    c->grid[y][x] = PATH;

    // run dijkstra's based on whether a gate exists
    for (i = 0; i < 4; i++) {
        if (c->direction[i] != -1) {
            if (i == 0) {
                right.x = MAP_W - 1;
                right.y = c->direction[i];
                dijkstra(c, right, central);
            } else if (i == 1) {
                left.x = 0;
                left.y = c->direction[i];
                dijkstra(c, left, central);
            } else if (i == 2) {
                top.x = c->direction[i];
                top.y = 0;
                dijkstra(c, top, central);
            } else {
                bottom.x = c->direction[i];
                bottom.y = MAP_H - 1;
                dijkstra(c, bottom, central);
            }
        }
    }
}

// function used to place valid marts and centers on the map
int place_buildings(map *c, point_t world_coords) {
    // chances of a center and mart spawning
    int want_center = should_spawn_building(world_coords);
    int want_mart = should_spawn_building(world_coords);

    point_t candidates[MAP_H * MAP_W];
    int count = 0;
    int i, j, idxC, cy, cx, idxM, mx, my;

    // valid places to place a center
    if (want_center) {
        for (i = 1; i < MAP_H - 2; i++) {
            for (j = 1; j < MAP_W - 2; j++) {
                if (!can_place_2x2_building(c, i, j) &&
                    !footprint_touches_path(c, i, j)) {
                    candidates[count].y = i;
                    candidates[count].x = j;
                    count++;
                }
            }
        }

        // no place on the map to place the center
        if (count == 0) return 1;

        // take one of those valid points and place a pokemon center there
        idxC = std::rand() % count;
        cy = candidates[idxC].y;
        cx = candidates[idxC].x;
        stamp_2x2(c, cy, cx, POKEMON_CENTER);
    }

    if (want_mart) {
        // valid places to place a mart
        count = 0;
        for (i = 1; i < MAP_H - 2; i++) {
            for (j = 1; j < MAP_W - 2; j++) {
                if (!can_place_2x2_building(c, i, j) &&
                    !footprint_touches_path(c, i, j)) {
                    candidates[count].y = i;
                    candidates[count].x = j;
                    count++;
                }
            }
        }

        // no place to place a mart
        if (count == 0) return 1;

        // take one of those valid points and place a mart there
        idxM = std::rand() % count;
        my = candidates[idxM].y;
        mx = candidates[idxM].x;
        stamp_2x2(c, my, mx, POKEMART);
    }

    return 0;
}

// method to randomly spawn the player in the map
point_t player_spawn(map *c) {
    int i, j, count = 0;
    point_t chosen{};
    chosen.x = -1;
    chosen.y = -1;

    for (i = 0; i < MAP_H; i++) {
        for (j = 0; j < MAP_W; j++) {
            if (c->grid[i][j] == PATH &&
                (i != 0 && i != MAP_H - 1 && j != 0 && j != MAP_W - 1)) {
                count++;
                if (std::rand() % count == 0) {
                    chosen.x = j;
                    chosen.y = i;
                }
            }
        }
    }

    return chosen;
}

static void dijkstra(map *c, point_t from, point_t to) {
    int i, j, x, k, nx, ny, w, minDist, curY, curX;
    int dist[MAP_H][MAP_W];
    int visited[MAP_H][MAP_W];
    point_t prev[MAP_H][MAP_W];
    point_t cur;

    for (i = 0; i < MAP_H; i++) {
        for (j = 0; j < MAP_W; j++) {
            dist[i][j] = INT_MAX;
            visited[i][j] = 0;
            prev[i][j].y = -1;
            prev[i][j].x = -1;
        }
    }

    dist[from.y][from.x] = 0;

    for (i = 0; i < MAP_H * MAP_W; i++) {
        minDist = INT_MAX;
        curY = -1;
        curX = -1;

        for (j = 0; j < MAP_H; j++) {
            for (x = 0; x < MAP_W; x++) {
                if (!visited[j][x] && dist[j][x] < minDist) {
                    minDist = dist[j][x];
                    curY = j;
                    curX = x;
                }
            }
        }

        if (curY == -1 || (curY == to.y && curX == to.x)) {
            // no reachable unvisited vertex remains
            break;
        }

        visited[curY][curX] = 1;

        for (k = 0; k < 4; k++) {
            ny = curY + dirs[k][0];
            nx = curX + dirs[k][1];

            // bounds
            if (ny < 0 || ny >= MAP_H || nx < 0 || nx >= MAP_W) continue;

            // border exclusion except exits
            if ((ny == 0 || ny == MAP_H - 1 || nx == 0 || nx == MAP_W - 1) &&
                c->grid[ny][nx] != PATH) {
                continue;
            }

            if (visited[ny][nx]) continue;

            w = cost_to_enter(c, nx, ny);

            if (dist[curY][curX] != INT_MAX &&
                dist[curY][curX] + w < dist[ny][nx]) {
                dist[ny][nx] = dist[curY][curX] + w;
                prev[ny][nx].y = curY;
                prev[ny][nx].x = curX;
            }
        }
    }

    // reconstruct the path
    cur = to;

    if (!(from.y == to.y && from.x == to.x)) {
        if (prev[cur.y][cur.x].y == -1 && prev[cur.y][cur.x].x == -1) {
            return;
        }

        while (!(cur.y == from.y && cur.x == from.x)) {
            c->grid[cur.y][cur.x] = PATH;
            cur = prev[cur.y][cur.x];
            if (cur.y == -1 || cur.x == -1) {
                return;
            }
        }
        c->grid[from.y][from.x] = PATH;
    }
}

// function used to calculate weights for dijkstra's
static int cost_to_enter(map *c, int x, int y) {
    if (c->grid[y][x] == TALL_GRASS || c->grid[y][x] == MOUNTAIN) {
        return 3;
    } else if (c->grid[y][x] == WATER) {
        return 10;
    } else if (c->grid[y][x] == BOULDER) {
        return 20;
    } else {
        return 1;
    }
}

// checks for places where the 2x2 building can be placed
static int can_place_2x2_building(map *c, int y, int x) {
    int dy, dx, k;
    char cell;

    // check if the point is out of bounds
    if (in_bounds_interior(y, x) ||
        in_bounds_interior(y, x + 1) ||
        in_bounds_interior(y + 1, x) ||
        in_bounds_interior(y + 1, x + 1)) {
        return 1;
    }

    // dont place a building over these
    const char bad[] = { BOULDER, WATER, PATH, POKEMON_CENTER, POKEMART };

    // check around the point to see if a valid place is available
    for (dy = 0; dy < 2; dy++) {
        for (dx = 0; dx < 2; dx++) {
            cell = c->grid[y + dy][x + dx];
            for (k = 0; k < static_cast<int>(sizeof(bad) / sizeof(bad[0])); k++) {
                if (cell == bad[k]) return 1;
            }
        }
    }

    return 0;
}

// function to check whether point is in bounds (0 is in bounds, 1 is out of bounds)
static int in_bounds_interior(int y, int x) {
    return !(y >= 1 && y <= MAP_H - 2 && x >= 1 && x <= MAP_W - 2);
}

// function to check whether the point touches the path
static int footprint_touches_path(map *c, int y, int x) {
    for (int dy = 0; dy < 2; dy++) {
        for (int dx = 0; dx < 2; dx++) {
            int cy = y + dy;
            int cx = x + dx;
            for (int k = 0; k < 4; k++) {
                int ny = cy + dirs[k][0];
                int nx = cx + dirs[k][1];
                if (ny < 0 || ny >= MAP_H || nx < 0 || nx >= MAP_W) continue;
                if (c->grid[ny][nx] == PATH) return 0;
            }
        }
    }
    return 1;
}

// places the mart or center into a 2x2 place
static void stamp_2x2(map *c, int y, int x, char ch) {
    c->grid[y][x] = ch;
    c->grid[y][x + 1] = ch;
    c->grid[y + 1][x] = ch;
    c->grid[y + 1][x + 1] = ch;
}

// calculates the manhattan distance and probability
static int should_spawn_building(point_t world_coords) {
    int px = world_coords.x - INIT_COORD;
    int py = world_coords.y - INIT_COORD;
    if (px < 0) px = -px;
    if (py < 0) py = -py;

    int d = px + py; // Manhattan distance
    if (d == 0) return 1; // always spawn at (0,0)

    int chance;
    if (d < 200) {
        chance = 50 - (45 * d) / 200;
        if (chance < 5) chance = 5;
    } else {
        chance = 5;
    }

    return (std::rand() % 100) < chance;
}