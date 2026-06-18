# Pokémon Roguelike Battle System

A terminal-based Pokémon roguelike written in C++ for COM S 327. This project builds on earlier terrain generation, world navigation, trainer movement, pathfinding, and CSV parsing assignments by adding a full turn-based Pokémon battle system.

The game uses `ncurses` for the terminal interface and loads Pokémon data from CSV files to generate real Pokémon, moves, stats, types, wild encounters, trainer parties, and battle behavior.

## Features

* Procedurally generated maps with terrain, roads, gates, PokéMarts, and Pokémon Centers
* 401x401 world grid with persistent maps
* Player movement across maps using roguelike-style controls
* Fly command for jumping to world coordinates
* NPC trainers with different movement behaviors
* Dijkstra-based pathfinding for trainer movement
* Trainer list display with relative positions
* CSV-based Pokédex loading
* Random starter Pokémon selection
* Wild Pokémon encounters in tall grass
* Trainer battles with full party logic
* Turn-based battle system
* Move priority, accuracy, damage calculation, and fainting
* Bag system with Potions, Revives, and Pokéballs
* Pokémon capture during wild battles
* Pokémon Center healing
* PokéMart item restocking
* Outside-of-battle bag usage

## Controls

| Key                | Action                           |
| ------------------ | -------------------------------- |
| `7` or `y`         | Move northwest                   |
| `8` or `k`         | Move north                       |
| `9` or `u`         | Move northeast                   |
| `6` or `l`         | Move east                        |
| `3` or `n`         | Move southeast                   |
| `2` or `j`         | Move south                       |
| `1` or `b`         | Move southwest                   |
| `4` or `h`         | Move west                        |
| `5`, `.`, or space | Rest for one turn                |
| `t`                | Show trainer list                |
| `>`                | Enter PokéMart or Pokémon Center |
| `f`                | Fly to another world coordinate  |
| `B`                | Open bag outside battle          |
| `Q`                | Quit game                        |

## Battle Controls

During battle, the player can choose from four main options:

| Option  | Description                              |
| ------- | ---------------------------------------- |
| Fight   | Choose one of the active Pokémon’s moves |
| Bag     | Use Potions, Revives, or Pokéballs       |
| Run     | Escape from a wild Pokémon battle        |
| Pokémon | Switch to another usable party member    |

Pokéballs only work in wild battles. Trainer battles continue until either the player or the trainer has no usable Pokémon left.

## Building the Project

This project uses a `Makefile`.

```bash
make
```

This creates the executable:

```bash
./assignment109
```

To clean compiled files:

```bash
make clean
```

## Running the Game

Run the game with the default number of trainers:

```bash
./assignment109
```

Run the game with a custom number of trainers:

```bash
./assignment109 --numtrainers 15
```

## CSV Data

The game loads Pokémon data from CSV files. It looks for the Pokédex database in one of the expected COM S 327 locations:

```text
/share/cs327/pokedex/pokedex/data/csv/
$HOME/.poke327/pokedex/pokedex/data/csv/
```

The project uses CSV data for Pokémon, moves, species, stats, types, experience, and Pokémon move learnsets.

## CSV Print Modes

The program can also print parsed CSV data directly by passing a dataset name as an argument.

Examples:

```bash
./assignment109 pokemon
./assignment109 moves
./assignment109 pokemon_moves
./assignment109 pokemon_species
./assignment109 experience
./assignment109 type_names
./assignment109 pokemon_stats
./assignment109 stats
./assignment109 pokemon_types
```


## How It Works

The game starts by loading the Pokédex CSV data, generating the first map at the center of the world, placing the player character, and asking the player to choose a starter Pokémon. From there, the game runs through a turn-based priority queue system where the player and NPC trainers act based on movement cost.

Maps are generated only when visited, then stored in the 401x401 world array so they can be revisited later. Gates align between neighboring maps, and PokéMart/Pokémon Center spawn rates decrease as the player moves farther from the center of the world.

Wild Pokémon can appear when the player walks through tall grass. Their level scales based on distance from the center of the world. NPC trainers also receive generated Pokémon parties and can challenge the player when encountered.

The battle system handles move selection, accuracy checks, priority, damage calculation, fainting, switching, item usage, capturing, and battle ending conditions.

## Notes

This project was developed as part of the COM S 327 Pokémon assignment sequence. Assignment 1.09 focuses mainly on implementing Pokémon battles, item usage, party management, wild encounters, trainer battles, and integration with Pokémon Centers and PokéMarts.

## Author

Aparneesh Patil
