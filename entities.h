#pragma once

#include "maze.h"
#include "spritesheet.h"
#include "direction.h"
#include <string>
#include <tuple>
#include <vector>
#include <algorithm>

// Forward declarations
class Ghost;

/**
 * Ghost state enumeration for different AI behaviors
 */
enum class GhostState
{
    CHASING, // Normal behavior - chase Pacman
    SCARED,  // Power mode - run away from Pacman
    CAUGHT,  // Returning to center after being caught
    COOLDOWN // Waiting at home before resuming chase
};

/**
 * Base class for all game entities (Pac-Man, ghosts, etc.)
 * Provides position, direction, and movement functionality
 */
class Entity
{
public:
    Entity(double start_x = 0, double start_y = 0, const std::string &palette = "YELLOW_PINK_SKY");
    virtual ~Entity() = default;

    // Getters
    double get_x() const { return x_; }
    double get_y() const { return y_; }
    direction_t get_direction() const { return dir_; }
    direction_t get_desired_direction() const { return desired_dir_; }
    const std::string &get_palette() const { return palette_; }

    // Setters
    void set_position(double x, double y);
    void set_desired_direction(direction_t dir) { desired_dir_ = dir; }
    void set_palette(const std::string &palette) { palette_ = palette; }
    void set_speed_multiplier(double multiplier) { speed_multiplier_ = multiplier; }

    // Virtual methods
    virtual void update(const Maze &maze, double delta_time = 1.0 / 60.0);
    virtual void draw() const = 0;
    virtual double get_current_speed() const { return MazeConfig::SPEED * speed_multiplier_; } // Can be overridden for different speeds

protected:
    double x_, y_;            // Position in pixels
    direction_t dir_;         // Current movement direction
    direction_t desired_dir_; // Desired movement direction
    std::string palette_;     // Color palette for rendering
    double speed_multiplier_; // Difficulty-based speed multiplier

private:
    void move_in_direction(const Maze &maze, double delta_time);
    void attempt_direction_change(const Maze &maze, int row, int col, double center_x, double center_y);
    void attempt_movement(const Maze &maze, double center_x, double center_y, double delta_time);
    static void get_next_cell(direction_t direction, int &row, int &col);
    bool is_aligned_for_direction(direction_t direction, double center_x, double center_y) const;
    void align_to_grid(direction_t direction, double center_x, double center_y);
    std::pair<double, double> get_next_position(direction_t direction, double delta_time) const;
    void snap_to_grid_if_close(double center_x, double center_y);
};

/**
 * Pacman class - The player character
 * Handles input, tunnel wrapping, animation, and rendering
 */
class Pacman : public Entity
{
public:
    // Animation states for Pac-Man's mouth
    enum class AnimationState
    {
        OPEN = 0,
        CLOSING = 1,
        CLOSED = 2
    };

    Pacman(double start_x, double start_y, SpriteSheet *sheet, const std::string &palette = "YELLOW_PINK_SKY");

    void capture_input();
    void update(const Maze &maze, double delta_time = 1.0 / 60.0) override;
    void update(const Maze &maze, GameState &game_state, double delta_time = 1.0 / 60.0);
    void draw() const override;
    double get_current_speed() const override;

    void set_power_mode(bool is_power_mode) { is_in_power_mode_ = is_power_mode; }

public:
    /**
     * @brief Play Pacman dying animation sequence
     * @param maze The maze to draw during animation
     * @param game_state The game state to draw tokens and score during animation
     * @param ghost1 First ghost to draw during animation (can be nullptr)
     * @param ghost2 Second ghost to draw during animation (can be nullptr)
     */
    void play_dying_animation(const Maze *maze, const GameState *game_state, const Ghost *ghost1 = nullptr, const Ghost *ghost2 = nullptr);

private:
    SpriteSheet *sheet_;
    AnimationState anim_state_;
    double anim_timer_;
    bool is_in_power_mode_;                           // True when in power mode for increased speed
    static constexpr double ANIMATION_DURATION = 0.1; // 100ms per frame

    void handle_tunnel_wrapping(const Maze &maze);
    void update_animation(double delta_time);
    std::tuple<int, int, bool, bool> get_sprite_info() const;
};

/**
 * Ghost AI behavior types
 */
enum class GhostAIType
{
    RANDOM_PATROL, ///< Wanders randomly, locks on when close to Pacman
    AMBUSHER       ///< Aims ahead of Pacman, chases when close
};

/**
 * Ghost class - AI-controlled enemy that chases Pac-Man
 * Uses directional sprites with animation and basic pathfinding
 */
class Ghost : public Entity
{
public:
    // Animation states for ghost movement
    enum class AnimationState
    {
        FRAME_1 = 0,
        FRAME_2 = 1
    };

    Ghost(double start_x, double start_y, SpriteSheet *sheet, const std::string &palette = "RED_BLUE_WHITE", GhostAIType ai_type = GhostAIType::RANDOM_PATROL);

    void update(const Maze &maze, double delta_time = 1.0 / 60.0) override;
    void update(const Maze &maze, double pacman_x, double pacman_y, double delta_time = 1.0 / 60.0);
    void update(const Maze &maze, double pacman_x, double pacman_y, direction_t pacman_dir, double delta_time = 1.0 / 60.0);
    void draw() const override;
    double get_current_speed() const override;

    // State management methods
    void set_scared_mode();
    void set_caught_mode();
    void set_chasing_mode();
    bool is_scared() const;
    bool is_caught() const;
    bool can_interact() const; // Returns false during COOLDOWN (immune to collisions)
    GhostState get_state() const;

    // Score popup management
    void update_score_popup(double delta_time);
    void trigger_score_popup(double x, double y);

private:
    SpriteSheet *sheet_;
    AnimationState anim_state_;
    double anim_timer_;
    double target_x_, target_y_;                      // Pac-Man's position to chase
    double escape_target_x_, escape_target_y_;        // Target position when running away
    static constexpr double ANIMATION_DURATION = 0.2; // 200ms per frame

    // Ghost state management
    GhostState current_state_;
    double scared_timer_;
    double scared_duration_actual_;                  // Actual scared duration based on difficulty
    double home_x_, home_y_;                         // Center spawn position
    static constexpr double SCARED_DURATION = 15.0;  // 15 seconds in scared mode
    static constexpr double WARNING_TIME = 3.0;      // Flash when 3 seconds remaining
    double flash_timer_;                             // Timer for flashing animation
    double cooldown_timer_;                          // Timer for cooldown after returning home
    static constexpr double COOLDOWN_DURATION = 3.0; // 3 seconds cooldown at home

    // AI behavior
    GhostAIType ai_type_;                                 // Type of AI behavior
    static constexpr double LOCK_ON_DISTANCE = 150.0;     // Distance to lock onto Pacman
    static constexpr double AMBUSH_DISTANCE = 200.0;      // Distance ahead to target for ambusher
    static constexpr double ESCAPE_DISTANCE = 100.0;      // Distance to flee from Pacman when caught
    direction_t random_target_dir_;                       // Current random direction for patrol
    double random_dir_timer_;                             // Timer to change random direction
    static constexpr double RANDOM_DIR_CHANGE_TIME = 2.0; // Change direction every 2 seconds

    // Score popup state
    bool show_score_popup_;
    double popup_timer_;
    double popup_x_, popup_y_;
    static constexpr double POPUP_DURATION = 1.0; // Show popup for 1 second

    // Helper methods
    void choose_direction_towards_target(const Maze &maze);
    void choose_direction_random_patrol(const Maze &maze);
    void choose_direction_ambush(const Maze &maze, direction_t pacman_dir);
    void choose_direction_away_from_target(const Maze &maze);
    void find_escape_target(const Maze &maze);
    void move_towards_home(const Maze &maze);
    direction_t get_opposite_direction(direction_t dir) const;
    std::pair<double, double> get_non_portal_distance(double target_x, double target_y) const;
    bool can_move_in_direction(const Maze &maze, direction_t dir) const;
    bool is_at_intersection(const Maze &maze) const;           // Check if ghost can turn (at corner/intersection)
    bool should_recalculate_direction(const Maze &maze) const; // Check if direction needs updating
    void update_animation(double delta_time);
    void handle_tunnel_wrapping(const Maze &maze);
    std::tuple<int, int, bool, bool> get_sprite_info() const;
};

/**
 * Fruit class - Bonus fruit that appears periodically
 * Awards bonus points when collected
 */
class Fruit
{
public:
    Fruit(SpriteSheet *sheet);

    // Update fruit state (spawn timer, visibility timer)
    void update(double delta_time, const Maze &maze);

    // Draw the fruit if it's active and visible
    void draw() const;

    // Check if fruit should be collected
    bool check_collision(double pacman_x, double pacman_y);

    // Getters
    bool is_active() const { return is_active_; }
    bool is_showing_score_popup() const { return show_score_popup_; }
    int get_points() const { return FRUIT_POINTS; }
    double get_popup_x() const { return popup_x_; }
    double get_popup_y() const { return popup_y_; }

private:
    void spawn_fruit(const Maze &maze);

    SpriteSheet *sheet_;
    double x_, y_;             // Position in pixels
    int fruit_type_;           // 0-3 for different fruit sprites
    bool is_active_;           // Whether fruit is currently visible
    double spawn_timer_;       // Time until next fruit spawn
    double visible_timer_;     // Time fruit has been visible
    bool show_score_popup_;    // Whether to show score popup
    double popup_timer_;       // Time score popup has been shown
    double popup_x_, popup_y_; // Position of score popup

    static constexpr double SPAWN_INTERVAL = 30.0;     // Spawn every 30 seconds
    static constexpr double VISIBLE_DURATION = 20.0;   // Visible for 20 seconds
    static constexpr double POPUP_DURATION = 1.0;      // Show popup for 1 second
    static constexpr int FRUIT_POINTS = 200;           // Points awarded
    static constexpr double COLLISION_DISTANCE = 15.0; // Collection distance
};
