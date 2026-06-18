#ifndef BATTLE_H
#define BATTLE_H

#include <cstddef>
#include "trainer.h"
#include "csv_parser.h"

void init_pc_items(pc *player);
void restock_pc_items(pc *player);
void heal_pc_party(pc *player);
void open_bag_menu_outside_battle(pc *player, char *msg, size_t msg_len);
void run_wild_battle(const PokedexData &db, pc *player, PokemonInstance *wild, char *msg, size_t msg_len);
void run_trainer_battle(const PokedexData &db, pc *player, character *trainer, char *msg, size_t msg_len);

#endif