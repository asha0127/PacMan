#pragma once

#include "maze.h"
#include "entities.h"
#include "spritesheet.h"
#include "game_config.h"
#include "sound_manager.h"
#include "menu.h"
#include "splashkit.h"
#include <memory>

/**
 * @file game.h
 * @brief Main game class for the Pac-Man game
 *
 * This file contains the Game class which orchestrates the entire game,
 * managing game objects, game state, and the main game loop.
 */

/**
 * @class Game
 * @brief Main game class that orchestrates the entire Pac-Man game
 *
 * The Game class is responsible for:
 * - Managing all game objects (maze, entities, sprites)
 * - Controlling the main game loop (update, render, events)
 * - Tracking game state and mode transitions
 * - Coordinating sound effects and background audio
 * - Handling collisions and game logic
 */
class Game
{
public:
    /**
     * @brief Constructor - initializes game with default state
     */
    Game();

    /**
     * @brief Destructor - ensures proper cleanup of resources
     */
    ~Game() = default;

    /**
     * @brief Initialize all game systems and load resources
     * @return true if initialization successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Main game loop - runs until game ends or window is closed
     */
    void run();

private:
    // === Core Game Loop Methods ===

    /**
     * @brief Update game logic, entities, and state
     * @param delta_time Time elapsed since last update (seconds)
     */
    void update(double delta_time);

    /**
     * @brief Render all game objects to the screen
     */
    void render();

    /**
     * @brief Process input events and player input
     */
    void handle_events();

    // === Game Objects ===
    std::unique_ptr<Maze> maze_;                  ///< Game maze and collision detection
    std::unique_ptr<SpriteSheet> sprite_sheet_;   ///< Sprite graphics management
    std::unique_ptr<Pacman> pacman_;              ///< Player character
    std::unique_ptr<Ghost> ghost1_;               ///< First AI ghost
    std::unique_ptr<Ghost> ghost2_;               ///< Second AI ghost
    std::unique_ptr<Fruit> fruit_;                ///< Bonus fruit
    std::unique_ptr<GameState> game_state_;       ///< Score, pellets, and game statistics
    std::unique_ptr<SoundManager> sound_manager_; ///< Audio management
    std::unique_ptr<Menu> menu_;                  ///< Menu system for navigation

    // === Game State ===
    bool running_;                ///< Whether the game is currently running
    bool game_initialized_;       ///< Whether game entities have been created
    bool paused_;                 ///< Whether the game is currently paused
    double escape_key_cooldown_;  ///< Cooldown timer for escape key to prevent double-triggering
    double last_time_;            ///< Last update time for delta calculation
    GameMode current_game_mode_;  ///< Current game mode (starting, normal, power, etc.)
    GameMode previous_game_mode_; ///< Previous mode for detecting transitions
    int current_level_;           ///< Current level (1-5)

    // === Game Logic Helper Methods ===

    /**
     * @brief Initialize game entities when starting to play
     */
    void initialize_game_entities();

    /**
     * @brief Update the current game mode based on game conditions
     * @param delta_time Time elapsed since last update (seconds)
     */
    void update_game_mode(double delta_time);

    /**
     * @brief Determine what the current game mode should be based on game state
     * @return The appropriate game mode for current conditions
     */
    GameMode determine_current_game_mode() const;

    /**
     * @brief Handle collisions between Pac-Man and ghosts
     */
    void handle_ghost_collisions();

    /**
     * @brief Check if the win condition has been met
     * @return true if player has won, false otherwise
     */
    bool check_win_condition();

    /**
     * @brief Calculate the percentage of pellets remaining
     * @return Percentage of pellets left (0-100)
     */
    double calculate_pellet_percentage() const;

    /**
     * @brief Advance to the next level
     */
    void advance_to_next_level();
};