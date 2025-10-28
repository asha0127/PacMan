#include "game.h"
#include "splashkit.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

/**
 * @file game.cpp
 * @brief Implementation of the main Game class
 *
 * This file contains the implementation of all Game class methods,
 * including initialization, the main game loop, and game logic.
 */

using namespace GameConfig;
using namespace MazeConfig;

/**
 * @brief Constructor - initializes game with default state
 */
Game::Game()
    : running_(false), game_initialized_(false), last_time_(0.0),
      current_game_mode_(GameMode::STARTING), previous_game_mode_(GameMode::STARTING),
      current_level_(1)
{
}

/**
 * @brief Initialize all game systems and load resources
 * @return true if initialization successful, false otherwise
 */
bool Game::initialize()
{
    try
    {
        // Initialize SplashKit window and random seed
        open_window(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
        srand(static_cast<unsigned>(time(nullptr)));

        // Create core game objects (but not entities yet - those are created when user selects Play)
        maze_ = std::make_unique<Maze>(current_level_);
        sprite_sheet_ = std::make_unique<SpriteSheet>(SPRITESHEET_NAME, SPRITESHEET_PATH, 16, 16, 4, 3, 1, 2);
        game_state_ = std::make_unique<GameState>();
        sound_manager_ = std::make_unique<SoundManager>();
        menu_ = std::make_unique<Menu>();

        // Set sprite sheet for menu (for color preview)
        menu_->set_sprite_sheet(sprite_sheet_.get());

        // Set sound manager for menu (for menu sound effects)
        menu_->set_sound_manager(sound_manager_.get());

        // Initialize sound system
        if (!sound_manager_->initialize())
        {
            return false;
        }

        running_ = true;
        game_initialized_ = false;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void Game::run()
{
    last_time_ = current_ticks() / 1000.0; // Convert to seconds

    while (running_ && !window_close_requested(GameConfig::WINDOW_TITLE))
    {
        double current_time = current_ticks() / 1000.0;
        double delta_time = current_time - last_time_;
        last_time_ = current_time;

        // Cap delta time to prevent huge jumps (e.g., when debugging)
        if (delta_time > 1.0 / 30.0)
            delta_time = 1.0 / 30.0;

        // Process events first (required for key_typed to work)
        process_events();

        // Check if we're in the menu or in-game
        if (menu_->get_state() != MenuState::IN_GAME)
        {
            // Handle menu navigation
            menu_->handle_input();
            menu_->render();

            // Check if user has selected to start the game
            if (menu_->should_start_game())
            {
                menu_->reset_game_start_flag();

                // Set the starting level based on menu selection
                current_level_ = menu_->get_selected_level();

                // Recreate the maze for the selected level
                maze_ = std::make_unique<Maze>(current_level_);

                // Recreate game state (fresh start for new level/game)
                game_state_ = std::make_unique<GameState>();

                // Initialize game entities (this will use the new maze and game state)
                initialize_game_entities();

                // Mark as initialized
                game_initialized_ = true;
            }
        }
        else
        {
            // In-game loop
            handle_events();
            update(delta_time);
            render();
            refresh_screen(GameConfig::TARGET_FPS);
        }
    }
}

void Game::handle_events()
{
    process_events();
    pacman_->capture_input();
}

void Game::update(double delta_time)
{
    // Update background audio BEFORE checking game mode (must play start sound before checking if it's done)
    double pellet_percentage = calculate_pellet_percentage();
    sound_manager_->update_background_audio(current_game_mode_, pellet_percentage);

    // Update game mode (handles STARTING timer - checks if start sound finished)
    update_game_mode(delta_time);

    // Skip entity updates during STARTING state
    if (current_game_mode_ == GameMode::STARTING)
    {
        return;
    }

    // Store previous power pellet count
    static int previous_power_pellets = -1; // -1 = uninitialized
    int current_power_pellets = game_state_->count_collected_power_pellets();

    // Update entities
    pacman_->update(*maze_, *game_state_, delta_time);

    // Handle token collection sounds
    if (game_state_->was_token_just_collected())
    {
        sound_manager_->play_dot_collection_sound();
        game_state_->reset_token_collection_flag();
    }

    // Handle power pellet collection and ghost state changes
    if (previous_power_pellets != -1 && current_power_pellets > previous_power_pellets)
    {
        // Power pellet was collected - set all non-caught ghosts to scared mode
        if (!ghost1_->is_caught())
            ghost1_->set_scared_mode();
        if (!ghost2_->is_caught())
            ghost2_->set_scared_mode();
    }
    previous_power_pellets = current_power_pellets;

    // Update ghost AI
    ghost1_->update(*maze_, pacman_->get_x(), pacman_->get_y(), pacman_->get_direction(), delta_time);
    ghost2_->update(*maze_, pacman_->get_x(), pacman_->get_y(), pacman_->get_direction(), delta_time);

    // Update ghost score popups
    ghost1_->update_score_popup(delta_time);
    ghost2_->update_score_popup(delta_time);

    // Update fruit
    fruit_->update(delta_time, *maze_);

    // Check fruit collision
    if (fruit_->check_collision(pacman_->get_x(), pacman_->get_y()))
    {
        game_state_->add_score(fruit_->get_points());
        play_sound_effect(SoundConfig::FRUIT_SOUND_NAME);
    }

    // Handle ghost collisions
    handle_ghost_collisions();

    // Check for game end conditions
    if (check_win_condition())
    {
        current_game_mode_ = GameMode::VICTORY;
        sound_manager_->stop_all_background_sounds();

        // Play cutscene sound
        sound_manager_->play_cutscene_sound();

        // Wait for cutscene sound to finish (approximately 4.3 seconds based on typical cutscene.wav length)
        delay(4300);

        // Advance to next level or end game
        advance_to_next_level();
    }
}

void Game::render()
{
    clear_screen(COLOR_BLACK);

    // Draw game objects
    maze_->draw();
    game_state_->draw_tokens();
    game_state_->draw_power_pellets();
    fruit_->draw();
    pacman_->draw();
    ghost1_->draw();
    ghost2_->draw();
    game_state_->draw_score();
}

void Game::initialize_game_entities()
{
    // Find optimal spawn positions for entities
    const int pacman_target_row = MAZE_ROWS / 2 + 3;
    const int pacman_target_col = MAZE_COLS / 2;
    const auto [pacman_spawn_row, pacman_spawn_col] = Maze::find_spawn_position(*maze_, pacman_target_row, pacman_target_col);

    const int ghost1_target_row = MAZE_ROWS / 2 - 3;
    const int ghost1_target_col = MAZE_COLS / 2;
    const auto [ghost1_spawn_row, ghost1_spawn_col] = Maze::find_spawn_position(*maze_, ghost1_target_row, ghost1_target_col);

    const int ghost2_target_row = MAZE_ROWS / 2 + 1;
    const int ghost2_target_col = MAZE_COLS / 2 + 5;
    const auto [ghost2_spawn_row, ghost2_spawn_col] = Maze::find_spawn_position(*maze_, ghost2_target_row, ghost2_target_col);

    // Create game entities
    // Use the palette selected in the settings menu
    const char *selected_palette = menu_->get_selected_pacman_palette();

    // Get difficulty speed multiplier
    double speed_multiplier = menu_->get_difficulty_speed_multiplier();

    // Set sound base path based on Velentina Mode setting
    if (menu_->is_velentina_mode_enabled())
    {
        sound_manager_->set_sound_base_path("Resources/Sounds/Velentina/");
    }
    else
    {
        sound_manager_->set_sound_base_path("Resources/Sounds/Normal/");
    }
    // Unload existing sounds and reinitialize with the new base path
    sound_manager_->unload_all_sounds();
    sound_manager_->initialize();

    pacman_ = std::make_unique<Pacman>(
        Maze::get_cell_center_x(pacman_spawn_col),
        Maze::get_cell_center_y(pacman_spawn_row),
        sprite_sheet_.get(),
        selected_palette);
    pacman_->set_speed_multiplier(speed_multiplier);

    ghost1_ = std::make_unique<Ghost>(
        Maze::get_cell_center_x(ghost1_spawn_col),
        Maze::get_cell_center_y(ghost1_spawn_row),
        sprite_sheet_.get(),
        "RED_BLUE_WHITE",
        GhostAIType::RANDOM_PATROL);
    ghost1_->set_speed_multiplier(speed_multiplier);

    ghost2_ = std::make_unique<Ghost>(
        Maze::get_cell_center_x(ghost2_spawn_col),
        Maze::get_cell_center_y(ghost2_spawn_row),
        sprite_sheet_.get(),
        "PINK_BLUE_WHTE",
        GhostAIType::AMBUSHER);
    ghost2_->set_speed_multiplier(speed_multiplier);

    // Initialize fruit
    fruit_ = std::make_unique<Fruit>(sprite_sheet_.get());

    // Initialize game elements
    maze_->initialize_tokens(*game_state_, pacman_spawn_row, pacman_spawn_col);
    maze_->initialize_power_pellets(*game_state_);

    // Stop all background sounds to reset sound state
    sound_manager_->stop_all_background_sounds();

    // Reset game mode to STARTING
    current_game_mode_ = GameMode::STARTING;
    previous_game_mode_ = GameMode::STARTING;

    game_initialized_ = true;
}

// === Helper Method Implementations ===

/**
 * @brief Handle collisions between Pac-Man and ghosts
 */
void Game::handle_ghost_collisions()
{
    // Check collision with ghost1
    double dx1 = pacman_->get_x() - ghost1_->get_x();
    double dy1 = pacman_->get_y() - ghost1_->get_y();
    double distance1 = sqrt(dx1 * dx1 + dy1 * dy1);

    if (distance1 <= COLLISION_DISTANCE && ghost1_->can_interact())
    {
        if (ghost1_->is_scared())
        {
            // Pac-Man catches scared ghost
            ghost1_->set_caught_mode();
            game_state_->add_score(400);
            // Show 400-point popup at ghost's location
            ghost1_->trigger_score_popup(ghost1_->get_x(), ghost1_->get_y());
            sound_manager_->play_ghost_eat_sound();
            sound_manager_->play_ghost_retreat_sound();
        }
        else if (!ghost1_->is_caught())
        {
            // Game over - Pacman caught by ghost
            current_game_mode_ = GameMode::GAME_OVER;
            sound_manager_->stop_all_background_sounds();
            play_sound_effect(SoundConfig::DIE_SOUND_NAME);
            pacman_->play_dying_animation(maze_.get(), game_state_.get(), ghost1_.get(), ghost2_.get());
            draw_text("GAME OVER!", COLOR_RED, "Arial", 48,
                      WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2);
            refresh_screen(TARGET_FPS);
            delay(GAME_OVER_DISPLAY_TIME);

            // Check if endless mode to trigger high score entry
            if (menu_->is_endless_mode())
            {
                int final_score = game_state_->get_score();
                menu_->start_name_entry(final_score);
                game_initialized_ = false;
            }
            else
            {
                // Single level mode - just return to menu
                menu_->reset_game_start_flag();
                menu_->set_state(MenuState::MAIN_MENU);
                game_initialized_ = false;
            }
            return;
        }
    }

    // Check collision with ghost2
    double dx2 = pacman_->get_x() - ghost2_->get_x();
    double dy2 = pacman_->get_y() - ghost2_->get_y();
    double distance2 = sqrt(dx2 * dx2 + dy2 * dy2);

    if (distance2 <= COLLISION_DISTANCE && ghost2_->can_interact())
    {
        if (ghost2_->is_scared())
        {
            // Pac-Man catches scared ghost
            ghost2_->set_caught_mode();
            game_state_->add_score(400);
            // Show 400-point popup at ghost's location
            ghost2_->trigger_score_popup(ghost2_->get_x(), ghost2_->get_y());
            sound_manager_->play_ghost_eat_sound();
            sound_manager_->play_ghost_retreat_sound();
        }
        else if (!ghost2_->is_caught())
        {
            // Game over - Pacman caught by ghost
            current_game_mode_ = GameMode::GAME_OVER;
            sound_manager_->stop_all_background_sounds();
            play_sound_effect(SoundConfig::DIE_SOUND_NAME);
            pacman_->play_dying_animation(maze_.get(), game_state_.get(), ghost1_.get(), ghost2_.get());
            draw_text("GAME OVER!", COLOR_RED, "Arial", 48,
                      WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2);
            refresh_screen(TARGET_FPS);
            delay(GAME_OVER_DISPLAY_TIME);

            // Check if endless mode to trigger high score entry
            if (menu_->is_endless_mode())
            {
                int final_score = game_state_->get_score();
                menu_->start_name_entry(final_score);
                game_initialized_ = false;
            }
            else
            {
                // Single level mode - just return to menu
                menu_->reset_game_start_flag();
                menu_->set_state(MenuState::MAIN_MENU);
                game_initialized_ = false;
            }
            return;
        }
    }
}

/**
 * @brief Check if the win condition has been met
 * @return true if player has won, false otherwise
 */
bool Game::check_win_condition()
{
    return game_state_->all_tokens_collected();
}

/**
 * @brief Calculate the percentage of pellets remaining
 * @return Percentage of pellets left (0-100)
 */
double Game::calculate_pellet_percentage() const
{
    int total = game_state_->get_total_tokens();
    int collected = game_state_->get_tokens_collected();
    return total > 0 ? 100.0 * (total - collected) / total : 100.0;
}

void Game::update_game_mode(double delta_time)
{
    previous_game_mode_ = current_game_mode_;

    // Handle STARTING state - wait for start.wav to finish playing
    if (current_game_mode_ == GameMode::STARTING)
    {
        // Check if start sound is no longer playing (finished)
        if (!sound_effect_playing(SoundConfig::START_SOUND_NAME))
        {
            current_game_mode_ = GameMode::NORMAL;
        }
    }
    else
    {
        current_game_mode_ = determine_current_game_mode();
    }

    // Update Pacman's power mode for speed boost
    pacman_->set_power_mode(current_game_mode_ == GameMode::POWER_MODE);
}

GameMode Game::determine_current_game_mode() const
{
    // Check if game is over or won first
    if (!running_)
    {
        return GameMode::GAME_OVER;
    }

    if (game_state_->all_tokens_collected())
    {
        return GameMode::VICTORY;
    }

    // Check if any ghosts are scared (power mode active)
    if (ghost1_->is_scared() || ghost2_->is_scared())
    {
        return GameMode::POWER_MODE;
    }

    // Default to normal mode (ghosts chasing Pac-Man)
    return GameMode::NORMAL;
}
void Game::advance_to_next_level()
{
    // Save current score before clearing game state
    int current_score = game_state_->get_score();

    // Check if we're in endless mode or single level mode
    if (!menu_->is_endless_mode())
    {
        // Single level mode - return to menu after completing
        current_game_mode_ = GameMode::VICTORY;
        sound_manager_->stop_all_background_sounds();

        // Show victory message briefly
        draw_text("LEVEL COMPLETE!", COLOR_GREEN, "Arial", 48,
                  WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2);
        refresh_screen(TARGET_FPS);
        delay(2000); // 2 second delay

        // Return to menu
        menu_->set_state(MenuState::MAIN_MENU);
        game_initialized_ = false;
        return;
    }

    // Endless mode - increment level, loop back to 1 after level 5
    current_level_++;
    if (current_level_ > 5)
    {
        current_level_ = 1; // Loop back to level 1
    }

    // Create new maze for the next level
    maze_ = std::make_unique<Maze>(current_level_);

    // Find optimal spawn positions for entities
    const int pacman_target_row = MAZE_ROWS / 2 + 3;
    const int pacman_target_col = MAZE_COLS / 2;
    const auto [pacman_spawn_row, pacman_spawn_col] = Maze::find_spawn_position(*maze_, pacman_target_row, pacman_target_col);

    const int ghost1_target_row = MAZE_ROWS / 2 - 3;
    const int ghost1_target_col = MAZE_COLS / 2;
    const auto [ghost1_spawn_row, ghost1_spawn_col] = Maze::find_spawn_position(*maze_, ghost1_target_row, ghost1_target_col);

    const int ghost2_target_row = MAZE_ROWS / 2 + 1;
    const int ghost2_target_col = MAZE_COLS / 2 + 5;
    const auto [ghost2_spawn_row, ghost2_spawn_col] = Maze::find_spawn_position(*maze_, ghost2_target_row, ghost2_target_col);

    // Reset entities to their spawn positions
    pacman_->set_position(Maze::get_cell_center_x(pacman_spawn_col), Maze::get_cell_center_y(pacman_spawn_row));
    ghost1_->set_position(Maze::get_cell_center_x(ghost1_spawn_col), Maze::get_cell_center_y(ghost1_spawn_row));
    ghost2_->set_position(Maze::get_cell_center_x(ghost2_spawn_col), Maze::get_cell_center_y(ghost2_spawn_row));

    // Reset ghosts to chasing mode
    ghost1_->set_chasing_mode();
    ghost2_->set_chasing_mode();

    // Recreate fruit for the new level
    fruit_ = std::make_unique<Fruit>(sprite_sheet_.get());

    // Clear and reinitialize game state for new level
    game_state_ = std::make_unique<GameState>();

    // Initialize tokens for the new level
    maze_->initialize_tokens(*game_state_, pacman_spawn_row, pacman_spawn_col);
    maze_->initialize_power_pellets(*game_state_);

    // Restore the score from previous level
    game_state_->add_score(current_score);

    // Reset game mode to STARTING
    current_game_mode_ = GameMode::STARTING;
    previous_game_mode_ = GameMode::STARTING;
}
