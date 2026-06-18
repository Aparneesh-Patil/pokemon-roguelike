#include "csv_parser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <unistd.h>

static bool file_exists(const std::string &path) {
  return access(path.c_str(), R_OK) == 0;
}

static std::string build_home_csv_dir() {
  const char *home = std::getenv("HOME");
  if (!home) {
    return "";
  }
  return std::string(home) + "/.poke327/pokedex/pokedex/data/csv/";
}

static std::string find_csv_dir() {
  const std::string share_dir = "/share/cs327/pokedex/pokedex/data/csv/";
  const std::string home_dir = build_home_csv_dir();

  if (file_exists(share_dir + "pokemon.csv")) {
    return share_dir;
  }

  if (!home_dir.empty() && file_exists(home_dir + "pokemon.csv")) {
    return home_dir;
  }

  return "";
}

static std::vector<std::string> split_csv_line(const std::string &line) {
  std::vector<std::string> fields;
  std::string cur;

  for (size_t i = 0; i < line.size(); i++) {
    if (line[i] == ',') {
      fields.push_back(cur);
      cur.clear();
    } else {
      cur += line[i];
    }
  }

  fields.push_back(cur);
  return fields;
}

static int to_int(const std::string &s) {
  if (s.empty()) {
    return INT_MAX;
  }
  return std::stoi(s);
}

static void print_int_field(int x) {
  if (x != INT_MAX) {
    std::cout << x;
  }
}

static void print_string_field(const std::string &s) {
  std::cout << s;
}

static void print_comma() {
  std::cout << ",";
}

// loads pokemon.csv

static std::vector<PokemonRow> parse_pokemon(const std::string &csv_dir) {
  std::vector<PokemonRow> rows;
  std::ifstream file(csv_dir + "pokemon.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open pokemon.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 8) {
      continue;
    }

    PokemonRow r;
    r.id = to_int(f[0]);
    r.identifier = f[1];
    r.species_id = to_int(f[2]);
    r.height = to_int(f[3]);
    r.weight = to_int(f[4]);
    r.base_experience = to_int(f[5]);
    r.order = to_int(f[6]);
    r.is_default = to_int(f[7]);

    rows.push_back(r);
  }

  return rows;
}

static void print_pokemon(const std::vector<PokemonRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].id); print_comma();
    print_string_field(rows[i].identifier); print_comma();
    print_int_field(rows[i].species_id); print_comma();
    print_int_field(rows[i].height); print_comma();
    print_int_field(rows[i].weight); print_comma();
    print_int_field(rows[i].base_experience); print_comma();
    print_int_field(rows[i].order); print_comma();
    print_int_field(rows[i].is_default);
    std::cout << "\n";
  }
}

// loads moves.csv

static std::vector<MovesRow> parse_moves(const std::string &csv_dir) {
  std::vector<MovesRow> rows;
  std::ifstream file(csv_dir + "moves.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open moves.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 15) {
      continue;
    }

    MovesRow r;
    r.id = to_int(f[0]);
    r.identifier = f[1];
    r.generation_id = to_int(f[2]);
    r.type_id = to_int(f[3]);
    r.power = to_int(f[4]);
    r.pp = to_int(f[5]);
    r.accuracy = to_int(f[6]);
    r.priority = to_int(f[7]);
    r.target_id = to_int(f[8]);
    r.damage_class_id = to_int(f[9]);
    r.effect_id = to_int(f[10]);
    r.effect_chance = to_int(f[11]);
    r.contest_type_id = to_int(f[12]);
    r.contest_effect_id = to_int(f[13]);
    r.super_contest_effect_id = to_int(f[14]);

    rows.push_back(r);
  }

  return rows;
}

static void print_moves(const std::vector<MovesRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].id); print_comma();
    print_string_field(rows[i].identifier); print_comma();
    print_int_field(rows[i].generation_id); print_comma();
    print_int_field(rows[i].type_id); print_comma();
    print_int_field(rows[i].power); print_comma();
    print_int_field(rows[i].pp); print_comma();
    print_int_field(rows[i].accuracy); print_comma();
    print_int_field(rows[i].priority); print_comma();
    print_int_field(rows[i].target_id); print_comma();
    print_int_field(rows[i].damage_class_id); print_comma();
    print_int_field(rows[i].effect_id); print_comma();
    print_int_field(rows[i].effect_chance); print_comma();
    print_int_field(rows[i].contest_type_id); print_comma();
    print_int_field(rows[i].contest_effect_id); print_comma();
    print_int_field(rows[i].super_contest_effect_id);
    std::cout << "\n";
  }
}

// loads pokemon_moves.csv

static std::vector<PokemonMovesRow> parse_pokemon_moves(const std::string &csv_dir) {
  std::vector<PokemonMovesRow> rows;
  std::ifstream file(csv_dir + "pokemon_moves.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open pokemon_moves.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 6) {
      continue;
    }

    PokemonMovesRow r;
    r.pokemon_id = to_int(f[0]);
    r.version_group_id = to_int(f[1]);
    r.move_id = to_int(f[2]);
    r.pokemon_move_method_id = to_int(f[3]);
    r.level = to_int(f[4]);
    r.order = to_int(f[5]);

    rows.push_back(r);
  }

  return rows;
}

static void print_pokemon_moves(const std::vector<PokemonMovesRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].pokemon_id); print_comma();
    print_int_field(rows[i].version_group_id); print_comma();
    print_int_field(rows[i].move_id); print_comma();
    print_int_field(rows[i].pokemon_move_method_id); print_comma();
    print_int_field(rows[i].level); print_comma();
    print_int_field(rows[i].order);
    std::cout << "\n";
  }
}

// loads pokemon_species.csv

static std::vector<PokemonSpeciesRow> parse_pokemon_species(const std::string &csv_dir) {
  std::vector<PokemonSpeciesRow> rows;
  std::ifstream file(csv_dir + "pokemon_species.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open pokemon_species.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 20) {
      continue;
    }

    PokemonSpeciesRow r;
    r.id = to_int(f[0]);
    r.identifier = f[1];
    r.generation_id = to_int(f[2]);
    r.evolves_from_species_id = to_int(f[3]);
    r.evolution_chain_id = to_int(f[4]);
    r.color_id = to_int(f[5]);
    r.shape_id = to_int(f[6]);
    r.habitat_id = to_int(f[7]);
    r.gender_rate = to_int(f[8]);
    r.capture_rate = to_int(f[9]);
    r.base_happiness = to_int(f[10]);
    r.is_baby = to_int(f[11]);
    r.hatch_counter = to_int(f[12]);
    r.has_gender_differences = to_int(f[13]);
    r.growth_rate_id = to_int(f[14]);
    r.forms_switchable = to_int(f[15]);
    r.is_legendary = to_int(f[16]);
    r.is_mythical = to_int(f[17]);
    r.order = to_int(f[18]);
    r.conquest_order = to_int(f[19]);

    rows.push_back(r);
  }

  return rows;
}

static void print_pokemon_species(const std::vector<PokemonSpeciesRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].id); print_comma();
    print_string_field(rows[i].identifier); print_comma();
    print_int_field(rows[i].generation_id); print_comma();
    print_int_field(rows[i].evolves_from_species_id); print_comma();
    print_int_field(rows[i].evolution_chain_id); print_comma();
    print_int_field(rows[i].color_id); print_comma();
    print_int_field(rows[i].shape_id); print_comma();
    print_int_field(rows[i].habitat_id); print_comma();
    print_int_field(rows[i].gender_rate); print_comma();
    print_int_field(rows[i].capture_rate); print_comma();
    print_int_field(rows[i].base_happiness); print_comma();
    print_int_field(rows[i].is_baby); print_comma();
    print_int_field(rows[i].hatch_counter); print_comma();
    print_int_field(rows[i].has_gender_differences); print_comma();
    print_int_field(rows[i].growth_rate_id); print_comma();
    print_int_field(rows[i].forms_switchable); print_comma();
    print_int_field(rows[i].is_legendary); print_comma();
    print_int_field(rows[i].is_mythical); print_comma();
    print_int_field(rows[i].order); print_comma();
    print_int_field(rows[i].conquest_order);
    std::cout << "\n";
  }
}

// loads experience.csv
static std::vector<ExperienceRow> parse_experience(const std::string &csv_dir) {
  std::vector<ExperienceRow> rows;
  std::ifstream file(csv_dir + "experience.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open experience.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 3) {
      continue;
    }

    ExperienceRow r;
    r.growth_rate_id = to_int(f[0]);
    r.level = to_int(f[1]);
    r.experience = to_int(f[2]);

    rows.push_back(r);
  }

  return rows;
}

static void print_experience(const std::vector<ExperienceRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].growth_rate_id); print_comma();
    print_int_field(rows[i].level); print_comma();
    print_int_field(rows[i].experience);
    std::cout << "\n";
  }
}

// loads type_names.csv

static std::vector<TypeNamesRow> parse_type_names(const std::string &csv_dir) {
  std::vector<TypeNamesRow> rows;
  std::ifstream file(csv_dir + "type_names.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open type_names.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 3) {
      continue;
    }

    TypeNamesRow r;
    r.type_id = to_int(f[0]);
    r.local_language_id = to_int(f[1]);
    r.name = f[2];

    if (r.local_language_id == 9) {
      rows.push_back(r);
    }
  }

  return rows;
}

static void print_type_names(const std::vector<TypeNamesRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].type_id); print_comma();
    print_int_field(rows[i].local_language_id); print_comma();
    print_string_field(rows[i].name);
    std::cout << "\n";
  }
}

// loads pokemon_stats.csv

static std::vector<PokemonStatsRow> parse_pokemon_stats(const std::string &csv_dir) {
  std::vector<PokemonStatsRow> rows;
  std::ifstream file(csv_dir + "pokemon_stats.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open pokemon_stats.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 4) {
      continue;
    }

    PokemonStatsRow r;
    r.pokemon_id = to_int(f[0]);
    r.stat_id = to_int(f[1]);
    r.base_stat = to_int(f[2]);
    r.effort = to_int(f[3]);

    rows.push_back(r);
  }

  return rows;
}

static void print_pokemon_stats(const std::vector<PokemonStatsRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].pokemon_id); print_comma();
    print_int_field(rows[i].stat_id); print_comma();
    print_int_field(rows[i].base_stat); print_comma();
    print_int_field(rows[i].effort);
    std::cout << "\n";
  }
}

// loads stats.csv

static std::vector<StatsRow> parse_stats(const std::string &csv_dir) {
  std::vector<StatsRow> rows;
  std::ifstream file(csv_dir + "stats.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open stats.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 5) {
      continue;
    }

    StatsRow r;
    r.id = to_int(f[0]);
    r.damage_class_id = to_int(f[1]);
    r.identifier = f[2];
    r.is_battle_only = to_int(f[3]);
    r.game_index = to_int(f[4]);

    rows.push_back(r);
  }

  return rows;
}

static void print_stats(const std::vector<StatsRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].id); print_comma();
    print_int_field(rows[i].damage_class_id); print_comma();
    print_string_field(rows[i].identifier); print_comma();
    print_int_field(rows[i].is_battle_only); print_comma();
    print_int_field(rows[i].game_index);
    std::cout << "\n";
  }
}

// loads pokemon_types.csv

static std::vector<PokemonTypesRow> parse_pokemon_types(const std::string &csv_dir) {
  std::vector<PokemonTypesRow> rows;
  std::ifstream file(csv_dir + "pokemon_types.csv");
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open pokemon_types.csv\n";
    return rows;
  }

  std::getline(file, line);

  while (std::getline(file, line)) {
    std::vector<std::string> f = split_csv_line(line);
    if (f.size() < 3) {
      continue;
    }

    PokemonTypesRow r;
    r.pokemon_id = to_int(f[0]);
    r.type_id = to_int(f[1]);
    r.slot = to_int(f[2]);

    rows.push_back(r);
  }

  return rows;
}

static void print_pokemon_types(const std::vector<PokemonTypesRow> &rows) {
  for (size_t i = 0; i < rows.size(); i++) {
    print_int_field(rows[i].pokemon_id); print_comma();
    print_int_field(rows[i].type_id); print_comma();
    print_int_field(rows[i].slot);
    std::cout << "\n";
  }
}

// handles the arugment inputted by the user in the main file

static std::string normalize_mode(int argc, char **argv) {
  if (argc < 2) {
    return "";
  }

  std::string mode = argv[1];

  if (mode == "pokemon") return "pokemon";
  if (mode == "moves") return "moves";
  if (mode == "experience") return "experience";
  if (mode == "stats") return "stats";

  if (mode == "pokemon_moves" || mode == "pokemon-moves") return "pokemon_moves";
  if (mode == "pokemon_species" || mode == "pokemon-species") return "pokemon_species";
  if (mode == "type_names" || mode == "type-names") return "type_names";
  if (mode == "pokemon_stats" || mode == "pokemon-stats") return "pokemon_stats";
  if (mode == "pokemon_types" || mode == "pokemon-types") return "pokemon_types";

  if (argc >= 3) {
    std::string two = std::string(argv[1]) + " " + argv[2];
    if (two == "pokemon moves") return "pokemon_moves";
    if (two == "pokemon species") return "pokemon_species";
    if (two == "type names") return "type_names";
    if (two == "pokemon stats") return "pokemon_stats";
    if (two == "pokemon types") return "pokemon_types";
  }

  return "";
}

int  handle_csv_mode(int argc, char **argv) {
  std::string mode = normalize_mode(argc, argv);

  if (mode.empty()) {
    return 1;
  }

  std::string csv_dir = find_csv_dir();
  if (csv_dir.empty()) {
    std::cerr << "Could not find Pokedex database in /share/cs327 or $HOME/.poke327\n";
    return 0;
  }

  if (mode == "pokemon") {
    print_pokemon(parse_pokemon(csv_dir));
    return 0;
  }

  if (mode == "moves") {
    print_moves(parse_moves(csv_dir));
    return 0;
  }

  if (mode == "pokemon_moves") {
    print_pokemon_moves(parse_pokemon_moves(csv_dir));
    return 0;
  }

  if (mode == "pokemon_species") {
    print_pokemon_species(parse_pokemon_species(csv_dir));
    return 0;
  }

  if (mode == "experience") {
    print_experience(parse_experience(csv_dir));
    return 0;
  }

  if (mode == "type_names") {
    print_type_names(parse_type_names(csv_dir));
    return 0;
  }

  if (mode == "pokemon_stats") {
    print_pokemon_stats(parse_pokemon_stats(csv_dir));
    return 0;
  }

  if (mode == "stats") {
    print_stats(parse_stats(csv_dir));
    return 0;
  }

  if (mode == "pokemon_types") {
    print_pokemon_types(parse_pokemon_types(csv_dir));
    return 0;
  }

  return -2;
}

bool load_pokedex_data(PokedexData *db) {
  if (!db) {
    return false;
  }

  std::string csv_dir = find_csv_dir();
  if (csv_dir.empty()) {
    std::cerr << "Could not find pokedex CSV directory.\n";
    return false;
  }

  db->pokemon = parse_pokemon(csv_dir);
  db->moves = parse_moves(csv_dir);
  db->pokemon_moves = parse_pokemon_moves(csv_dir);
  db->pokemon_species = parse_pokemon_species(csv_dir);
  db->experience = parse_experience(csv_dir);
  db->type_names = parse_type_names(csv_dir);
  db->pokemon_stats = parse_pokemon_stats(csv_dir);
  db->stats = parse_stats(csv_dir);
  db->pokemon_types = parse_pokemon_types(csv_dir);

  if (db->pokemon.empty() || db->moves.empty() ||
      db->pokemon_moves.empty() || db->pokemon_stats.empty()) {
    std::cerr << "Failed to load required CSV data.\n";
    return false;
  }

  return true;
}