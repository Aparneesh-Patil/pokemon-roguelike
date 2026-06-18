#ifndef TRAINER_H
#define TRAINER_H

#include "pokemon.h"
#include "queue.h"

#define MAX_TRAINERS 64

struct map;


enum trainer_type {
    TRAINER_HIKER,
    TRAINER_RIVAL,
    TRAINER_PACER,
    TRAINER_WANDERER,
    TRAINER_SENTRY,
    TRAINER_EXPLORER
};

class character {
public:
    point_t pos;
    char type;
    int next_time;
    int id;
    int dir;
    char spawn_terrain;
    int defeated;
    PokemonInstance party[6];
    int party_size;

    character()
        : type('@'),
          next_time(0),
          id(0),
          dir(0),
          spawn_terrain('.'),
          defeated(0),
          party_size(0) {
        pos.x = 0;
        pos.y = 0;
    }

    virtual ~character() {}
};

class pc : public character {
public:
    int potions;
    int revives;
    int pokeballs;

    pc()
        : character(),
          potions(5),
          revives(3),
          pokeballs(5) {
        type = '@';
    }
};

class npc : public character {
public:
    trainer_type ai;

    npc()
        : character(),
          ai(TRAINER_HIKER) {}
};

int spawn_trainers(map *m, npc *out, int num, point_t blocked_pos);

#endif