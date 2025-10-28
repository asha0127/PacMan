#pragma once

#include "maze.h"

/**
 * @file game_config.h
 * @brief Configuration constants for the Pac-Man game
 *
 * This file contains all configuration constants used throughout the game,
 * including window settings, graphics paths, and game parameters.
 */

/**
 * Game configuration constants
 * Contains all non-audio configuration settings for the game
 */
namespace GameConfig
{
    // Window and display settings
    constexpr int WINDOW_WIDTH = MazeConfig::MAZE_COLS * MazeConfig::CELL_SIZE;
    constexpr int WINDOW_HEIGHT = MazeConfig::MAZE_ROWS * MazeConfig::CELL_SIZE;
    constexpr int TARGET_FPS = 60;
    constexpr const char *WINDOW_TITLE = "Pac-Man";

    // Graphics settings
    constexpr const char *SPRITESHEET_NAME = "pacman_spritemap";
    constexpr const char *SPRITESHEET_PATH = "Resources/Images/pacman_spritemap.png";

    // Gameplay settings
    constexpr double COLLISION_DISTANCE = 20.0;  ///< Distance for collision detection between entities (increased from 15 to prevent corner stuck bug)
    constexpr int GHOST_CATCH_POINTS = 200;      ///< Points awarded for catching a ghost
    constexpr int GAME_OVER_DISPLAY_TIME = 3000; ///< Time to display game over message (milliseconds)
}