#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>
#include <climits>

struct PokemonRow {
  int id;
  std::string identifier;
  int species_id;
  int height;
  int weight;
  int base_experience;
  int order;
  int is_default;
};

struct MovesRow {
  int id;
  std::string identifier;
  int generation_id;
  int type_id;
  int power;
  int pp;
  int accuracy;
  int priority;
  int target_id;
  int damage_class_id;
  int effect_id;
  int effect_chance;
  int contest_type_id;
  int contest_effect_id;
  int super_contest_effect_id;
};

struct PokemonMovesRow {
  int pokemon_id;
  int version_group_id;
  int move_id;
  int pokemon_move_method_id;
  int level;
  int order;
};

struct PokemonSpeciesRow {
  int id;
  std::string identifier;
  int generation_id;
  int evolves_from_species_id;
  int evolution_chain_id;
  int color_id;
  int shape_id;
  int habitat_id;
  int gender_rate;
  int capture_rate;
  int base_happiness;
  int is_baby;
  int hatch_counter;
  int has_gender_differences;
  int growth_rate_id;
  int forms_switchable;
  int is_legendary;
  int is_mythical;
  int order;
  int conquest_order;
};

struct ExperienceRow {
  int growth_rate_id;
  int level;
  int experience;
};

struct TypeNamesRow {
  int type_id;
  int local_language_id;
  std::string name;
};

struct PokemonStatsRow {
  int pokemon_id;
  int stat_id;
  int base_stat;
  int effort;
};

struct StatsRow {
  int id;
  int damage_class_id;
  std::string identifier;
  int is_battle_only;
  int game_index;
};

struct PokemonTypesRow {
  int pokemon_id;
  int type_id;
  int slot;
};

struct PokedexData {
  std::vector<PokemonRow> pokemon;
  std::vector<MovesRow> moves;
  std::vector<PokemonMovesRow> pokemon_moves;
  std::vector<PokemonSpeciesRow> pokemon_species;
  std::vector<ExperienceRow> experience;
  std::vector<TypeNamesRow> type_names;
  std::vector<PokemonStatsRow> pokemon_stats;
  std::vector<StatsRow> stats;
  std::vector<PokemonTypesRow> pokemon_types;
};

int handle_csv_mode(int argc, char **argv);
bool load_pokedex_data(PokedexData *db);

#endif