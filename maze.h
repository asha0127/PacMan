#pragma once

#include "splashkit.h"
#include "direction.h"
#include <vector>
#include <string>
#include <cmath>

// Forward declarations
class GameState;

/**
 * Constants related to maze and gameplay
 */
namespace MazeConfig
{
    constexpr int MAZE_ROWS = 13;
    constexpr int MAZE_COLS = 25;
    constexpr int CELL_SIZE = 40;
    constexpr double SPEED = 120.0;            // pixels per second (was 2.0 pixels per frame at 60fps)
    constexpr double ANIMATION_DURATION = 0.2; // seconds per animation frame (was 12 frames at 60fps)
    constexpr int PACMAN_RADIUS_OFFSET = 2;
    constexpr double ALIGNMENT_TOLERANCE = 4.0;
    constexpr double SPRITE_SCALE = 1.584;

    // Token and scoring constants
    constexpr int TOKEN_POINTS = 10;
    constexpr double TOKEN_RADIUS = 3.0;
    constexpr double COLLECTION_DISTANCE = 15.0;

    // Power pellet constants
    constexpr int POWER_PELLET_POINTS = 50;
    constexpr double POWER_PELLET_RADIUS = 8.0;
    constexpr double POWER_PELLET_COLLECTION_DISTANCE = 20.0;
    // Power mode duration removed - using individual ghost SCARED_DURATION
}

/**
 * Token class - Represents collectible pellets in the maze
 */
class Token
{
public:
    Token(int row, int col);

    // Getters
    int get_row() const { return row_; }
    int get_col() const { return col_; }
    bool is_collected() const { return collected_; }
    double get_x() const;
    double get_y() const;

    // Actions
    void collect() { collected_ = true; }
    void draw() const;

private:
    int row_, col_;
    bool collected_;
};

/**
 * PowerPellet class - Represents special large pellets that activate power mode
 */
class PowerPellet
{
public:
    PowerPellet(int row, int col);

    // Getters
    int get_row() const { return row_; }
    int get_col() const { return col_; }
    bool is_collected() const { return collected_; }
    double get_x() const;
    double get_y() const;

    // Actions
    void collect() { collected_ = true; }
    void draw() const;

private:
    int row_, col_;
    bool collected_;
};

/**
 * GameState class - Manages score, tokens, and game statistics
 */
class GameState
{
public:
    GameState();

    // Score management
    int get_score() const { return score_; }
    void add_score(int points) { score_ += points; }

    // Token management
    void add_token(int row, int col);
    void add_power_pellet(int row, int col);
    int get_tokens_collected() const { return tokens_collected_; }
    int get_total_tokens() const { return total_tokens_; }
    bool all_tokens_collected() const { return tokens_collected_ >= total_tokens_; }

    // Power pellet tracking (for ghost management)
    int count_collected_power_pellets() const;

    // Game operations
    bool check_token_collection(double pacman_x, double pacman_y);
    bool check_power_pellet_collection(double pacman_x, double pacman_y);
    void draw_tokens() const;
    void draw_power_pellets() const;
    void draw_score() const;
    void update(double delta_time);

    // Sound-related methods
    bool was_token_just_collected() const { return token_just_collected_; }
    void reset_token_collection_flag() { token_just_collected_ = false; }

private:
    int score_;
    int tokens_collected_;
    int total_tokens_;
    std::vector<Token> tokens_;
    std::vector<PowerPellet> power_pellets_;
    bool token_just_collected_; // Flag for sound effects

    // Power mode state
    // Power mode removed - using individual ghost timers only
};

/**
 * Maze class - Represents the game maze with walls and empty spaces
 * Walls are represented by 1, empty spaces by 0
 */
class Maze
{
public:
    Maze(int level = 1);

    // Rendering
    void draw() const;

    // Collision and movement
    bool can_move_to(double x, double y) const;
    bool is_empty(int row, int col) const;
    bool is_empty_or_tunnel(int row, int col) const;

    // Utility methods
    static double get_cell_center_x(int col);
    static double get_cell_center_y(int row);

    // Game initialization
    void initialize_tokens(GameState &game_state, int spawn_row, int spawn_col) const;
    void initialize_power_pellets(GameState &game_state) const;

    // Helper function
    static std::pair<int, int> find_spawn_position(const Maze &maze, int target_row, int target_col);

    // Load maze from CSV file
    bool load_from_csv(const std::string &filename);

private:
    std::vector<std::vector<int>> maze_layout_;
    int level_; ///< Current level number (1-5)
    bool is_valid_position(int row, int col) const;

    /**
     * @brief Get the wall color for the current level
     * @return The color to use for drawing walls
     */
    color get_level_color() const;
};