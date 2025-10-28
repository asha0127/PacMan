# Pac-Man Game

A C++ implementation of the classic Pac-Man arcade game using the SplashKit graphics library.

## Features

### Gameplay
- **Classic Pac-Man mechanics**: Navigate mazes, collect pellets, and avoid ghosts
- **Power Pellets**: Eat power pellets to turn ghosts blue and chase them for bonus points
- **Bonus Fruit**: Collect fruit for 200 bonus points
- **Multiple Levels**: 5 unique maze layouts with increasing difficulty
- **Two Game Modes**:
  - **Endless Mode**: Play through all 5 levels continuously, looping back to level 1
  - **Single Level**: Complete one specific level and return to menu

### Advanced Features
- **Difficulty Settings**: Easy (75% speed), Medium (100%), Hard (125%), Crazy (200%)
- **Ghost AI**: Two different AI behaviors:
  - **Random Patrol**: Wanders randomly, locks onto Pac-Man when close
  - **Ambusher**: Aims ahead of Pac-Man's position to cut him off
- **Ghost States**: Chasing, Scared (fleeing), Caught (returning home), Cooldown (immune)
- **Dynamic Audio**: Background music changes based on game state and pellets remaining
- **Velentina Mode**: Alternative sound theme with custom audio files
- **Customizable Pac-Man**: Choose from multiple color palettes
- **Score Popups**: Visual feedback when catching ghosts (400 points) or collecting fruit (200 points)
- **Smooth Animation**: Frame-independent movement with delta time
- **Tunnel Wrapping**: Entities can wrap around screen edges

## Requirements

- C++17 compatible compiler (clang++ or g++)
- SplashKit library installed
- MSYS2 (Windows) or equivalent Unix-like environment

## File Structure

```
Pacman/
├── main.cpp              # Program entry point
├── game.h/cpp            # Main game orchestrator
├── entities.h/cpp        # Entity classes (Pacman, Ghost, Fruit)
├── maze.h/cpp            # Maze and collision system
├── menu.h/cpp            # Menu navigation system
├── sound_manager.h/cpp   # Audio management
├── spritesheet.h/cpp     # Graphics rendering
├── game_config.h         # Configuration constants
├── direction.h           # Direction enum
├── Resources/
│   ├── Images/
│   │   └── pacman_spritemap.png
│   └── Sounds/
│       ├── Normal/       # Standard sound effects
│       │   ├── start.wav
│       │   ├── ghost1-5.wav
│       │   ├── ghostblue.wav
│       │   ├── ghosteat.wav
│       │   ├── ghostretreat.wav
│       │   ├── dot1.wav, dot2.wav
│       │   ├── die.wav
│       │   ├── cutscene.wav
│       │   └── fruit.wav
│       └── Velentina/    # Alternative sound theme
│           └── (same files as Normal/)
└── README.md
```

## Compilation

### Windows (MSYS2)
```bash
clang++ -std=c++17 main.cpp game.cpp menu.cpp entities.cpp maze.cpp \
  spritesheet.cpp sound_manager.cpp \
  -I"$MSYS2_ROOT/mingw64/include" \
  -L"$MSYS2_ROOT/mingw64/lib" \
  -lSplashKit -o pacman.exe
```

### Linux/macOS
```bash
clang++ -std=c++17 main.cpp game.cpp menu.cpp entities.cpp maze.cpp \
  spritesheet.cpp sound_manager.cpp \
  -lSplashKit -o pacman
```

## Running the Game

A precompiled executable is included in the repository:

### Windows
```bash
./pacman.exe
```

### Linux/macOS
```bash
./pacman
```

**Note**: If you encounter issues running the precompiled executable (e.g., missing dependencies or different system architecture), you will need to recompile from source using the instructions above.

## How to Play

### Controls
- **Arrow Keys**: Move Pac-Man (Up, Down, Left, Right)
- **Space/Enter**: Select menu options
- **Arrow Keys (Menu)**: Navigate menu options

### Menu Navigation
1. **Main Menu**:
   - Play Endless: Play through all 5 levels continuously
   - Play Level Select: Choose a specific level (1-5)
   - Difficulty: Adjust game speed
   - High Scores: View high scores (placeholder)
   - Settings: Customize Pac-Man and sound theme

2. **Settings**:
   - Pac-Man Palette: Change Pac-Man's colors
   - Velentina Mode: Toggle alternative sound theme

### Gameplay Tips
- Collect all pellets to complete a level
- Eat power pellets to make ghosts vulnerable
- Catch scared (blue) ghosts for 400 points each
- Collect fruit for 200 bonus points
- Ghosts have different AI patterns - learn their behaviors!
- Use tunnels on the sides to wrap around and escape ghosts

## Architecture

### Class Structure

#### **Game** (Main Orchestrator)
- Coordinates all game systems
- Manages game loop (update, render, events)
- Handles collision detection
- Transitions between game modes (STARTING, NORMAL, POWER_MODE, GAME_OVER, VICTORY)

#### **Entity Hierarchy**
- **Entity** (Base Class): Common movement and grid-alignment logic
  - **Pacman**: Player-controlled character with input, animation, and power mode
  - **Ghost**: AI-controlled enemy with pathfinding and state management
- **Fruit**: Bonus collectible with spawn timer and collision detection

#### **Maze**
- 13x25 cell grid (520x520 pixels)
- Wall collision detection
- Token and power pellet management
- Coordinate conversion utilities

#### **GameState**
- Score tracking
- Token collection management
- Win condition checking

#### **SoundManager**
- Background music based on game mode
- Dynamic chase music (speeds up as pellets decrease)
- Sound effect playback
- Support for multiple sound themes

#### **Menu**
- Menu state management
- Keyboard input handling
- Settings persistence
- Menu sound effects

#### **SpriteSheet**
- Sprite atlas rendering (16x16 sprites)
- Multiple color palette support
- Sprite flipping and trimming
- Scale and positioning utilities

### Game Flow

1. **Initialization**: Window opens, menu is displayed
2. **Menu Phase**: User selects options and starts game
3. **Entity Creation**: Pac-Man, ghosts, fruit, and maze are initialized
4. **Starting Sequence**: Start sound plays, entities frozen
5. **Normal Gameplay**: Entities move, collisions detected, score tracked
6. **Power Mode**: Ghost scared timer active, ghosts flee
7. **Level Complete**: All pellets collected, advance to next level
8. **Game Over**: Return to menu or continue in endless mode

## Scoring

- **Small Pellet**: 10 points
- **Power Pellet**: 50 points
- **Fruit**: 200 points
- **Ghost**: 400 points (only when scared)

## Game Modes

### GameMode States
- **STARTING**: Initial countdown with start.wav playing
- **NORMAL**: Standard chase mode with dynamic background music
- **POWER_MODE**: Ghosts scared, flee from Pac-Man (ghostblue.wav plays)
- **GAME_OVER**: All lives lost, die.wav plays
- **VICTORY**: Level complete, cutscene.wav plays

### Ghost States
- **CHASING**: Hunting Pac-Man (red/normal color)
- **SCARED**: Fleeing from Pac-Man (blue color, 15 seconds)
- **CAUGHT**: Returning to spawn point after being eaten
- **COOLDOWN**: Immune period at home before resuming chase (3 seconds)

## Technical Details

- **Frame Rate**: 60 FPS target
- **Movement**: Delta-time based for smooth, frame-independent motion
- **Grid Alignment**: Entities snap to grid for precise corner turning
- **Collision Distance**: 20 pixels for entity interactions
- **Cell Size**: 40x40 pixels

## Known Features

- Fruit spawns every 30 seconds and remains visible for 20 seconds
- Ghosts flash white when scared timer has 3 seconds remaining
- Score popups display for 1 second after collection
- Background chase music progresses through 5 tracks as pellets decrease
- Tunnel wrapping allows seamless screen edge teleportation


## Credits

Developed using the SplashKit graphics library for FIT1045.

Sprites were acquired from [spriters-resource.com](https://www.spriters-resource.com/) and are credited to user Superjustinbros.

## License

Educational project - all rights reserved.
