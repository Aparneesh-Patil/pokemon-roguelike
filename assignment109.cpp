#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <climits>
#include <ncurses.h>

#include "worldgen.h"
#include "mapgen.h"
#include "pathfind.h"
#include "trainer.h"
#include "turnpq.h"
#include "movement.h"
#include "csv_parser.h"
#include "pokemon.h"
#include "battle.h"

static int parse_numtrainers(int argc, char **argv, int defval);
static void init_curses_colors(void);
static int color_for_tile(char ch);
static void draw_game(world *w, map *m, const character *pc, const char *msg);
static void trainer_relative_text(const character *pc, const character *t, char *buf, size_t len);
static void show_trainer_list(const character *pc, character *trainers, int num_trainers);
static void show_building_screen(pc *player, char building, char *msg, size_t msg_len);
static int prompt_fly_coordinates(int *world_x, int *world_y, char *msg, size_t msg_len);
static int handle_pc_turn(world *w, map **m_ptr, pc *player, int *quit_game, char *msg, size_t msg_len);
static void print_pokemon_lines(int start_row, const PokemonInstance *p);
static void show_wild_pokemon_screen(const PokedexData &db, pc *player, PokemonInstance *p, char *msg, size_t msg_len);
static void show_trainer_battle_screen(const PokedexData &db, pc *player, character *trainer, char *msg, size_t msg_len);
static void choose_starter(pc *player, const PokedexData &db);



int main(int argc, char **argv) {
  // if arugment is passed, we check for which file to load
  int csv_status = handle_csv_mode(argc, argv);

  if (csv_status == -1) {
    printf("Database failed to reload\n");
    return -1;
  }
  
  world *w;
  PokedexData pokedex;
  point_t center;
  map *m;
  int numtrainers, cur_time, action_result, cost, quit_game = 0;
  pc player;
  character *c;
  char msg[MSG_LEN] = "Pokemon Trial";

  std::srand(std::time(nullptr));

  if (!load_pokedex_data(&pokedex)) {
    std::fprintf(stderr, "Failed to load pokedex data.\n");
    return -1;
  }

  numtrainers = parse_numtrainers(argc, argv, 10);

  center.x = INIT_COORD;
  center.y = INIT_COORD;

  w = new world();
  w->coords = center;
  w->default_num_trainers = numtrainers;
  w->db = &pokedex;

  m = visit_map(w, w->coords);
  if (!m) {
    destroy_world(w);
    return -1;
  }

  player.id = 0;
  player.type = '@';
  player.next_time = 0;
  player.dir = 0;
  player.defeated = 0;
  player.pos = player_spawn(m);
  player.spawn_terrain = m->grid[player.pos.y][player.pos.x];
  player.party_size = 0;
  init_pc_items(&player);

  place_pc_on_map(m, &player, player.pos);
  compute_distance_maps(m, player.pos);

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  init_curses_colors();
  choose_starter(&player, pokedex);

  while (!quit_game) {
    m = current_map(w);

    c = turnpq_peek(&m->pq);

    if (!c || player.next_time <= c->next_time) {
      action_result = handle_pc_turn(w, &m, &player, &quit_game, msg, sizeof(msg));
      if (quit_game) {
        break;
      }

      if (action_result) {
        cost = movement_cost(m, player.pos.x, player.pos.y, player.type);
        if (cost == INT_MAX) {
          cost = 10;
        }
        player.next_time += cost;
      }
    } else {
      c = turnpq_pop(&m->pq);
      if (!c) {
        continue;
      }

      cur_time = c->next_time;
      action_result = take_turn(m, c, &player.pos);

      if (action_result == 2) {
        show_trainer_battle_screen(pokedex, &player, c, msg, sizeof(msg));;
      }

      cost = movement_cost(m, c->pos.x, c->pos.y, c->type);
      if (cost == INT_MAX) {
        cost = 10;
      }
      c->next_time = cur_time + cost;

      turnpq_push(&m->pq, c);
    }
  }

  endwin();
  destroy_world(w);
  return 0;
}

static int parse_numtrainers(int argc, char **argv, int defval) {
  int n = defval;

  for (int i = 1; i < argc; i++) {
    if (!std::strcmp(argv[i], "--numtrainers") && i + 1 < argc) {
      n = std::atoi(argv[i + 1]);
      i++;
    }
  }

  if (n < 0) n = 0;
  if (n > MAX_TRAINERS) n = MAX_TRAINERS;
  return n;
}

static void init_curses_colors(void) {
  if (!has_colors()) {
    return;
  }

  start_color();
  use_default_colors();

  init_pair(CP_DEFAULT, -1, -1);
  init_pair(CP_PATH, COLOR_YELLOW, -1);
  init_pair(CP_CENTER, COLOR_RED, -1);
  init_pair(CP_MART, COLOR_BLUE, -1);
  init_pair(CP_GRASS, COLOR_GREEN, -1);
  init_pair(CP_CLEAR, COLOR_WHITE, -1);
  init_pair(CP_WATER, COLOR_CYAN, -1);
  init_pair(CP_FOREST, COLOR_GREEN, -1);
  init_pair(CP_BOULDER, COLOR_WHITE, -1);
  init_pair(CP_MOUNTAIN, COLOR_MAGENTA, -1);
  init_pair(CP_PC, COLOR_YELLOW, -1);
  init_pair(CP_TRAINER, COLOR_RED, -1);
  init_pair(CP_STATUS, COLOR_CYAN, -1);
}

static int color_for_tile(char ch) {
  switch (ch) {
    case PATH: return CP_PATH;
    case POKEMON_CENTER: return CP_CENTER;
    case POKEMART: return CP_MART;
    case TALL_GRASS: return CP_GRASS;
    case CLEARING: return CP_CLEAR;
    case WATER: return CP_WATER;
    case FOREST: return CP_FOREST;
    case BOULDER: return CP_BOULDER;
    case MOUNTAIN: return CP_MOUNTAIN;
    case '@': return CP_PC;
    case 'h':
    case 'r':
    case 'p':
    case 'w':
    case 'e':
    case 's': return CP_TRAINER;
    default: return CP_DEFAULT;
  }
}

static void draw_game(world *w, map *m, const character *pc, const char *msg) {
  int wx = w->coords.x - INIT_COORD;
  int wy = w->coords.y - INIT_COORD;

  erase();
  mvprintw(0, 0, "%.*s", MAP_W - 1, msg ? msg : "");

  for (int y = 0; y < MAP_H; y++) {
    move(y + 1, 0);
    for (int x = 0; x < MAP_W; x++) {
      char ch = (m->character_map[y][x] != ' ') ? m->character_map[y][x] : m->grid[y][x];
      attron(COLOR_PAIR(color_for_tile(ch)));
      addch(ch);
      attroff(COLOR_PAIR(color_for_tile(ch)));
    }
  }

  attron(COLOR_PAIR(CP_STATUS));
  mvprintw(22, 0, "World (%d,%d)  PC (%d,%d)  Trainers: %d  Move: ykuhlbnj or 1-9  Fly: f  Rest: . space 5", wx, wy, pc->pos.x, pc->pos.y, m->num_trainers);
  mvprintw(23, 0, "List: t  Enter building: >  Bag: B  Quit: Q");;
  attroff(COLOR_PAIR(CP_STATUS));
  refresh();
}

static void trainer_relative_text(const character *pc, const character *t, char *buf, size_t len) {
  int dy = t->pos.y - pc->pos.y;
  int dx = t->pos.x - pc->pos.x;
  char ns[32] = "same row";
  char ew[32] = "same column";

  if (dy < 0) {
    std::snprintf(ns, sizeof(ns), "%d north", -dy);
  } else if (dy > 0) {
    std::snprintf(ns, sizeof(ns), "%d south", dy);
  }

  if (dx < 0) {
    std::snprintf(ew, sizeof(ew), "%d west", -dx);
  } else if (dx > 0) {
    std::snprintf(ew, sizeof(ew), "%d east", dx);
  }

  std::snprintf(buf, len, "%c, %s and %s%s", t->type, ns, ew, t->defeated ? " (defeated)" : "");
}

static void show_trainer_list(const character *pc, character *trainers, int num_trainers) {
  int offset = 0;
  int rows = LINES - 2;

  if (rows < 1) rows = 1;

  while (1) {
    erase();
    mvprintw(0, 0, "Trainer list (ESC to return)");

    for (int i = 0; i < rows && offset + i < num_trainers; i++) {
      char line[96];
      trainer_relative_text(pc, &trainers[offset + i], line, sizeof(line));
      mvprintw(i + 1, 0, "%s", line);
    }

    refresh();
    int ch = getch();

    if (ch == 27) {
      break;
    }
    if (ch == KEY_UP && offset > 0) {
      offset--;
    } else if (ch == KEY_DOWN && offset + rows < num_trainers) {
      offset++;
    }
  }
}

static void show_building_screen(pc *player, char building, char *msg, size_t msg_len) {
  const char *name = (building == POKEMART) ? "Pokemart" : "Pokemon Center";

  erase();
  mvprintw(10, 20, "%s", name);
  if (building == POKEMART) {
    restock_pc_items(player);
    mvprintw(12, 20, "Your supplies were restocked.");
    std::snprintf(msg, msg_len, "Pokemart restored your supplies.");
  } else {
    heal_pc_party(player);
    mvprintw(12, 20, "Your party was fully healed.");
    std::snprintf(msg, msg_len, "Pokemon Center healed your party.");
  }
  mvprintw(14, 20, "Press any key to leave");
  refresh();
  getch();
}

static int prompt_fly_coordinates(int *world_x, int *world_y, char *msg, size_t msg_len) {
  char xbuf[32], ybuf[32];

  echo();
  curs_set(1);

  erase();
  mvprintw(10, 10, "Fly to world coordinates.");
  mvprintw(12, 10, "Enter X [-200,200]: ");
  getnstr(xbuf, sizeof(xbuf) - 1);

  mvprintw(13, 10, "Enter Y [-200,200]: ");
  getnstr(ybuf, sizeof(ybuf) - 1);

  noecho();
  curs_set(0);

  *world_x = std::atoi(xbuf);
  *world_y = std::atoi(ybuf);

  if (*world_x < -200 || *world_x > 200 || *world_y < -200 || *world_y > 200) {
    std::snprintf(msg, msg_len, "Invalid fly coordinates.");
    return 0;
  }

  std::snprintf(msg, msg_len, "Flying to (%d,%d).", *world_x, *world_y);
  return 1;
}

static int handle_pc_turn(world *w, map **m_ptr, pc *player, int *quit_game, char *msg, size_t msg_len) {
  int ch, battle_index, rc, dy = 0, dx = 0;
  int fly_x, fly_y;
  map *m = *m_ptr;
  PokemonInstance wild_pokemon;

  while (!*quit_game) {
    m = current_map(w);
    *m_ptr = m;

    draw_game(w, m, player, msg);
    ch = getch();

    dy = 0;
    dx = 0;

    switch (ch) {
      case '7': case 'y': dy = -1; dx = -1; break;
      case '8': case 'k': dy = -1; dx = 0; break;
      case '9': case 'u': dy = -1; dx = 1; break;
      case '6': case 'l': dy = 0; dx = 1; break;
      case '3': case 'n': dy = 1; dx = 1; break;
      case '2': case 'j': dy = 1; dx = 0; break;
      case '1': case 'b': dy = 1; dx = -1; break;
      case '4': case 'h': dy = 0; dx = -1; break;

      case '5':
      case ' ':
      case '.':
        std::snprintf(msg, msg_len, "PC rests for a turn.");
        return 1;

      case 't':
        show_trainer_list(player, m->trainers, m->num_trainers);
        std::snprintf(msg, msg_len, "Returned from trainer list.");
        break;

      case '>':
        if (m->grid[player->pos.y][player->pos.x] == POKEMART || m->grid[player->pos.y][player->pos.x] == POKEMON_CENTER) {
          show_building_screen(player, m->grid[player->pos.y][player->pos.x], msg, msg_len);
          return 1;
        } else {
          std::snprintf(msg, msg_len, "You are not standing on a building.");
        }
        break;

      case 'f':
        if (prompt_fly_coordinates(&fly_x, &fly_y, msg, msg_len)) {
          if (fly_pc_to(w, player, fly_x, fly_y)) {
            *m_ptr = current_map(w);
            compute_distance_maps(*m_ptr, player->pos);
            return 1;
          } else {
            std::snprintf(msg, msg_len, "Fly failed.");
          }
        }
        break;

      case 'B':
        open_bag_menu_outside_battle(player, msg, msg_len);
        return 1;

      case 'Q':
        *quit_game = 1;
        return 0;

      default:
        std::snprintf(msg, msg_len, "Unknown key.");
        break;
    }

    if (dy != 0 || dx != 0) {
      rc = attempt_pc_move(w, m, player, dy, dx, &battle_index, &wild_pokemon, msg, msg_len);

      if (rc == PC_MOVE_BATTLE && battle_index >= 0) {
        show_trainer_battle_screen(*w->db, player, &m->trainers[battle_index], msg, msg_len);
        compute_distance_maps(m, player->pos);
        return 1;
      }

      if (rc == PC_MOVE_WILD) {
        show_wild_pokemon_screen(*w->db, player, &wild_pokemon, msg, msg_len);
        compute_distance_maps(m, player->pos);
        return 1;
      }

      if (rc == PC_MOVE_OK) {
        compute_distance_maps(m, player->pos);
        return 1;
      }

      if (rc == PC_MOVE_GATE_W || rc == PC_MOVE_GATE_E ||
          rc == PC_MOVE_GATE_N || rc == PC_MOVE_GATE_S) {
        if (move_pc_to_neighbor(w, player, rc)) {
          *m_ptr = current_map(w);
          compute_distance_maps(*m_ptr, player->pos);
          return 1;
        } else {
          std::snprintf(msg, msg_len, "You cannot leave the world bounds.");
        }
      }
    }
  }

  return 0;
}

static void print_pokemon_lines(int start_row, const PokemonInstance *p) {
  mvprintw(start_row, 2, "%s  Lv%d  Gender:%c  %s", p->name.c_str(), p->level, p->gender, p->shiny ? "Shiny" : "Normal");
  mvprintw(start_row + 1, 4, "HP:%d/%d Atk:%d Def:%d SpA:%d SpD:%d Spe:%d", p->hp, p->max_hp, p->attack, p->defense, p->sp_attack, p->sp_defense, p->speed);

  if (p->num_moves == 0) {
    mvprintw(start_row + 2, 4, "Moves: none");
  } else if (p->num_moves == 1) {
    mvprintw(start_row + 2, 4, "Moves: %s", p->moves[0].name.c_str());
  } else {
    mvprintw(start_row + 2, 4, "Moves: %s, %s", p->moves[0].name.c_str(), p->moves[1].name.c_str());
  }
}

static void show_wild_pokemon_screen(const PokedexData &db, pc *player, PokemonInstance *p, char *msg, size_t msg_len) {
  run_wild_battle(db, player, p, msg, msg_len);
}

static void show_trainer_battle_screen(const PokedexData &db, pc *player, character *trainer, char *msg, size_t msg_len) {
  run_trainer_battle(db, player, trainer, msg, msg_len);
}

static void choose_starter(pc *player, const PokedexData &db) {
  PokemonInstance choices[3];

  for (int i = 0; i < 3; i++) {
    generate_random_pokemon_for_world(db, 1, 1, &choices[i]);
  }

  while (1) {
    erase();
    mvprintw(1, 2, "Choose your starter Pokemon");
    mvprintw(2, 2, "Press 1, 2, or 3");

    for (int i = 0; i < 3; i++) {
      mvprintw(4 + i * 5, 2, "%d)", i + 1);
      print_pokemon_lines(4 + i * 5, &choices[i]);
    }

    refresh();
    int ch = getch();

    if (ch == '1' || ch == '2' || ch == '3') {
      int idx = ch - '1';
      player->party[0] = choices[idx];
      player->party_size = 1;
      break;
    }
  }
}