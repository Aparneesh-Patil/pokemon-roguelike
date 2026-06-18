#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <ncurses.h>
#include "battle.h"
#include "pokemon.h"

namespace {
const int DEFAULT_POTIONS = 5;
const int DEFAULT_REVIVES = 3;
const int DEFAULT_POKEBALLS = 5;

enum BattleActionType {
    ACTION_MOVE,
    ACTION_BAG_POTION,
    ACTION_BAG_REVIVE,
    ACTION_BAG_POKEBALL,
    ACTION_SWITCH,
    ACTION_RUN,
    ACTION_NONE
};

struct BattleAction {
    BattleActionType type;
    int move_index;
    int target_index;

    BattleAction() : type(ACTION_NONE), move_index(-1), target_index(-1) {}
};

void draw_battle_header(const pc *player, const PokemonInstance *my_poke,
                        const character *opponent, const PokemonInstance *their_poke,
                        bool is_wild, const char *status) {
    (void) opponent;
    erase();
    mvprintw(1, 2, "%s", is_wild ? "Wild battle" : "Trainer battle");
    mvprintw(2, 2, "Your %s Lv%d HP %d/%d", my_poke->name.c_str(), my_poke->level, my_poke->hp, my_poke->max_hp);
    mvprintw(3, 2, "%s %s Lv%d HP %d/%d",
             is_wild ? "Wild" : "Enemy",
             their_poke->name.c_str(),
             their_poke->level,
             their_poke->hp,
             their_poke->max_hp);
    mvprintw(5, 2, "Items: Potions %d  Revives %d  Pokeballs %d",
             player->potions, player->revives, player->pokeballs);
    mvprintw(6, 2, "%s", status ? status : "");
}

void draw_party_lines(const character *c, int active_idx, int start_row) {
    for (int i = 0; i < c->party_size; i++) {
        const PokemonInstance &p = c->party[i];
        mvprintw(start_row + i, 4, "%d) %s Lv%d HP %d/%d%s%s",
                 i + 1, p.name.c_str(), p.level, p.hp, p.max_hp,
                 pokemon_is_knocked_out(p) ? " [KO]" : "",
                 i == active_idx ? " [ACTIVE]" : "");
    }
}

int prompt_party_choice(const character *c, int active_idx, bool allow_ko, const char *title) {
    while (1) {
        erase();
        mvprintw(1, 2, "%s", title);
        draw_party_lines(c, active_idx, 3);
        mvprintw(LINES - 2, 2, "Press 1-6 to choose, ESC to cancel");
        refresh();

        int ch = getch();
        if (ch == 27) return -1;
        if (ch >= '1' && ch <= '6') {
            int idx = ch - '1';
            if (idx >= c->party_size) continue;
            if (!allow_ko && pokemon_is_knocked_out(c->party[idx])) continue;
            return idx;
        }
    }
}

BattleAction prompt_player_action(const pc *player, int active_idx, const character *opponent,
                                  int opp_active_idx, bool is_wild, const char *status) {
    BattleAction action;

    while (1) {
        draw_battle_header(player, &player->party[active_idx], opponent, &opponent->party[opp_active_idx], is_wild, status);
        mvprintw(8, 2, "1) Fight");
        mvprintw(9, 2, "2) Bag");
        mvprintw(10, 2, "3) Run");
        mvprintw(11, 2, "4) Pokemon");
        refresh();

        int ch = getch();
        if (ch == '1') {
            while (1) {
                erase();
                mvprintw(1, 2, "Choose a move");
                const PokemonInstance &p = player->party[active_idx];
                for (int i = 0; i < p.num_moves; i++) {
                    mvprintw(3 + i, 4, "%d) %s (Pow %d Acc %d Pri %d)", i + 1,
                             p.moves[i].name.c_str(), p.moves[i].power, p.moves[i].accuracy, p.moves[i].priority);
                }
                mvprintw(LINES - 2, 2, "Press move number or ESC to go back");
                refresh();
                int mch = getch();
                if (mch == 27) break;
                if (mch >= '1' && mch <= '2') {
                    int idx = mch - '1';
                    if (idx < p.num_moves) {
                        action.type = ACTION_MOVE;
                        action.move_index = idx;
                        return action;
                    }
                }
            }
        } else if (ch == '2') {
            while (1) {
                erase();
                mvprintw(1, 2, "Bag");
                mvprintw(3, 4, "1) Potion x%d", player->potions);
                mvprintw(4, 4, "2) Revive x%d", player->revives);
                if (is_wild) {
                    mvprintw(5, 4, "3) Pokeball x%d", player->pokeballs);
                }
                mvprintw(LINES - 2, 2, "Press item number or ESC to go back");
                refresh();
                int bch = getch();
                if (bch == 27) break;
                if (bch == '1' && player->potions > 0) {
                    int idx = prompt_party_choice(player, active_idx, false, "Use potion on which Pokemon?");
                    if (idx >= 0) {
                        action.type = ACTION_BAG_POTION;
                        action.target_index = idx;
                        return action;
                    }
                } else if (bch == '2' && player->revives > 0) {
                    int idx = prompt_party_choice(player, active_idx, true, "Use revive on which Pokemon?");
                    if (idx >= 0 && pokemon_is_knocked_out(player->party[idx])) {
                        action.type = ACTION_BAG_REVIVE;
                        action.target_index = idx;
                        return action;
                    }
                } else if (is_wild && bch == '3' && player->pokeballs > 0) {
                    action.type = ACTION_BAG_POKEBALL;
                    return action;
                }
            }
        } else if (ch == '3') {
            action.type = ACTION_RUN;
            return action;
        } else if (ch == '4') {
            int idx = prompt_party_choice(player, active_idx, false, "Switch to which Pokemon?");
            if (idx >= 0 && idx != active_idx) {
                action.type = ACTION_SWITCH;
                action.target_index = idx;
                return action;
            }
        }
    }
}

BattleAction choose_ai_action(const character *trainer, int active_idx, bool is_wild) {
    BattleAction action;
    const PokemonInstance &p = trainer->party[active_idx];
    (void) is_wild;
    action.type = ACTION_MOVE;
    action.move_index = std::rand() % p.num_moves;
    return action;
}

bool move_hits(const LearnedMove &move) {
    return (std::rand() % 100) < move.accuracy;
}

bool is_critical_hit(const PokemonInstance &attacker) {
    int threshold = attacker.base_speed / 2;
    return (std::rand() % 256) < threshold;
}

int calculate_damage(const PokemonInstance &attacker, const PokemonInstance &defender, const LearnedMove &move) {
    int power = move.power > 0 ? move.power : 1;
    int critical = is_critical_hit(attacker) ? 150 : 100;
    int random_factor = 85 + (std::rand() % 16);
    int stab = pokemon_has_type(attacker, move.type_id) ? 150 : 100;
    int type_effect = 100;
    int defense = defender.defense > 0 ? defender.defense : 1;

    int base = (((((2 * attacker.level) / 5) + 2) * power * attacker.attack) / defense) / 50 + 2;
    int damage = base;
    damage = (damage * critical) / 100;
    damage = (damage * random_factor) / 100;
    damage = (damage * stab) / 100;
    damage = (damage * type_effect) / 100;
    if (damage < 1) damage = 1;
    return damage;
}

bool action_has_non_move_priority(const BattleAction &a) {
    return a.type != ACTION_MOVE;
}

bool player_goes_first(const BattleAction &player_action, const BattleAction &enemy_action,
                       const PokemonInstance &player_poke, const PokemonInstance &enemy_poke) {
    if (action_has_non_move_priority(player_action) != action_has_non_move_priority(enemy_action)) {
        return action_has_non_move_priority(player_action);
    }

    if (player_action.type == ACTION_MOVE && enemy_action.type == ACTION_MOVE) {
        int pp = player_poke.moves[player_action.move_index].priority;
        int ep = enemy_poke.moves[enemy_action.move_index].priority;
        if (pp != ep) return pp > ep;
        if (player_poke.speed != enemy_poke.speed) return player_poke.speed > enemy_poke.speed;
        return (std::rand() & 1) == 0;
    }

    return true;
}

bool resolve_action(const PokedexData &db, pc *player, int &player_active, character *enemy,
                    int &enemy_active, bool is_wild, const BattleAction &action,
                    bool actor_is_player, char *status, size_t status_len) {
    (void) db;
    character *actor = actor_is_player ? static_cast<character *>(player) : enemy;
    character *target = actor_is_player ? enemy : static_cast<character *>(player);
    int &actor_active = actor_is_player ? player_active : enemy_active;
    int &target_active = actor_is_player ? enemy_active : player_active;
    PokemonInstance &actor_poke = actor->party[actor_active];
    PokemonInstance &target_poke = target->party[target_active];

    if (pokemon_is_knocked_out(actor_poke)) {
        std::snprintf(status, status_len, "%s is knocked out and loses its turn.", actor_poke.name.c_str());
        return false;
    }

    switch (action.type) {
        case ACTION_MOVE: {
            const LearnedMove &move = actor_poke.moves[action.move_index];
            if (!move_hits(move)) {
                std::snprintf(status, status_len, "%s used %s, but it missed.", actor_poke.name.c_str(), move.name.c_str());
                return false;
            }
            int damage = calculate_damage(actor_poke, target_poke, move);
            target_poke.hp -= damage;
            if (target_poke.hp < 0) target_poke.hp = 0;
            std::snprintf(status, status_len, "%s used %s for %d damage.", actor_poke.name.c_str(), move.name.c_str(), damage);
            return pokemon_is_knocked_out(target_poke);
        }
        case ACTION_BAG_POTION:
            if (actor_is_player && player->potions > 0 && use_potion_on_pokemon(&player->party[action.target_index])) {
                player->potions--;
                std::snprintf(status, status_len, "Used a potion on %s.", player->party[action.target_index].name.c_str());
            } else {
                std::snprintf(status, status_len, "Potion had no effect.");
            }
            return false;
        case ACTION_BAG_REVIVE:
            if (actor_is_player && player->revives > 0 && use_revive_on_pokemon(&player->party[action.target_index])) {
                player->revives--;
                std::snprintf(status, status_len, "Revived %s.", player->party[action.target_index].name.c_str());
            } else {
                std::snprintf(status, status_len, "Revive had no effect.");
            }
            return false;
        case ACTION_BAG_POKEBALL:
            if (actor_is_player && is_wild && player->pokeballs > 0) {
                player->pokeballs--;
                if (player->party_size < 6) {
                    player->party[player->party_size] = target_poke;
                    player->party_size++;
                    std::snprintf(status, status_len, "Caught %s!", target_poke.name.c_str());
                    target_poke.hp = 0;
                    return true;
                }
                std::snprintf(status, status_len, "Your party is full. The wild %s broke free and fled.", target_poke.name.c_str());
                target_poke.hp = 0;
                return true;
            }
            std::snprintf(status, status_len, "You cannot use that now.");
            return false;
        case ACTION_SWITCH:
            actor_active = action.target_index;
            std::snprintf(status, status_len, "%s switched to %s.", actor_is_player ? "You" : "Enemy", actor->party[actor_active].name.c_str());
            return false;
        case ACTION_RUN:
            if (actor_is_player && is_wild) {
                if ((std::rand() % 100) < 50) {
                    std::snprintf(status, status_len, "You successfully ran away.");
                    return true;
                }
                std::snprintf(status, status_len, "Could not escape.");
            } else {
                std::snprintf(status, status_len, "You cannot run from a trainer battle.");
            }
            return false;
        case ACTION_NONE:
        default:
            std::snprintf(status, status_len, "Nothing happened.");
            return false;
    }
}

bool force_switch_if_needed(pc *player, int &active_idx, char *status, size_t status_len) {
    if (!pokemon_is_knocked_out(player->party[active_idx])) return true;
    int idx = prompt_party_choice(player, active_idx, false, "Choose a new active Pokemon");
    while (idx < 0) {
        idx = prompt_party_choice(player, active_idx, false, "You must choose a usable Pokemon");
    }
    active_idx = idx;
    std::snprintf(status, status_len, "Go, %s!", player->party[active_idx].name.c_str());
    return true;
}

bool enemy_force_switch_if_needed(character *enemy, int &active_idx, char *status, size_t status_len) {
    if (!pokemon_is_knocked_out(enemy->party[active_idx])) return true;
    int idx = first_usable_pokemon_index(enemy);
    if (idx < 0) return false;
    active_idx = idx;
    std::snprintf(status, status_len, "Enemy sent out %s!", enemy->party[active_idx].name.c_str());
    return true;
}

void run_battle_core(const PokedexData &db, pc *player, character *enemy, bool is_wild, char *msg, size_t msg_len) {
    int player_active = first_usable_pokemon_index(player);
    int enemy_active = first_usable_pokemon_index(enemy);
    char status[256];

    if (msg && msg_len > 0) {
        msg[0] = '\0';
    }

    std::snprintf(status, sizeof(status), "%s",
                  is_wild ? "A wild Pokemon appeared!" : "A trainer challenges you!");

    auto show_end_dialog = [&](const char *line1) {
        erase();
        mvprintw(10, 20, "%s", line1);
        mvprintw(12, 20, "Press any key to continue");
        refresh();
        getch();
    };

    auto finish_battle = [&](const char *final_msg, bool show_dialog) {
        if (show_dialog) {
            show_end_dialog(final_msg);
        }
        std::snprintf(msg, msg_len, "%s", final_msg);
    };

    if (player_active < 0) {
        finish_battle("You have no usable Pokemon.", false);
        return;
    }

    if (enemy_active < 0) {
        if (!is_wild) {
            enemy->defeated = 1;
            finish_battle("Trainer has no usable Pokemon.", false);
        } else {
            finish_battle("The wild Pokemon is gone.", false);
        }
        return;
    }

    while (1) {
        if (!party_has_usable_pokemon(player)) {
            finish_battle("You lost the battle.", true);
            return;
        }

        if (!party_has_usable_pokemon(enemy)) {
            if (!is_wild) {
                enemy->defeated = 1;
                finish_battle("You defeated the trainer.", true);
            } else {
                finish_battle("The wild Pokemon was defeated.", true);
            }
            return;
        }

        if (pokemon_is_knocked_out(player->party[player_active])) {
            force_switch_if_needed(player, player_active, status, sizeof(status));
        }

        if (pokemon_is_knocked_out(enemy->party[enemy_active])) {
            if (!enemy_force_switch_if_needed(enemy, enemy_active, status, sizeof(status))) {
                if (!is_wild) {
                    enemy->defeated = 1;
                    finish_battle("You defeated the trainer.", true);
                } else {
                    finish_battle("The wild Pokemon was defeated.", true);
                }
                return;
            }
        }

        BattleAction player_action =
            prompt_player_action(player, player_active, enemy, enemy_active, is_wild, status);

        if (!is_wild && player_action.type == ACTION_RUN) {
            std::snprintf(status, sizeof(status), "You cannot run from a trainer battle.");
            continue;
        }

        BattleAction enemy_action = choose_ai_action(enemy, enemy_active, is_wild);

        bool player_first = player_goes_first(player_action, enemy_action,
                                              player->party[player_active],
                                              enemy->party[enemy_active]);

        bool battle_over = false;

        if (player_first) {
            battle_over = resolve_action(db, player, player_active, enemy, enemy_active,
                                         is_wild, player_action, true, status, sizeof(status));

            if (battle_over) {
                if (player_action.type == ACTION_RUN) {
                    finish_battle(status, false);
                    return;
                }

                if (player_action.type == ACTION_BAG_POKEBALL) {
                    finish_battle(status, true);
                    return;
                }

                if (!party_has_usable_pokemon(enemy)) {
                    if (!is_wild) {
                        enemy->defeated = 1;
                        finish_battle("You defeated the trainer.", true);
                    } else {
                        finish_battle("The wild Pokemon was defeated.", true);
                    }
                    return;
                }
            }

            if (!party_has_usable_pokemon(enemy)) {
                if (!is_wild) {
                    enemy->defeated = 1;
                    finish_battle("You defeated the trainer.", true);
                } else {
                    finish_battle("The wild Pokemon was defeated.", true);
                }
                return;
            }

            if (!pokemon_is_knocked_out(enemy->party[enemy_active])) {
                battle_over = resolve_action(db, player, player_active, enemy, enemy_active,
                                             is_wild, enemy_action, false, status, sizeof(status));

                if (battle_over && !party_has_usable_pokemon(player)) {
                    finish_battle("You lost the battle.", true);
                    return;
                }
            }
        } else {
            battle_over = resolve_action(db, player, player_active, enemy, enemy_active,
                                         is_wild, enemy_action, false, status, sizeof(status));

            if (battle_over && !party_has_usable_pokemon(player)) {
                finish_battle("You lost the battle.", true);
                return;
            }

            if (!pokemon_is_knocked_out(player->party[player_active])) {
                battle_over = resolve_action(db, player, player_active, enemy, enemy_active,
                                             is_wild, player_action, true, status, sizeof(status));

                if (battle_over) {
                    if (player_action.type == ACTION_RUN) {
                        finish_battle(status, false);
                        return;
                    }

                    if (player_action.type == ACTION_BAG_POKEBALL) {
                        finish_battle(status, true);
                        return;
                    }

                    if (!party_has_usable_pokemon(enemy)) {
                        if (!is_wild) {
                            enemy->defeated = 1;
                            finish_battle("You defeated the trainer.", true);
                        } else {
                            finish_battle("The wild Pokemon was defeated.", true);
                        }
                        return;
                    }
                }
            }
        }
    }
}

} // namespace

void init_pc_items(pc *player) {
    if (!player) return;
    player->potions = DEFAULT_POTIONS;
    player->revives = DEFAULT_REVIVES;
    player->pokeballs = DEFAULT_POKEBALLS;
}

void restock_pc_items(pc *player) {
    init_pc_items(player);
}

void heal_pc_party(pc *player) {
    fully_heal_party(player);
}

void open_bag_menu_outside_battle(pc *player, char *msg, size_t msg_len) {
    while (1) {
        erase();
        mvprintw(1, 2, "Bag outside battle");
        mvprintw(3, 4, "1) Potion x%d", player->potions);
        mvprintw(4, 4, "2) Revive x%d", player->revives);
        mvprintw(LINES - 2, 2, "Press item number or ESC to leave");
        refresh();

        int ch = getch();
        if (ch == 27) {
            std::snprintf(msg, msg_len, "Closed bag.");
            return;
        }
        if (ch == '1' && player->potions > 0) {
            int idx = prompt_party_choice(player, -1, false, "Use potion on which Pokemon?");
            if (idx >= 0 && use_potion_on_pokemon(&player->party[idx])) {
                player->potions--;
                std::snprintf(msg, msg_len, "Used a potion on %s.", player->party[idx].name.c_str());
                return;
            }
        }
        if (ch == '2' && player->revives > 0) {
            int idx = prompt_party_choice(player, -1, true, "Use revive on which Pokemon?");
            if (idx >= 0 && use_revive_on_pokemon(&player->party[idx])) {
                player->revives--;
                std::snprintf(msg, msg_len, "Revived %s.", player->party[idx].name.c_str());
                return;
            }
        }
    }
}

void run_wild_battle(const PokedexData &db, pc *player, PokemonInstance *wild, char *msg, size_t msg_len) {
    character enemy;
    enemy.type = 'W';
    enemy.party[0] = *wild;
    enemy.party_size = 1;
    run_battle_core(db, player, &enemy, true, msg, msg_len);
}

void run_trainer_battle(const PokedexData &db, pc *player, character *trainer, char *msg, size_t msg_len) {
    run_battle_core(db, player, trainer, false, msg, msg_len);
}