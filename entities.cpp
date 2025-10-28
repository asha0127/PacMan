#include "entities.h"
#include <cmath>
#include <vector>
#include <algorithm>

using namespace MazeConfig;

// ============== Entity Implementation ==============

Entity::Entity(double start_x, double start_y, const std::string &palette)
    : x_(start_x), y_(start_y), dir_(DIR_NONE), desired_dir_(DIR_NONE), palette_(palette), speed_multiplier_(1.0) {}

void Entity::set_position(double x, double y)
{
    x_ = x;
    y_ = y;
}

void Entity::update(const Maze &maze, double delta_time)
{
    move_in_direction(maze, delta_time);
}

void Entity::move_in_direction(const Maze &maze, double delta_time)
{
    const int col = static_cast<int>(x_ / CELL_SIZE);
    const int row = static_cast<int>(y_ / CELL_SIZE);
    const double center_x = Maze::get_cell_center_x(col);
    const double center_y = Maze::get_cell_center_y(row);

    // Try to change direction if desired direction differs from current
    if (desired_dir_ != DIR_NONE && desired_dir_ != dir_)
    {
        attempt_direction_change(maze, row, col, center_x, center_y);
    }

    // Move in current direction
    attempt_movement(maze, center_x, center_y, delta_time);
}

void Entity::attempt_direction_change(const Maze &maze, int row, int col, double center_x, double center_y)
{
    int next_col = col;
    int next_row = row;

    get_next_cell(desired_dir_, next_row, next_col);

    const bool aligned = is_aligned_for_direction(desired_dir_, center_x, center_y);

    if (aligned && maze.is_empty(next_row, next_col))
    {
        align_to_grid(desired_dir_, center_x, center_y);
        dir_ = desired_dir_;
    }
}

void Entity::attempt_movement(const Maze &maze, double center_x, double center_y, double delta_time)
{
    if (dir_ == DIR_NONE)
        return;

    const auto [test_x, test_y] = get_next_position(dir_, delta_time);

    if (maze.can_move_to(test_x, test_y))
    {
        x_ = test_x;
        y_ = test_y;
        snap_to_grid_if_close(center_x, center_y);
    }
    else
    {
        dir_ = DIR_NONE; // Stop if can't move
    }
}

void Entity::get_next_cell(direction_t direction, int &row, int &col)
{
    switch (direction)
    {
    case DIR_LEFT:
        col--;
        break;
    case DIR_RIGHT:
        col++;
        break;
    case DIR_UP:
        row--;
        break;
    case DIR_DOWN:
        row++;
        break;
    default:
        break;
    }
}

bool Entity::is_aligned_for_direction(direction_t direction, double center_x, double center_y) const
{
    switch (direction)
    {
    case DIR_LEFT:
    case DIR_RIGHT:
        return fabs(y_ - center_y) < ALIGNMENT_TOLERANCE;
    case DIR_UP:
    case DIR_DOWN:
        return fabs(x_ - center_x) < ALIGNMENT_TOLERANCE;
    default:
        return false;
    }
}

void Entity::align_to_grid(direction_t direction, double center_x, double center_y)
{
    switch (direction)
    {
    case DIR_LEFT:
    case DIR_RIGHT:
        y_ = center_y;
        break;
    case DIR_UP:
    case DIR_DOWN:
        x_ = center_x;
        break;
    default:
        break;
    }
}

std::pair<double, double> Entity::get_next_position(direction_t direction, double delta_time) const
{
    double test_x = x_;
    double test_y = y_;
    double movement = get_current_speed() * delta_time; // pixels per second * seconds = pixels

    switch (direction)
    {
    case DIR_LEFT:
        test_x -= movement;
        break;
    case DIR_RIGHT:
        test_x += movement;
        break;
    case DIR_UP:
        test_y -= movement;
        break;
    case DIR_DOWN:
        test_y += movement;
        break;
    default:
        break;
    }

    return {test_x, test_y};
}

void Entity::snap_to_grid_if_close(double center_x, double center_y)
{
    if ((dir_ == DIR_LEFT || dir_ == DIR_RIGHT) &&
        fabs(y_ - center_y) < ALIGNMENT_TOLERANCE)
    {
        y_ = center_y;
    }

    if ((dir_ == DIR_UP || dir_ == DIR_DOWN) &&
        fabs(x_ - center_x) < ALIGNMENT_TOLERANCE)
    {
        x_ = center_x;
    }
}

// ============== Pacman Implementation ==============

Pacman::Pacman(double start_x, double start_y, SpriteSheet *sheet, const std::string &palette)
    : Entity(start_x, start_y, palette), sheet_(sheet), anim_state_(AnimationState::OPEN), anim_timer_(0), is_in_power_mode_(false) {}

void Pacman::capture_input()
{
    if (key_down(LEFT_KEY))
        set_desired_direction(DIR_LEFT);
    else if (key_down(RIGHT_KEY))
        set_desired_direction(DIR_RIGHT);
    else if (key_down(UP_KEY))
        set_desired_direction(DIR_UP);
    else if (key_down(DOWN_KEY))
        set_desired_direction(DIR_DOWN);
}

void Pacman::update(const Maze &maze, double delta_time)
{
    Entity::update(maze, delta_time);
    handle_tunnel_wrapping(maze);
    update_animation(delta_time);
}

void Pacman::update(const Maze &maze, GameState &game_state, double delta_time)
{
    Entity::update(maze, delta_time);
    handle_tunnel_wrapping(maze);
    update_animation(delta_time);

    // Check for token collection
    game_state.check_token_collection(get_x(), get_y());

    // Check for power pellet collection
    game_state.check_power_pellet_collection(get_x(), get_y());
}

void Pacman::draw() const
{
    if (!sheet_)
        return;

    const auto [sprite_col, sprite_row, flip_x, flip_y] = get_sprite_info();

    sheet_->draw_sprite_at_pixel(get_palette(), sprite_col, sprite_row,
                                 get_x(), get_y(), SPRITE_SCALE, flip_x, flip_y, true);
}

void Pacman::handle_tunnel_wrapping(const Maze &maze)
{
    const int row = static_cast<int>(floor(get_y() / CELL_SIZE));
    const int col = static_cast<int>(floor(get_x() / CELL_SIZE));

    // Wrap from left edge to right edge
    if (col < 0)
    {
        const double new_x = (row >= 0 && row < MAZE_ROWS && maze.is_empty(row, MAZE_COLS - 1)) ? Maze::get_cell_center_x(MAZE_COLS - 1) : Maze::get_cell_center_x(0);
        set_position(new_x, get_y());
    }
    // Wrap from right edge to left edge
    else if (col >= MAZE_COLS)
    {
        const double new_x = (row >= 0 && row < MAZE_ROWS && maze.is_empty(row, 0)) ? Maze::get_cell_center_x(0) : Maze::get_cell_center_x(MAZE_COLS - 1);
        set_position(new_x, get_y());
    }
}

void Pacman::update_animation(double delta_time)
{
    anim_timer_ += delta_time;
    if (anim_timer_ > ANIMATION_DURATION)
    {
        anim_state_ = static_cast<AnimationState>((static_cast<int>(anim_state_) + 1) % 3);
        anim_timer_ = 0.0;
    }
}

std::tuple<int, int, bool, bool> Pacman::get_sprite_info() const
{
    const direction_t current_dir = get_direction();
    const int anim_frame = static_cast<int>(anim_state_);

    // Sprite coordinates for different animation frames
    // Frame 0 (open): col 3, Frame 1 (closing): col 4, Frame 2 (closed): col 5
    const int sprite_col = 3 + anim_frame;

    switch (current_dir)
    {
    case DIR_RIGHT:
        return {sprite_col, 6, false, false};

    case DIR_LEFT:
        return {sprite_col, 6, true, false};

    case DIR_DOWN:
        // Special case: closed state uses row 6 instead of 7
        return {sprite_col, (anim_state_ == AnimationState::CLOSED) ? 6 : 7, false, false};

    case DIR_UP:
        // Special case: closed state uses row 6 instead of 7, with flip_y
        return {sprite_col, (anim_state_ == AnimationState::CLOSED) ? 6 : 7, false, true};

    default:                         // DIR_NONE
        return {5, 6, false, false}; // Default to closed mouth
    }
}

double Pacman::get_current_speed() const
{
    // 10% speed boost during power mode, multiplied by difficulty multiplier
    double base_speed = MazeConfig::SPEED * speed_multiplier_;
    return is_in_power_mode_ ? base_speed * 1.1 : base_speed;
}

void Pacman::play_dying_animation(const Maze *maze, const GameState *game_state, const Ghost *ghost1, const Ghost *ghost2)
{
    // Dying animation sprite coordinates
    const int dying_coords[12][2] = {
        {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}};

    for (int i = 0; i < 12; ++i)
    {
        clear_screen(COLOR_BLACK);

        // Draw complete game scene during animation
        if (maze)
        {
            maze->draw();
        }

        if (game_state)
        {
            game_state->draw_tokens();
            game_state->draw_power_pellets();
            game_state->draw_score();
        }

        // Draw ghosts if provided
        if (ghost1)
        {
            ghost1->draw();
        }
        if (ghost2)
        {
            ghost2->draw();
        }

        // Draw Pacman dying frame
        if (sheet_)
        {
            sheet_->draw_sprite_at_pixel(get_palette(), dying_coords[i][0], dying_coords[i][1],
                                         get_x(), get_y(), SPRITE_SCALE, false, false, true);
        }

        refresh_screen(60);
        delay(80); // ~80ms per frame for smooth animation
    }
}

// ============== Ghost Implementation ==============

Ghost::Ghost(double start_x, double start_y, SpriteSheet *sheet, const std::string &palette, GhostAIType ai_type)
    : Entity(start_x, start_y, palette), sheet_(sheet), anim_state_(AnimationState::FRAME_1),
      anim_timer_(0), target_x_(0), target_y_(0), escape_target_x_(0), escape_target_y_(0),
      current_state_(GhostState::CHASING), scared_timer_(0.0), flash_timer_(0.0),
      cooldown_timer_(0.0),
      home_x_(Maze::get_cell_center_x(MazeConfig::MAZE_COLS / 2)),
      home_y_(Maze::get_cell_center_y(MazeConfig::MAZE_ROWS / 2)),
      ai_type_(ai_type), random_target_dir_(DIR_RIGHT), random_dir_timer_(0.0),
      show_score_popup_(false), popup_timer_(0.0), popup_x_(0.0), popup_y_(0.0) {}

void Ghost::update(const Maze &maze, double delta_time)
{
    // Base update for movement
    Entity::update(maze, delta_time);
    handle_tunnel_wrapping(maze);
    update_animation(delta_time);
}

void Ghost::update(const Maze &maze, double pacman_x, double pacman_y, double delta_time)
{
    // Call the overloaded version with DIR_NONE (no direction info)
    update(maze, pacman_x, pacman_y, DIR_NONE, delta_time);
}

void Ghost::update(const Maze &maze, double pacman_x, double pacman_y, direction_t pacman_dir, double delta_time)
{
    target_x_ = pacman_x;
    target_y_ = pacman_y;

    // Update scared timer if in scared mode
    if (current_state_ == GhostState::SCARED)
    {
        scared_timer_ += delta_time; // Use actual delta time
        if (scared_timer_ >= scared_duration_actual_)
        {
            current_state_ = GhostState::CHASING;
            scared_timer_ = 0.0;
            flash_timer_ = 0.0; // Reset flash timer
        }

        // Update flash timer for warning animation
        flash_timer_ += delta_time;
    }

    // Choose movement based on current state and AI type
    switch (current_state_)
    {
    case GhostState::CHASING:
    {
        // Calculate distance to Pacman (needed for AI and force movement)
        double distance_to_pacman = sqrt(pow(target_x_ - get_x(), 2) + pow(target_y_ - get_y(), 2));

        // Only recalculate direction at intersections or when blocked
        if (should_recalculate_direction(maze))
        {
            // AI-specific behavior
            if (ai_type_ == GhostAIType::RANDOM_PATROL)
            {
                // Random patrol: wander randomly until close enough, then lock on
                if (distance_to_pacman < LOCK_ON_DISTANCE)
                {
                    // Close enough - lock on and chase
                    choose_direction_towards_target(maze);
                }
                else
                {
                    // Too far - wander randomly
                    choose_direction_random_patrol(maze);
                }
            }
            else if (ai_type_ == GhostAIType::AMBUSHER)
            {
                // Ambusher: aim ahead of Pacman until close, then chase
                if (distance_to_pacman < LOCK_ON_DISTANCE)
                {
                    // Close enough - chase directly
                    choose_direction_towards_target(maze);
                }
                else
                {
                    // Aim ahead of Pacman
                    choose_direction_ambush(maze, pacman_dir);
                }
            }
        }

        // Normal movement with collision detection
        Entity::update(maze, delta_time);

        // If ghost is very close to target and not moving, force movement
        if (distance_to_pacman < 25.0 && get_direction() == DIR_NONE)
        {
            // Force the ghost to move directly towards Pacman
            double dx = target_x_ - get_x();
            double dy = target_y_ - get_y();
            double movement = get_current_speed() * delta_time;

            if (std::abs(dx) > std::abs(dy) && std::abs(dx) > 1.0)
            {
                set_position(get_x() + (dx > 0 ? movement : -movement), get_y());
            }
            else if (std::abs(dy) > 1.0)
            {
                set_position(get_x(), get_y() + (dy > 0 ? movement : -movement));
            }
        }

        handle_tunnel_wrapping(maze);
        break;
    }
    case GhostState::SCARED:
    {
        // Only recalculate direction at intersections or when blocked
        if (should_recalculate_direction(maze))
        {
            // Calculate distance to Pacman for smart fleeing behavior
            double distance_to_pacman = sqrt(pow(target_x_ - get_x(), 2) + pow(target_y_ - get_y(), 2));

            if (distance_to_pacman < ESCAPE_DISTANCE)
            {
                // Close to Pacman - flee directly away
                choose_direction_away_from_target(maze);
            }
            else
            {
                // Far from Pacman - move randomly
                choose_direction_random_patrol(maze);
            }
        }

        // Normal movement with collision detection
        Entity::update(maze, delta_time);
        handle_tunnel_wrapping(maze);
        break;
    }
    case GhostState::CAUGHT:
        move_towards_home(maze);
        // No collision detection - caught ghosts can pass through walls
        break;
    case GhostState::COOLDOWN:
        // Stay at home and wait for cooldown to complete
        cooldown_timer_ += delta_time;
        if (cooldown_timer_ >= COOLDOWN_DURATION)
        {
            set_chasing_mode();
        }
        break;
    }

    update_animation(delta_time);
}

void Ghost::draw() const
{
    if (!sheet_)
        return;

    const auto [sprite_col, sprite_row, flip_x, flip_y] = get_sprite_info();

    // Determine palette based on ghost state
    std::string palette_to_use = get_palette();

    if (current_state_ == GhostState::CAUGHT || current_state_ == GhostState::COOLDOWN)
    {
        // Use black/blue/white palette when caught, returning home, or cooling down
        palette_to_use = "BLACK_BLUE_WHITE";
    }
    else if (current_state_ == GhostState::SCARED)
    {
        double time_remaining = SCARED_DURATION - scared_timer_;

        // Flash when less than 3 seconds remaining
        if (time_remaining <= WARNING_TIME)
        {
            // Flash with 50% duty cycle every 0.33 seconds (3 times per second)
            // 0.167 seconds normal, 0.167 seconds flashing = 50% duty cycle
            bool should_flash = fmod(flash_timer_, 0.33) >= 0.167;
            if (should_flash)
            {
                palette_to_use = "RED_WHITE_GREEN";
            }
        }
    }

    sheet_->draw_sprite_at_pixel(palette_to_use, sprite_col, sprite_row,
                                 get_x(), get_y(), SPRITE_SCALE, flip_x, flip_y, true);

    // Draw score popup if showing
    if (show_score_popup_)
    {
        // "400" sprite is at (5, 3)
        sheet_->draw_sprite_at_pixel("WHITE_GREEN_RED", 5, 3, popup_x_, popup_y_);
    }
}

void Ghost::update_score_popup(double delta_time)
{
    if (show_score_popup_)
    {
        popup_timer_ += delta_time;
        if (popup_timer_ >= POPUP_DURATION)
        {
            show_score_popup_ = false;
            popup_timer_ = 0.0;
        }
    }
}

void Ghost::trigger_score_popup(double x, double y)
{
    show_score_popup_ = true;
    popup_timer_ = 0.0;
    popup_x_ = x;
    popup_y_ = y;
}

double Ghost::get_current_speed() const
{
    // Caught ghosts move faster to return home quickly, multiplied by difficulty multiplier
    if (current_state_ == GhostState::CAUGHT)
    {
        return MazeConfig::SPEED * speed_multiplier_ * 1.5; // 50% faster when caught
    }

    // Normal speed for chasing and scared states, with difficulty multiplier
    return MazeConfig::SPEED * speed_multiplier_;
}

void Ghost::choose_direction_towards_target(const Maze &maze)
{
    // Use non-portal distance calculation for pathfinding
    const auto [dx, dy] = get_non_portal_distance(target_x_, target_y_);
    const direction_t current_dir = get_direction();
    const direction_t opposite_dir = get_opposite_direction(current_dir);

    // List of potential directions to try, prioritized by distance to target
    std::vector<std::pair<direction_t, double>> directions;

    // Calculate how much each direction helps us get closer
    if (dx > 0)
        directions.push_back({DIR_RIGHT, std::abs(dx)});
    if (dx < 0)
        directions.push_back({DIR_LEFT, std::abs(dx)});
    if (dy > 0)
        directions.push_back({DIR_DOWN, std::abs(dy)});
    if (dy < 0)
        directions.push_back({DIR_UP, std::abs(dy)});

    // Sort by priority (larger distance difference = higher priority)
    std::sort(directions.begin(), directions.end(),
              [](const auto &a, const auto &b)
              { return a.second > b.second; });

    // Try directions in order of priority, but avoid going backward unless necessary
    for (const auto &[dir, priority] : directions)
    {
        if (dir != opposite_dir && can_move_in_direction(maze, dir))
        {
            set_desired_direction(dir);
            return;
        }
    }

    // If no forward direction works, try any valid direction (including backward)
    for (const auto &[dir, priority] : directions)
    {
        if (can_move_in_direction(maze, dir))
        {
            set_desired_direction(dir);
            return;
        }
    }

    // Last resort: try any direction that's not a wall
    const std::vector<direction_t> all_dirs = {DIR_RIGHT, DIR_LEFT, DIR_DOWN, DIR_UP};
    for (direction_t dir : all_dirs)
    {
        if (can_move_in_direction(maze, dir))
        {
            set_desired_direction(dir);
            return;
        }
    }
}

void Ghost::choose_direction_random_patrol(const Maze &maze)
{
    // Update random direction timer
    random_dir_timer_ += 1.0 / 60.0; // Approximate delta time

    const direction_t current_dir = get_direction();
    const direction_t opposite_dir = get_opposite_direction(current_dir);

    // Change direction periodically or if stuck
    if (random_dir_timer_ >= RANDOM_DIR_CHANGE_TIME || current_dir == DIR_NONE || !can_move_in_direction(maze, current_dir))
    {
        random_dir_timer_ = 0.0;

        // Try to find a new valid random direction (not backward)
        std::vector<direction_t> valid_dirs;
        const std::vector<direction_t> all_dirs = {DIR_RIGHT, DIR_LEFT, DIR_DOWN, DIR_UP};

        for (direction_t dir : all_dirs)
        {
            if (dir != opposite_dir && can_move_in_direction(maze, dir))
            {
                valid_dirs.push_back(dir);
            }
        }

        // If no forward directions available, allow backward
        if (valid_dirs.empty())
        {
            for (direction_t dir : all_dirs)
            {
                if (can_move_in_direction(maze, dir))
                {
                    valid_dirs.push_back(dir);
                }
            }
        }

        // Pick a random direction from valid options
        if (!valid_dirs.empty())
        {
            int random_index = rand() % valid_dirs.size();
            random_target_dir_ = valid_dirs[random_index];
        }
    }

    // Set the current random direction
    set_desired_direction(random_target_dir_);
}

void Ghost::choose_direction_ambush(const Maze &maze, direction_t pacman_dir)
{
    // Calculate position ahead of Pacman based on their direction
    double ambush_x = target_x_;
    double ambush_y = target_y_;

    // Project ahead by AMBUSH_DISTANCE pixels in Pacman's direction
    switch (pacman_dir)
    {
    case DIR_RIGHT:
        ambush_x += AMBUSH_DISTANCE;
        break;
    case DIR_LEFT:
        ambush_x -= AMBUSH_DISTANCE;
        break;
    case DIR_DOWN:
        ambush_y += AMBUSH_DISTANCE;
        break;
    case DIR_UP:
        ambush_y -= AMBUSH_DISTANCE;
        break;
    case DIR_NONE:
        // If Pacman isn't moving, just target their current position
        break;
    }

    // Temporarily set target to ambush position
    double original_x = target_x_;
    double original_y = target_y_;
    target_x_ = ambush_x;
    target_y_ = ambush_y;

    // Use standard pathfinding to reach ambush point
    choose_direction_towards_target(maze);

    // Restore original target
    target_x_ = original_x;
    target_y_ = original_y;
}

direction_t Ghost::get_opposite_direction(direction_t dir) const
{
    switch (dir)
    {
    case DIR_LEFT:
        return DIR_RIGHT;
    case DIR_RIGHT:
        return DIR_LEFT;
    case DIR_UP:
        return DIR_DOWN;
    case DIR_DOWN:
        return DIR_UP;
    default:
        return DIR_NONE;
    }
}

bool Ghost::can_move_in_direction(const Maze &maze, direction_t dir) const
{
    const int current_row = static_cast<int>(get_y() / CELL_SIZE);
    const int current_col = static_cast<int>(get_x() / CELL_SIZE);
    int next_row = current_row;
    int next_col = current_col;

    // Get next cell coordinates
    switch (dir)
    {
    case DIR_LEFT:
        next_col--;
        break;
    case DIR_RIGHT:
        next_col++;
        break;
    case DIR_UP:
        next_row--;
        break;
    case DIR_DOWN:
        next_row++;
        break;
    default:
        return false;
    }

    // Use maze's built-in collision detection that handles tunnels
    return maze.is_empty_or_tunnel(next_row, next_col);
}

bool Ghost::is_at_intersection(const Maze &maze) const
{
    // Check if ghost is approximately centered in a cell
    const double cell_center_x = Maze::get_cell_center_x(static_cast<int>(get_x() / CELL_SIZE));
    const double cell_center_y = Maze::get_cell_center_y(static_cast<int>(get_y() / CELL_SIZE));
    const double dx = std::abs(get_x() - cell_center_x);
    const double dy = std::abs(get_y() - cell_center_y);

    // Only consider it at intersection if close to cell center
    if (dx > 3.0 || dy > 3.0)
    {
        return false;
    }

    // Count how many directions are available (excluding the opposite of current direction)
    const direction_t current = get_direction();
    const direction_t opposite = get_opposite_direction(current);
    int available_directions = 0;

    const std::vector<direction_t> all_dirs = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
    for (direction_t dir : all_dirs)
    {
        if (dir != opposite && can_move_in_direction(maze, dir))
        {
            available_directions++;
        }
    }

    // It's an intersection if there are 2+ directions available (not counting backward)
    return available_directions >= 2;
}

bool Ghost::should_recalculate_direction(const Maze &maze) const
{
    const direction_t current = get_direction();

    // Always recalculate if not moving or can't continue in current direction
    if (current == DIR_NONE || !can_move_in_direction(maze, current))
    {
        return true;
    }

    // Otherwise, only recalculate at intersections
    return is_at_intersection(maze);
}

void Ghost::update_animation(double delta_time)
{
    anim_timer_ += delta_time;
    if (anim_timer_ > ANIMATION_DURATION)
    {
        anim_state_ = (anim_state_ == AnimationState::FRAME_1) ? AnimationState::FRAME_2 : AnimationState::FRAME_1;
        anim_timer_ = 0.0;
    }
}

void Ghost::handle_tunnel_wrapping(const Maze &maze)
{
    const int row = static_cast<int>(floor(get_y() / CELL_SIZE));
    const int col = static_cast<int>(floor(get_x() / CELL_SIZE));

    // Wrap from left edge to right edge
    if (col < 0)
    {
        const double new_x = (row >= 0 && row < MAZE_ROWS && maze.is_empty(row, MAZE_COLS - 1)) ? Maze::get_cell_center_x(MAZE_COLS - 1) : Maze::get_cell_center_x(0);
        set_position(new_x, get_y());
    }
    // Wrap from right edge to left edge
    else if (col >= MAZE_COLS)
    {
        const double new_x = (row >= 0 && row < MAZE_ROWS && maze.is_empty(row, 0)) ? Maze::get_cell_center_x(0) : Maze::get_cell_center_x(MAZE_COLS - 1);
        set_position(new_x, get_y());
    }
}

std::tuple<int, int, bool, bool> Ghost::get_sprite_info() const
{
    const direction_t current_dir = get_direction();
    const bool is_frame_2 = (anim_state_ == AnimationState::FRAME_2);

    // If ghost is scared, use scared sprites regardless of direction
    if (current_state_ == GhostState::SCARED)
    {
        return is_frame_2 ? std::make_tuple(GhostSprites::SCARED_2_COL, GhostSprites::SCARED_2_ROW, false, false) : std::make_tuple(GhostSprites::SCARED_1_COL, GhostSprites::SCARED_1_ROW, false, false);
    }

    // Normal directional sprites for chasing and caught states
    switch (current_dir)
    {
    case DIR_RIGHT:
        return is_frame_2 ? std::make_tuple(0, 1, false, false) : std::make_tuple(0, 0, false, false);

    case DIR_LEFT:
        return is_frame_2 ? std::make_tuple(0, 5, false, false) : std::make_tuple(0, 4, false, false);

    case DIR_DOWN:
        return is_frame_2 ? std::make_tuple(0, 3, false, false) : std::make_tuple(0, 2, false, false);

    case DIR_UP:
        return is_frame_2 ? std::make_tuple(0, 7, false, false) : std::make_tuple(0, 6, false, false);

    default: // DIR_NONE - default to right direction
        return std::make_tuple(0, 0, false, false);
    }
}

void Ghost::set_scared_mode()
{
    current_state_ = GhostState::SCARED;
    scared_timer_ = 0.0;
    flash_timer_ = 0.0; // Reset flash timer
    // Set actual scared duration inversely to speed multiplier
    scared_duration_actual_ = SCARED_DURATION / speed_multiplier_;
}

void Ghost::set_caught_mode()
{
    current_state_ = GhostState::CAUGHT;
}

void Ghost::set_chasing_mode()
{
    current_state_ = GhostState::CHASING;
    scared_timer_ = 0.0;   // Reset scared timer
    flash_timer_ = 0.0;    // Reset flash timer
    cooldown_timer_ = 0.0; // Reset cooldown timer
}

bool Ghost::is_scared() const
{
    return current_state_ == GhostState::SCARED;
}

bool Ghost::is_caught() const
{
    return current_state_ == GhostState::CAUGHT;
}

bool Ghost::can_interact() const
{
    // Ghost cannot interact during COOLDOWN (immune to collisions)
    return current_state_ != GhostState::COOLDOWN;
}

GhostState Ghost::get_state() const
{
    return current_state_;
}

void Ghost::choose_direction_away_from_target(const Maze &maze)
{
    // Find the best escape position (furthest from Pac-Man)
    find_escape_target(maze);

    // Now use the same pathfinding logic as chasing, but towards the escape target
    const double dx = escape_target_x_ - get_x();
    const double dy = escape_target_y_ - get_y();
    const direction_t current_dir = get_direction();
    const direction_t opposite_dir = get_opposite_direction(current_dir);

    // List of potential directions to try, prioritized by distance to escape target
    std::vector<std::pair<direction_t, double>> directions;

    // Calculate how much each direction helps us get TO the escape target
    if (dx > 0) // Target is to the right
        directions.push_back({DIR_RIGHT, std::abs(dx)});
    if (dx < 0) // Target is to the left
        directions.push_back({DIR_LEFT, std::abs(dx)});
    if (dy > 0) // Target is below
        directions.push_back({DIR_DOWN, std::abs(dy)});
    if (dy < 0) // Target is above
        directions.push_back({DIR_UP, std::abs(dy)});

    // Sort by priority (larger distance difference = higher priority)
    std::sort(directions.begin(), directions.end(),
              [](const auto &a, const auto &b)
              { return a.second > b.second; });

    // Try directions in order of priority, but avoid going backward unless necessary
    for (const auto &[dir, priority] : directions)
    {
        if (dir != opposite_dir && can_move_in_direction(maze, dir))
        {
            set_desired_direction(dir);
            return;
        }
    }

    // If no forward direction works, try any valid direction (including backward)
    for (const auto &[dir, priority] : directions)
    {
        if (can_move_in_direction(maze, dir))
        {
            set_desired_direction(dir);
            return;
        }
    }
}

void Ghost::move_towards_home(const Maze &maze)
{
    const double dx = home_x_ - get_x();
    const double dy = home_y_ - get_y();
    const double distance = sqrt(dx * dx + dy * dy);

    // If we're very close to home, snap to center and enter cooldown mode
    if (distance < 5.0)
    {
        set_position(home_x_, home_y_);
        current_state_ = GhostState::COOLDOWN;
        cooldown_timer_ = 0.0;
        return;
    }

    // Move directly towards home (caught ghosts can move through walls)
    const double speed = get_current_speed();
    const double move_distance = speed / 60.0; // Assuming 60 FPS

    // Normalize the direction vector
    const double move_x = (dx / distance) * move_distance;
    const double move_y = (dy / distance) * move_distance;

    // Update position directly (no collision detection - ghosts can pass through walls when caught)
    set_position(get_x() + move_x, get_y() + move_y);

    // Set sprite direction based on movement direction for visual feedback
    direction_t sprite_dir = DIR_NONE;
    if (std::abs(dx) > std::abs(dy))
    {
        sprite_dir = dx > 0 ? DIR_RIGHT : DIR_LEFT;
    }
    else
    {
        sprite_dir = dy > 0 ? DIR_DOWN : DIR_UP;
    }

    set_desired_direction(sprite_dir);
    dir_ = sprite_dir; // Set current direction immediately for sprite rendering
}

void Ghost::find_escape_target(const Maze &maze)
{
    double max_distance = 0.0;
    double best_x = get_x();
    double best_y = get_y();

    // Sample positions across the maze to find the furthest valid position
    const int sample_step = 2; // Check every 2 cells for performance
    for (int row = 0; row < MAZE_ROWS; row += sample_step)
    {
        for (int col = 0; col < MAZE_COLS; col += sample_step)
        {
            if (maze.is_empty(row, col))
            {
                double test_x = Maze::get_cell_center_x(col);
                double test_y = Maze::get_cell_center_y(row);

                // Calculate distance from this position to the target (Pac-Man)
                double dx = target_x_ - test_x;
                double dy = target_y_ - test_y;
                double distance = sqrt(dx * dx + dy * dy);

                if (distance > max_distance)
                {
                    max_distance = distance;
                    best_x = test_x;
                    best_y = test_y;
                }
            }
        }
    }

    escape_target_x_ = best_x;
    escape_target_y_ = best_y;
}

std::pair<double, double> Ghost::get_non_portal_distance(double target_x, double target_y) const
{
    // Calculate direct distance without considering portal wrapping
    double dx = target_x - get_x();
    double dy = target_y - get_y();

    // If the horizontal distance is more than half the maze width,
    // it likely means we're considering a portal route - ignore it by using the direct route
    const double maze_width = MAZE_COLS * CELL_SIZE;
    if (std::abs(dx) > maze_width / 2)
    {
        // Force direct non-portal path by clamping the distance calculation
        if (dx > 0)
            dx = maze_width - dx; // Go the long way around
        else
            dx = -(maze_width + dx); // Go the long way around
    }

    return {dx, dy};
}

// ============================================================================
// Fruit Implementation
// ============================================================================

Fruit::Fruit(SpriteSheet *sheet)
    : sheet_(sheet), x_(0), y_(0), fruit_type_(0), is_active_(false),
      spawn_timer_(SPAWN_INTERVAL), visible_timer_(0.0), show_score_popup_(false),
      popup_timer_(0.0), popup_x_(0), popup_y_(0)
{
}

void Fruit::update(double delta_time, const Maze &maze)
{
    // Update score popup timer if active
    if (show_score_popup_)
    {
        popup_timer_ += delta_time;
        if (popup_timer_ >= POPUP_DURATION)
        {
            show_score_popup_ = false;
            popup_timer_ = 0.0;
        }
    }

    if (is_active_)
    {
        // Count down visibility timer
        visible_timer_ += delta_time;
        if (visible_timer_ >= VISIBLE_DURATION)
        {
            // Fruit disappears after visible duration
            is_active_ = false;
            spawn_timer_ = SPAWN_INTERVAL; // Reset spawn timer
        }
    }
    else if (!show_score_popup_)
    {
        // Count down spawn timer (only if not showing popup)
        spawn_timer_ -= delta_time;
        if (spawn_timer_ <= 0.0)
        {
            spawn_fruit(maze);
            spawn_timer_ = SPAWN_INTERVAL; // Reset for next spawn
        }
    }
}

void Fruit::spawn_fruit(const Maze &maze)
{
    // Pick a random fruit type (0-3)
    fruit_type_ = rand() % 4;

    // Find a random empty cell to spawn the fruit
    std::vector<std::pair<int, int>> empty_cells;
    for (int row = 0; row < MAZE_ROWS; row++)
    {
        for (int col = 0; col < MAZE_COLS; col++)
        {
            if (maze.is_empty(row, col))
            {
                empty_cells.push_back({row, col});
            }
        }
    }

    if (!empty_cells.empty())
    {
        // Pick a random empty cell
        int random_index = rand() % empty_cells.size();
        int spawn_row = empty_cells[random_index].first;
        int spawn_col = empty_cells[random_index].second;

        // Set fruit position to cell center
        x_ = Maze::get_cell_center_x(spawn_col);
        y_ = Maze::get_cell_center_y(spawn_row);

        is_active_ = true;
        visible_timer_ = 0.0;
    }
}

void Fruit::draw() const
{
    if (!sheet_)
        return;

    // Draw fruit if active
    if (is_active_)
    {
        // Fruit sprites are at col 2, rows 0-3 (swap x/y)
        const int sprite_col = 2;
        const int sprite_row = fruit_type_;

        sheet_->draw_sprite_at_pixel("WHITE_GREEN_RED", sprite_col, sprite_row, x_, y_);
    }

    // Draw score popup if showing
    if (show_score_popup_)
    {
        // "200" sprite is at (5, 2)
        sheet_->draw_sprite_at_pixel("WHITE_GREEN_RED", 5, 2, popup_x_, popup_y_);
    }
}

bool Fruit::check_collision(double pacman_x, double pacman_y)
{
    if (!is_active_)
        return false;

    // Check distance to Pacman
    double dx = pacman_x - x_;
    double dy = pacman_y - y_;
    double distance = sqrt(dx * dx + dy * dy);

    if (distance <= COLLISION_DISTANCE)
    {
        // Fruit collected!
        is_active_ = false;

        // Show score popup at fruit location
        show_score_popup_ = true;
        popup_timer_ = 0.0;
        popup_x_ = x_;
        popup_y_ = y_;

        spawn_timer_ = SPAWN_INTERVAL; // Reset spawn timer
        return true;
    }

    return false;
}
