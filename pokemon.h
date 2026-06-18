#ifndef POKEMON_H
#define POKEMON_H

#include <string>
#include "csv_parser.h"

class npc;
class character;

struct LearnedMove {
    int move_id;
    std::string name;
    int power;
    int accuracy;
    int priority;
    int type_id;

    LearnedMove() : move_id(-1), power(0), accuracy(0), priority(0), type_id(-1) {}
};

struct PokemonInstance {
    int pokemon_id;
    int species_id;
    std::string name;
    int level;
    char gender;
    int shiny;

    int iv_hp;
    int iv_attack;
    int iv_defense;
    int iv_speed;
    int iv_sp_attack;
    int iv_sp_defense;

    int hp;
    int max_hp;
    int attack;
    int defense;
    int speed;
    int sp_attack;
    int sp_defense;
    int base_speed;

    int type_ids[2];
    int num_types;

    LearnedMove moves[2];
    int num_moves;

    PokemonInstance()
        : pokemon_id(-1), species_id(-1), level(1), gender('M'), shiny(0),
          iv_hp(0), iv_attack(0), iv_defense(0), iv_speed(0),
          iv_sp_attack(0), iv_sp_defense(0),
          hp(0), max_hp(0), attack(0), defense(0), speed(0),
          sp_attack(0), sp_defense(0), base_speed(0),
          num_types(0), num_moves(0) {
        type_ids[0] = type_ids[1] = -1;
    }
};

int world_distance_from_center(int world_x, int world_y);
int random_pokemon_level_for_world(int world_x, int world_y);
bool generate_random_pokemon_for_world(const PokedexData &db, int world_x, int world_y, PokemonInstance *out);
void generate_npc_party(npc *trainer, const PokedexData &db, int world_x, int world_y);

bool pokemon_is_knocked_out(const PokemonInstance &p);
bool pokemon_has_type(const PokemonInstance &p, int type_id);
void fully_heal_pokemon(PokemonInstance *p);
void fully_heal_party(character *c);
int first_usable_pokemon_index(const character *c);
bool party_has_usable_pokemon(const character *c);
bool use_potion_on_pokemon(PokemonInstance *p);
bool use_revive_on_pokemon(PokemonInstance *p);

#endif