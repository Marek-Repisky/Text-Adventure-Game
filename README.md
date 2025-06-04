# Text Adventure Game

A C-based text adventure game where players navigate through various rooms, battle enemies, solve puzzles, and collect keys to escape.

## Overview

You awaken in a dimly lit room with no memory of how you got there. Your goal is to find keys and escape by navigating through six different types of rooms, each presenting unique challenges and opportunities.

## Features

- **Character Customization**: Allocate 5 special points across 5 attributes (Strength, Endurance, Charisma, Agility, Luck)
- **Save/Load System**: Save your progress and continue later
- **Combat System**: Turn-based fighting with damage calculations based on stats
- **Multiple Room Types**: 6 different room types with unique mechanics
- **Inventory Management**: Collect coins, keys, and items
- **Logging System**: Automatic game session logging

## Game Mechanics

### Character Attributes
- **Strength**: Affects damage output in combat
- **Endurance**: Determines HP and reduces enemy damage
- **Charisma**: Influences success rate of distraction attempts
- **Agility**: Affects dodge chance and trap avoidance
- **Luck**: Influences starting coins and revive token chance

### Room Types

1. **Fight Room**: Battle enemies with options to fight, run away, or distract
2. **Maze**: Navigate through a timed maze puzzle (25-second limit)
3. **Merchant**: Trade coins for items including health potions, equipment, and keys
4. **Trap**: Avoid dangerous traps using agility
5. **Alchemy**: Solve a combination puzzle to create keys
6. **Boss Room**: Face the final boss (requires 5 keys to enter)

### Items Available from Merchant
- Small Health Potion (10 coins) - Restores 30 HP
- Large Health Potion (20 coins) - Restores 80 HP
- Luck Amulet (15 coins) - Increases luck by 1
- Sword (20 coins) - Increases damage by 10
- Shield (20 coins) - Increases endurance by 1
- Stealth Cloak (15 coins) - Increases agility by 1
- Revive Token (20 coins) - Grants an extra life
- Key (10 coins) - Required for boss room access

## Compilation and Execution

### Prerequisites
- GCC compiler or compatible C compiler
- Standard C libraries (stdio.h, stdlib.h, time.h, string.h, stdarg.h)

### Compilation
```bash
gcc -o adventure_game main.c
```

### Running the Game
```bash
# With command line argument for player name
./adventure_game "PlayerName"

# Or run without arguments to be prompted for name
./adventure_game
```

## File Structure

### Game Files
- `main.c` - Main game source code
- `[PlayerName].sav` - Binary save file for game progress
- `[PlayerName]_log.txt` - Text log of game session

### Save System
The game automatically creates save files in binary format containing:
- Hero statistics and inventory
- Current room position
- Room visit status

## Gameplay Tips

1. **Stat Allocation**: Balance your attributes based on your preferred playstyle
   - High Strength for combat effectiveness
   - High Agility for trap avoidance and dodging
   - High Charisma for successful distractions
   - High Luck for better starting resources

2. **Combat Strategy**: 
   - Fighting is turn-based with damage ranges
   - Running away may cause damage if unsuccessful
   - Distraction requires 10 coins and good charisma

3. **Resource Management**:
   - Collect keys from defeated enemies and completed rooms
   - Manage coins carefully for merchant purchases
   - Save health potions for difficult encounters

4. **Boss Preparation**: 
   - You need exactly 5 keys to enter the boss room
   - The boss has significantly higher HP and damage
   - Consider purchasing equipment and health items beforehand

## Game Flow

1. **Character Creation**: Allocate attribute points
2. **Room Navigation**: Progress through rooms linearly (west to east)
3. **Challenge Resolution**: Complete each room's unique challenge
4. **Key Collection**: Gather keys from successful room completions
5. **Boss Fight**: Face the final boss with 5 keys
6. **Victory**: Escape and win the game

## Technical Details

### Memory Management
- Dynamic memory allocation for world map and rooms
- Proper cleanup with `free_world()` function
- Error handling for memory allocation failures

### Random Number Generation
- Seeded with current time for gameplay variety
- Used for damage calculations, enemy stats, and puzzle generation

### Logging System
- Comprehensive logging of all game events
- Timestamped session records
- Both console output and file logging

## Known Features

- Maze room has a 25-second time limit
- Alchemy room provides debug output showing the correct combination
- Combat system includes miss chances based on agility
- Revive system allows continuation after death (if tokens available)
