#pragma once

#include "splashkit.h"
#include <vector>
#include <string>

// Forward declaration
class SpriteSheet;
class SoundManager;

/**
 * @file menu.h
 * @brief Menu system for the Pac-Man game
 *
 * This file contains the menu states and menu manager class
 * for navigating the game's menu system.
 */

/**
 * Menu state enumeration
 * Determines which menu screen is currently displayed
 */
enum class MenuState
{
    MAIN_MENU,    ///< Main menu with Play, Difficulty, High Scores, Settings
    LEVEL_SELECT, ///< Level selection submenu
    DIFFICULTY,   ///< Difficulty selection screen (placeholder)
    HIGH_SCORES,  ///< High scores display screen (placeholder)
    SETTINGS,     ///< Settings screen (placeholder)
    NAME_ENTRY,   ///< Arcade-style name entry for high score
    IN_GAME       ///< Currently playing the game
};

/**
 * Menu option enumeration for main menu
 */
enum class MainMenuOption
{
    PLAY_ENDLESS = 0,
    PLAY_LEVEL_SELECT = 1,
    DIFFICULTY = 2,
    HIGH_SCORES = 3,
    SETTINGS = 4,
    COUNT = 5 ///< Total number of options
};

/**
 * Difficulty level enumeration
 */
enum class DifficultyLevel
{
    EASY = 0,   ///< 75% speed
    MEDIUM = 1, ///< 100% speed (default)
    HARD = 2,   ///< 125% speed
    CRAZY = 3,  ///< 200% speed
    COUNT = 4   ///< Total number of difficulty levels
};

/**
 * High score entry structure
 */
struct HighScoreEntry
{
    std::string name; ///< 3-letter player name
    int score;        ///< Score achieved
};

/**
 * @class Menu
 * @brief Manages menu navigation and rendering
 *
 * The Menu class handles:
 * - Rendering menu screens
 * - Processing keyboard input for navigation
 * - Tracking selected menu option
 * - State transitions between menu screens
 */
class Menu
{
public:
    /**
     * @brief Render the settings screen
     */
    void render_settings_screen();
    /**
     * @brief Handle input for the settings screen
     */
    void handle_settings_input();
    /**
     * @brief Set the current menu state
     */
    void set_state(MenuState state) { current_state_ = state; }
    /**
     * @brief Constructor - initializes menu with default state
     */
    Menu();

    /**
     * @brief Set the sprite sheet for rendering Pac-Man preview
     * @param sheet Pointer to the sprite sheet
     */
    void set_sprite_sheet(SpriteSheet *sheet) { sprite_sheet_ = sheet; }

    /**
     * @brief Set the sound manager for menu sound effects
     * @param sound_manager Pointer to the sound manager
     */
    void set_sound_manager(SoundManager *sound_manager) { sound_manager_ = sound_manager; }

    /**
     * @brief Process keyboard input for menu navigation
     * Arrow keys to navigate, spacebar to select
     */
    void handle_input();

    /**
     * @brief Render the current menu screen
     */
    void render();

    /**
     * @brief Get the current menu state
     * @return Current MenuState
     */
    MenuState get_state() const { return current_state_; }

    /**
     * @brief Check if user has selected to start the game
     * @return true if game should start, false otherwise
     */
    bool should_start_game() const { return should_start_game_; }

    /**
     * @brief Reset the game start flag (call after starting game)
     */
    void reset_game_start_flag() { should_start_game_ = false; }

    /**
     * @brief Get the selected Pac-Man palette name
     * @return The currently selected palette name
     */
    const char *get_selected_pacman_palette() const;

    /**
     * @brief Check if Velentina Mode is enabled
     * @return true if enabled, false otherwise
     */
    bool is_velentina_mode_enabled() const { return velentina_mode_; }

    /**
     * @brief Get the current difficulty level
     * @return The selected difficulty level
     */
    DifficultyLevel get_difficulty_level() const { return difficulty_level_; }

    /**
     * @brief Get the speed multiplier for the current difficulty
     * @return Speed multiplier (0.75, 1.0, 1.25, or 2.0)
     */
    double get_difficulty_speed_multiplier() const;

    /**
     * @brief Check if endless mode is enabled
     * @return true if endless mode, false if single level mode
     */
    bool is_endless_mode() const { return endless_mode_; }

    /**
     * @brief Get the selected level number (1-5)
     * @return Selected level number
     */
    int get_selected_level() const { return selected_level_; }

    /**
     * @brief Initiate high score name entry
     * @param score The score to save
     */
    void start_name_entry(int score);

    /**
     * @brief Check if name entry is complete
     * @return true if player has entered their name
     */
    bool is_name_entry_complete() const { return name_entry_complete_; }

    /**
     * @brief Reset name entry flag
     */
    void reset_name_entry_flag() { name_entry_complete_ = false; }

private:
    MenuState current_state_;          ///< Current menu screen
    int selected_option_;              ///< Currently selected option (0-indexed)
    bool should_start_game_;           ///< Flag to signal game should start
    int selected_palette_index_;       ///< Index of the selected Pac-Man color palette
    SpriteSheet *sprite_sheet_;        ///< Pointer to sprite sheet for rendering preview
    SoundManager *sound_manager_;      ///< Pointer to sound manager for menu sounds
    bool velentina_mode_;              ///< Velentina Mode toggle flag
    DifficultyLevel difficulty_level_; ///< Current difficulty level
    int selected_difficulty_option_;   ///< Currently selected difficulty option in menu
    bool endless_mode_;                ///< true for endless mode, false for single level
    int selected_level_;               ///< Selected level number (1-5) for single level mode

    // High score name entry state
    bool name_entry_complete_;                ///< Flag to signal name entry is done
    int pending_score_;                       ///< Score waiting to be saved
    char name_letters_[3];                    ///< 3-letter name being entered
    int name_cursor_position_;                ///< Current letter position (0-2)
    std::vector<HighScoreEntry> high_scores_; ///< Loaded high scores

    // Cooldown to prevent rapid menu navigation
    double last_input_time_;                       ///< Time of last input
    static constexpr double INPUT_COOLDOWN = 0.15; ///< Seconds between inputs

    /**
     * @brief Render the main menu
     */
    void render_main_menu();

    /**
     * @brief Render the difficulty selection screen
     */
    void render_difficulty_screen();

    /**
     * @brief Render the high scores screen
     */
    void render_high_scores_screen();

    /**
     * @brief Handle input for the main menu
     */
    void handle_main_menu_input();

    /**
     * @brief Handle input for the difficulty screen
     */
    void handle_difficulty_input();

    /**
     * @brief Handle input for the high scores screen
     */
    void handle_high_scores_input();

    /**
     * @brief Render the level selection screen
     */
    void render_level_select_screen();

    /**
     * @brief Handle input for the level selection screen
     */
    void handle_level_select_input();

    /**
     * @brief Render the name entry screen
     */
    void render_name_entry_screen();

    /**
     * @brief Handle input for the name entry screen
     */
    void handle_name_entry_input();

    /**
     * @brief Load high scores from file
     */
    void load_high_scores();

    /**
     * @brief Save high scores to file
     */
    void save_high_scores();

    /**
     * @brief Add a new high score entry
     * @param name 3-letter player name
     * @param score Score achieved
     */
    void add_high_score(const std::string &name, int score);
};
