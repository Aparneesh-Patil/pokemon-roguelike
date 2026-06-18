#include <climits>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <random>
#include "pokemon.h"
#include "trainer.h"
#include "worldgen.h"

static int clamp_int(int x, int lo, int hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

int world_distance_from_center(int world_x, int world_y) {
    return std::abs(world_x) + std::abs(world_y);
}

int random_pokemon_level_for_world(int world_x, int world_y) {
    int dist = world_distance_from_center(world_x, world_y);

    int min_level = 1;
    int max_level = 100;

    if (dist <= 200) {
        min_level = 1;
        max_level = dist / 2;
        if (max_level < 1) max_level = 1;
    } else {
        min_level = (dist - 200) / 2;
        max_level = 100;
        if (min_level < 1) min_level = 1;
    }

    min_level = clamp_int(min_level, 1, 100);
    max_level = clamp_int(max_level, 1, 100);

    if (min_level > max_level) {
        int tmp = min_level;
        min_level = max_level;
        max_level = tmp;
    }

    return min_level + (std::rand() % (max_level - min_level + 1));
}

static const PokemonRow *random_pokemon_row(const PokedexData &db) {
    if (db.pokemon.empty()) return nullptr;
    size_t idx = static_cast<size_t>(std::rand() % db.pokemon.size());
    return &db.pokemon[idx];
}

static void fill_base_stats(const PokedexData &db, int pokemon_id,
                            int *base_hp, int *base_attack, int *base_defense,
                            int *base_sp_attack, int *base_sp_defense, int *base_speed) {
    *base_hp = *base_attack = *base_defense = *base_sp_attack = *base_sp_defense = *base_speed = 1;

    for (size_t i = 0; i < db.pokemon_stats.size(); i++) {
        const PokemonStatsRow &row = db.pokemon_stats[i];
        if (row.pokemon_id != pokemon_id) continue;
        if (row.base_stat == INT_MAX || row.stat_id == INT_MAX) continue;

        switch (row.stat_id) {
            case 1: *base_hp = row.base_stat; break;
            case 2: *base_attack = row.base_stat; break;
            case 3: *base_defense = row.base_stat; break;
            case 4: *base_sp_attack = row.base_stat; break;
            case 5: *base_sp_defense = row.base_stat; break;
            case 6: *base_speed = row.base_stat; break;
            default: break;
        }
    }
}

static void load_pokemon_types(const PokedexData &db, int pokemon_id, PokemonInstance *out) {
    out->num_types = 0;
    out->type_ids[0] = out->type_ids[1] = -1;

    for (size_t i = 0; i < db.pokemon_types.size() && out->num_types < 2; i++) {
        const PokemonTypesRow &r = db.pokemon_types[i];
        if (r.pokemon_id != pokemon_id) continue;
        out->type_ids[out->num_types++] = r.type_id;
    }
}

static int calc_hp(int base, int iv, int level) {
    return (((base + iv) * 2) * level) / 100 + level + 10;
}

static int calc_other_stat(int base, int iv, int level) {
    return (((base + iv) * 2) * level) / 100 + 5;
}

static void fill_ivs(PokemonInstance *p) {
    p->iv_hp = std::rand() % 16;
    p->iv_attack = std::rand() % 16;
    p->iv_defense = std::rand() % 16;
    p->iv_speed = std::rand() % 16;
    p->iv_sp_attack = std::rand() % 16;
    p->iv_sp_defense = std::rand() % 16;
}

static std::vector<int> legal_moves_for_pokemon_level(const PokedexData &db, int pokemon_id, int level) {
    std::vector<int> ids;
    for (size_t i = 0; i < db.pokemon_moves.size(); i++) {
        const PokemonMovesRow &row = db.pokemon_moves[i];
        if (row.pokemon_id != pokemon_id) continue;
        if (row.level == INT_MAX || row.move_id == INT_MAX) continue;
        if (row.level <= level) {
            ids.push_back(row.move_id);
        }
    }
    return ids;
}

static const MovesRow *find_move_row(const PokedexData &db, int move_id) {
    for (size_t i = 0; i < db.moves.size(); i++) {
        if (db.moves[i].id == move_id) return &db.moves[i];
    }
    return nullptr;
}

static void fill_struggle(PokemonInstance *p) {
    p->moves[0].move_id = -1;
    p->moves[0].name = "struggle";
    p->moves[0].power = 50;
    p->moves[0].accuracy = 100;
    p->moves[0].priority = 0;
    p->moves[0].type_id = -1;
    p->num_moves = 1;
}

static void choose_up_to_two_moves(const PokedexData &db, const std::vector<int> &legal_ids, PokemonInstance *p) {
    p->num_moves = 0;

    if (legal_ids.empty()) {
        fill_struggle(p);
        return;
    }

    std::vector<int> unique_ids = legal_ids;
    std::sort(unique_ids.begin(), unique_ids.end());
    unique_ids.erase(std::unique(unique_ids.begin(), unique_ids.end()), unique_ids.end());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(unique_ids.begin(), unique_ids.end(), gen);

    for (size_t i = 0; i < unique_ids.size() && p->num_moves < 2; i++) {
        const MovesRow *m = find_move_row(db, unique_ids[i]);
        if (!m) continue;

        p->moves[p->num_moves].move_id = m->id;
        p->moves[p->num_moves].name = m->identifier;
        p->moves[p->num_moves].power = (m->power == INT_MAX) ? 0 : m->power;
        p->moves[p->num_moves].accuracy = (m->accuracy == INT_MAX) ? 100 : m->accuracy;
        p->moves[p->num_moves].priority = (m->priority == INT_MAX) ? 0 : m->priority;
        p->moves[p->num_moves].type_id = (m->type_id == INT_MAX) ? -1 : m->type_id;
        p->num_moves++;
    }

    if (p->num_moves == 0) {
        fill_struggle(p);
    }
}

bool generate_random_pokemon_for_world(const PokedexData &db, int world_x, int world_y, PokemonInstance *out) {
    if (!out) return false;

    const PokemonRow *row = random_pokemon_row(db);
    if (!row) return false;

    out->pokemon_id = row->id;
    out->species_id = row->species_id;
    out->name = row->identifier;
    out->level = random_pokemon_level_for_world(world_x, world_y);
    out->gender = (std::rand() & 1) ? 'M' : 'F';
    out->shiny = ((std::rand() % 8192) == 0) ? 1 : 0;

    fill_ivs(out);

    int base_hp, base_attack, base_defense, base_sp_attack, base_sp_defense, base_speed;
    fill_base_stats(db, out->pokemon_id,
                   &base_hp, &base_attack, &base_defense,
                   &base_sp_attack, &base_sp_defense, &base_speed);

    out->max_hp = calc_hp(base_hp, out->iv_hp, out->level);
    out->hp = out->max_hp;
    out->attack = calc_other_stat(base_attack, out->iv_attack, out->level);
    out->defense = calc_other_stat(base_defense, out->iv_defense, out->level);
    out->sp_attack = calc_other_stat(base_sp_attack, out->iv_sp_attack, out->level);
    out->sp_defense = calc_other_stat(base_sp_defense, out->iv_sp_defense, out->level);
    out->speed = calc_other_stat(base_speed, out->iv_speed, out->level);
    out->base_speed = base_speed;

    load_pokemon_types(db, out->pokemon_id, out);

    std::vector<int> legal = legal_moves_for_pokemon_level(db, out->pokemon_id, out->level);
    choose_up_to_two_moves(db, legal, out);

    return true;
}

void generate_npc_party(npc *trainer, const PokedexData &db, int world_x, int world_y) {
    if (!trainer) return;
    trainer->party_size = 0;

    int count = 1 + (std::rand() % 6);
    for (int i = 0; i < count && trainer->party_size < 6; i++) {
        generate_random_pokemon_for_world(db, world_x, world_y, &trainer->party[trainer->party_size]);
        trainer->party_size++;
    }
}

bool pokemon_is_knocked_out(const PokemonInstance &p) {
    return p.hp <= 0;
}

bool pokemon_has_type(const PokemonInstance &p, int type_id) {
    for (int i = 0; i < p.num_types; i++) {
        if (p.type_ids[i] == type_id) return true;
    }
    return false;
}

void fully_heal_pokemon(PokemonInstance *p) {
    if (!p) return;
    p->hp = p->max_hp;
}

void fully_heal_party(character *c) {
    if (!c) return;
    for (int i = 0; i < c->party_size; i++) {
        fully_heal_pokemon(&c->party[i]);
    }
}

int first_usable_pokemon_index(const character *c) {
    if (!c) return -1;
    for (int i = 0; i < c->party_size; i++) {
        if (!pokemon_is_knocked_out(c->party[i])) return i;
    }
    return -1;
}

bool party_has_usable_pokemon(const character *c) {
    return first_usable_pokemon_index(c) >= 0;
}

bool use_potion_on_pokemon(PokemonInstance *p) {
    if (!p || pokemon_is_knocked_out(*p) || p->hp >= p->max_hp) return false;
    p->hp += 20;
    if (p->hp > p->max_hp) p->hp = p->max_hp;
    return true;
}

bool use_revive_on_pokemon(PokemonInstance *p) {
    if (!p || !pokemon_is_knocked_out(*p)) return false;
    p->hp = p->max_hp / 2;
    if (p->hp < 1) p->hp = 1;
    return true;
}