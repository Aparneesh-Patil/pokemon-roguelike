#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <cstdio>
#include "mapgen.h"
#include "trainer.h"
#include "queue.h"
#include "pokemon.h"

#define MSG_LEN 128

class world;

enum {
    CP_DEFAULT = 1,
    CP_PATH,
    CP_CENTER,
    CP_MART,
    CP_GRASS,
    CP_CLEAR,
    CP_WATER,
    CP_FOREST,
    CP_BOULDER,
    CP_MOUNTAIN,
    CP_PC,
    CP_TRAINER,
    CP_STATUS
};

enum pc_move_result {
    PC_MOVE_BLOCKED = 0,
    PC_MOVE_OK = 1,
    PC_MOVE_BATTLE = 2,
    PC_MOVE_GATE_W = 3,
    PC_MOVE_GATE_E = 4,
    PC_MOVE_GATE_N = 5,
    PC_MOVE_GATE_S = 6,
    PC_MOVE_WILD = 7
};

int take_turn(map *m, character *c, point_t *pc_pos);
int attempt_pc_move(world *w, map *m, character *pc, int dy, int dx, int *battle_index, PokemonInstance *wild_out, char *msg, size_t msg_len);

#endif