#include "menu.h"
#include "maze.h"
#include "spritesheet.h"
#include "sound_manager.h"
#include <string>
#include <fstream>
#include <algorithm>

/**
 * @file menu.cpp
 * @brief Implementation of the Menu class
 *
 * This file contains the implementation of menu rendering and input handling.
 */

using namespace MazeConfig;

// Helper: approximate center X position for text drawn with draw_text
static int center_text_x(const std::string &text, int font_size, int window_width)
{
    return window_width / 2 - static_cast<int>(text.length()) * font_size / 4;
}

/**
 * @brief Constructor - initializes menu with default state
 */
Menu::Menu()
    : current_state_(MenuState::MAIN_MENU),
      selected_option_(0),
      should_start_game_(false),
      last_input_time_(0.0),
      selected_palette_index_(0),
      sprite_sheet_(nullptr),
      sound_manager_(nullptr),
      velentina_mode_(false),
      difficulty_level_(DifficultyLevel::MEDIUM),
      selected_difficulty_option_(1), // Default to MEDIUM (index 1)
      endless_mode_(true),
      selected_level_(1),
      name_entry_complete_(false),
      pending_score_(0),
      name_cursor_position_(0)
{
    name_letters_[0] = 'A';
    name_letters_[1] = 'A';
    name_letters_[2] = 'A';
    load_high_scores();
}

/**
 * @brief Process keyboard input for menu navigation
 */
void Menu::handle_input()
{
    double current_time = current_ticks() / 1000.0;

    // Input cooldown to prevent rapid navigation
    if (current_time - last_input_time_ < INPUT_COOLDOWN)
    {
        return;
    }

    switch (current_state_)
    {
    case MenuState::MAIN_MENU:
        handle_main_menu_input();
        break;
    case MenuState::LEVEL_SELECT:
        handle_level_select_input();
        break;
    case MenuState::DIFFICULTY:
        handle_difficulty_input();
        break;
    case MenuState::HIGH_SCORES:
        handle_high_scores_input();
        break;
    case MenuState::SETTINGS:
        handle_settings_input();
        break;
    case MenuState::NAME_ENTRY:
        handle_name_entry_input();
        break;
    case MenuState::IN_GAME:
        // No menu input while in game
        break;
    }
}

/**
 * @brief Handle input for the main menu
 */
void Menu::handle_main_menu_input()
{
    bool input_handled = false;

    // Navigate up
    if (key_typed(UP_KEY))
    {
        selected_option_--;
        if (selected_option_ < 0)
        {
            selected_option_ = static_cast<int>(MainMenuOption::COUNT) - 1;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Navigate down
    else if (key_typed(DOWN_KEY))
    {
        selected_option_++;
        if (selected_option_ >= static_cast<int>(MainMenuOption::COUNT))
        {
            selected_option_ = 0;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Select option
    else if (key_typed(SPACE_KEY))
    {
        // Play selection sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT1_SOUND_NAME);
        }

        switch (static_cast<MainMenuOption>(selected_option_))
        {
        case MainMenuOption::PLAY_ENDLESS:
            endless_mode_ = true;
            selected_level_ = 1; // Start at level 1 for endless mode
            should_start_game_ = true;
            current_state_ = MenuState::IN_GAME;
            break;
        case MainMenuOption::PLAY_LEVEL_SELECT:
            endless_mode_ = false;
            current_state_ = MenuState::LEVEL_SELECT;
            selected_option_ = 0; // Reset for level selection
            break;
        case MainMenuOption::DIFFICULTY:
            current_state_ = MenuState::DIFFICULTY;
            break;
        case MainMenuOption::HIGH_SCORES:
            current_state_ = MenuState::HIGH_SCORES;
            break;
        case MainMenuOption::SETTINGS:
            current_state_ = MenuState::SETTINGS;
            break;
        default:
            break;
        }
        input_handled = true;
    }

    if (input_handled)
    {
        last_input_time_ = current_ticks() / 1000.0;
    }
}

/**
 * @brief Handle input for the difficulty screen
 */
void Menu::handle_difficulty_input()
{
    bool input_handled = false;

    // Navigate up
    if (key_typed(UP_KEY))
    {
        selected_difficulty_option_--;
        if (selected_difficulty_option_ < 0)
        {
            selected_difficulty_option_ = static_cast<int>(DifficultyLevel::COUNT) - 1;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Navigate down
    else if (key_typed(DOWN_KEY))
    {
        selected_difficulty_option_++;
        if (selected_difficulty_option_ >= static_cast<int>(DifficultyLevel::COUNT))
        {
            selected_difficulty_option_ = 0;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Select difficulty
    else if (key_typed(SPACE_KEY))
    {
        // Play selection sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT1_SOUND_NAME);
        }

        difficulty_level_ = static_cast<DifficultyLevel>(selected_difficulty_option_);
        current_state_ = MenuState::MAIN_MENU;
        selected_option_ = static_cast<int>(MainMenuOption::DIFFICULTY);
        input_handled = true;
    }
    // Go back without changing
    else if (key_typed(ESCAPE_KEY))
    {
        current_state_ = MenuState::MAIN_MENU;
        selected_option_ = static_cast<int>(MainMenuOption::DIFFICULTY);
        input_handled = true;
    }

    if (input_handled)
    {
        last_input_time_ = current_ticks() / 1000.0;
    }
}

/**
 * @brief Handle input for the high scores screen
 */
void Menu::handle_high_scores_input()
{
    // For now, just allow going back to main menu
    if (key_typed(SPACE_KEY) || key_typed(ESCAPE_KEY))
    {
        // Play selection sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT1_SOUND_NAME);
        }

        current_state_ = MenuState::MAIN_MENU;
        selected_option_ = static_cast<int>(MainMenuOption::HIGH_SCORES);
        last_input_time_ = current_ticks() / 1000.0;
    }
}

/**
 * @brief Handle input for the level selection screen
 */
void Menu::handle_level_select_input()
{
    bool input_handled = false;

    // Navigate up
    if (key_typed(UP_KEY))
    {
        selected_option_--;
        if (selected_option_ < 0)
        {
            selected_option_ = 4; // 5 levels (0-4)
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Navigate down
    else if (key_typed(DOWN_KEY))
    {
        selected_option_++;
        if (selected_option_ >= 5)
        {
            selected_option_ = 0;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Select level
    else if (key_typed(SPACE_KEY))
    {
        // Play selection sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT1_SOUND_NAME);
        }

        selected_level_ = selected_option_ + 1; // Convert 0-4 to 1-5
        should_start_game_ = true;
        current_state_ = MenuState::IN_GAME;
        input_handled = true;
    }
    // Go back without selecting
    else if (key_typed(ESCAPE_KEY))
    {
        current_state_ = MenuState::MAIN_MENU;
        selected_option_ = static_cast<int>(MainMenuOption::PLAY_LEVEL_SELECT);
        input_handled = true;
    }

    if (input_handled)
    {
        last_input_time_ = current_ticks() / 1000.0;
    }
}

/**
 * @brief Render the current menu screen
 */
void Menu::render()
{
    switch (current_state_)
    {
    case MenuState::MAIN_MENU:
        render_main_menu();
        break;
    case MenuState::LEVEL_SELECT:
        render_level_select_screen();
        break;
    case MenuState::DIFFICULTY:
        render_difficulty_screen();
        break;
    case MenuState::HIGH_SCORES:
        render_high_scores_screen();
        break;
    case MenuState::SETTINGS:
        render_settings_screen();
        break;
    case MenuState::NAME_ENTRY:
        render_name_entry_screen();
        break;
    case MenuState::IN_GAME:
        // Don't render menu while in game
        break;
    }
}

/**
 * @brief Handle input for the settings screen
 */
void Menu::handle_settings_input()
{
    bool input_handled = false;

    // Available Pac-Man color palettes
    static const char *pacman_palettes[] = {
        "YELLOW_PINK_SKY", // Default yellow
        "RED_BLUE_WHITE",
        "PINK_BLUE_WHITE",
        "ORANGE_BLUE_WHITE",
        "SKY_BLUE_WHITE",
        "PEACH_BLUE_GREEN",
        "WHITE_GREEN_RED",
        "TAN_GREEN_ORANGE"};
    static const int num_palettes = 9;

    // Navigate left - previous palette
    if (key_typed(LEFT_KEY))
    {
        selected_palette_index_--;
        if (selected_palette_index_ < 0)
        {
            selected_palette_index_ = num_palettes - 1;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Navigate right - next palette
    else if (key_typed(RIGHT_KEY))
    {
        selected_palette_index_++;
        if (selected_palette_index_ >= num_palettes)
        {
            selected_palette_index_ = 0;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Navigate up/down - toggle Velentina Mode
    else if (key_typed(UP_KEY) || key_typed(DOWN_KEY))
    {
        velentina_mode_ = !velentina_mode_;
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Go back to main menu
    else if (key_typed(SPACE_KEY) || key_typed(ESCAPE_KEY))
    {
        // Play selection sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT1_SOUND_NAME);
        }

        current_state_ = MenuState::MAIN_MENU;
        selected_option_ = static_cast<int>(MainMenuOption::SETTINGS);
        input_handled = true;
    }

    if (input_handled)
    {
        last_input_time_ = current_ticks() / 1000.0;
    }
}

/**
 * @brief Render the settings screen
 */
void Menu::render_settings_screen()
{
    // Available Pac-Man color palettes (must match handle_settings_input)
    static const char *pacman_palettes[] = {
        "YELLOW_PINK_SKY", // Default yellow
        "RED_BLUE_WHITE",
        "PINK_BLUE_WHITE",
        "ORANGE_BLUE_WHITE",
        "SKY_BLUE_WHITE",
        "PEACH_BLUE_GREEN",
        "WHITE_ORANGE_RED",
        "WHITE_GREEN_RED",
        "TAN_GREEN_ORANGE"};
    static const int num_palettes = 9;

    const int window_width = MAZE_COLS * CELL_SIZE;
    const int window_height = MAZE_ROWS * CELL_SIZE;

    clear_screen(COLOR_BLACK);

    // Title
    const std::string title = "SETTINGS";
    const int title_size = 40;
    draw_text(title, COLOR_YELLOW, "Arial", title_size,
              center_text_x(title, title_size, window_width),
              window_height / 5 - 15);

    // Color selector section
    const std::string color_label = "PAC-MAN COLOR:";
    const int label_size = 25;
    draw_text(color_label, COLOR_WHITE, "Arial", label_size,
              center_text_x(color_label, label_size, window_width),
              window_height / 2 - 95);

    // Draw Pac-Man sprite preview if sprite sheet is available
    if (sprite_sheet_ != nullptr)
    {
        const char *current_palette = pacman_palettes[selected_palette_index_];

        // Draw Pac-Man sprite (open mouth, facing right)
        sprite_sheet_->draw_sprite_at_pixel(current_palette, 3, 6,
                                            window_width / 2,
                                            window_height / 2 - 15,
                                            3.0, false, false, true);

        // Draw left arrow
        draw_text("<", COLOR_YELLOW, "Arial", 40,
                  center_text_x(std::string("<"), 40, window_width) - 80,
                  window_height / 2 - 35);

        // Draw right arrow
        draw_text(">", COLOR_YELLOW, "Arial", 40,
                  center_text_x(std::string(">"), 40, window_width) + 60,
                  window_height / 2 - 35);
    }

    // Velentina Mode toggle section
    const std::string velentina_label = "VELENTINA MODE:";
    const int velentina_label_size = 25;
    draw_text(velentina_label, COLOR_WHITE, "Arial", velentina_label_size,
              center_text_x(velentina_label, velentina_label_size, window_width),
              window_height / 2 + 85);

    // Display toggle state
    std::string toggle_state = velentina_mode_ ? "ON" : "OFF";
    color toggle_color = velentina_mode_ ? COLOR_GREEN : COLOR_RED;
    const int toggle_size = 30;
    draw_text(toggle_state, toggle_color, "Arial", toggle_size,
              center_text_x(toggle_state, toggle_size, window_width),
              window_height / 2 + 125);

    // Navigation instructions
    const std::string nav_text = "LEFT/RIGHT: Change color  |  UP/DOWN: Toggle Velentina Mode";
    const int nav_size = 14;
    draw_text(nav_text, COLOR_GRAY, "Arial", nav_size,
              center_text_x(nav_text, nav_size, window_width),
              window_height - 115);

    // Back instruction
    const std::string back_text = "Press RED or YELLOW to go back";
    const int back_size = 16;
    draw_text(back_text, COLOR_GRAY, "Arial", back_size,
              center_text_x(back_text, back_size, window_width),
              window_height - 85);

    refresh_screen(60);
}

/**
 * @brief Render the main menu
 */
void Menu::render_main_menu()
{
    const int window_width = MAZE_COLS * CELL_SIZE;
    const int window_height = MAZE_ROWS * CELL_SIZE;

    // Clear screen with black background
    clear_screen(COLOR_BLACK);

    // Title
    const std::string title = "PAC-MAN";
    const int title_size = 60;
    const int title_y = window_height / 4 - 15;
    draw_text(title, COLOR_YELLOW, "Arial", title_size,
              center_text_x(title, title_size, window_width), title_y);

    // Menu options
    const int option_size = 30;
    const int option_start_y = window_height / 2 - 15;
    const int option_spacing = 50;

    const char *options[] = {"PLAY ENDLESS", "PLAY LEVEL SELECT", "CHANGE DIFFICULTY", "VIEW HIGH SCORES", "SETTINGS"};

    for (int i = 0; i < static_cast<int>(MainMenuOption::COUNT); i++)
    {
        color option_color = (i == selected_option_) ? COLOR_YELLOW : COLOR_WHITE;
        std::string prefix = (i == selected_option_) ? "> " : "  ";
        std::string option_text = prefix + options[i];

        int y_pos = option_start_y + i * option_spacing;
        draw_text(option_text, option_color, "Arial", option_size,
                  center_text_x(option_text, option_size, window_width), y_pos);
    }

    // Instructions
    const std::string instructions = "Use JOYSTICK to navigate, YELLOW to select";
    const int instr_size = 15;
    draw_text(instructions, COLOR_GRAY, "Arial", instr_size,
              center_text_x(instructions, instr_size, window_width),
              window_height - 20);

    refresh_screen(60);
}

/**
 * @brief Render the difficulty selection screen
 */
void Menu::render_difficulty_screen()
{
    const int window_width = MAZE_COLS * CELL_SIZE;
    const int window_height = MAZE_ROWS * CELL_SIZE;

    clear_screen(COLOR_BLACK);

    // Title
    const std::string title = "SELECT DIFFICULTY";
    const int title_size = 40;
    draw_text(title, COLOR_YELLOW, "Arial", title_size,
              center_text_x(title, title_size, window_width),
              window_height / 5 - 15);

    // Difficulty options
    const int option_size = 30;
    const int option_start_y = window_height / 2 - 75;
    const int option_spacing = 50;

    const char *difficulty_names[] = {"EASY", "MEDIUM", "HARD", "CRAZY"};
    const char *difficulty_speeds[] = {"(75% Speed)", "(100% Speed)", "(125% Speed)", "(200% Speed)"};

    for (int i = 0; i < static_cast<int>(DifficultyLevel::COUNT); i++)
    {
        color option_color = (i == selected_difficulty_option_) ? COLOR_YELLOW : COLOR_WHITE;
        std::string prefix = (i == selected_difficulty_option_) ? "> " : "  ";
        std::string option_text = prefix + difficulty_names[i];

        int y_pos = option_start_y + i * option_spacing;
        draw_text(option_text, option_color, "Arial", option_size,
                  center_text_x(option_text, option_size, window_width), y_pos);

        // Draw speed description
        const int speed_size = 18;
        std::string speed_text = difficulty_speeds[i];
        color speed_color = (i == selected_difficulty_option_) ? COLOR_YELLOW : COLOR_GRAY;
        draw_text(speed_text, speed_color, "Arial", speed_size,
                  center_text_x(speed_text, speed_size, window_width), y_pos + 28);
    }

    // Current difficulty indicator
    const std::string current_text = "Current: " + std::string(difficulty_names[static_cast<int>(difficulty_level_)]);
    const int current_size = 20;
    draw_text(current_text, COLOR_GREEN, "Arial", current_size,
              center_text_x(current_text, current_size, window_width),
              window_height - 135);

    // Instructions
    const std::string nav_text = "Use UP/DOWN arrows to select, YELLOW to confirm";
    const int nav_size = 16;
    draw_text(nav_text, COLOR_GRAY, "Arial", nav_size,
              center_text_x(nav_text, nav_size, window_width),
              window_height - 95);

    // Back instruction
    const std::string back_text = "Press RED to go back without changing";
    const int back_size = 16;
    draw_text(back_text, COLOR_GRAY, "Arial", back_size,
              center_text_x(back_text, back_size, window_width),
              window_height - 65);

    refresh_screen(60);
}

/**
 * @brief Render the level selection screen
 */
void Menu::render_level_select_screen()
{
    const int window_width = MAZE_COLS * CELL_SIZE;
    const int window_height = MAZE_ROWS * CELL_SIZE;

    clear_screen(COLOR_BLACK);

    // Title
    const std::string title = "SELECT LEVEL";
    const int title_size = 40;
    draw_text(title, COLOR_YELLOW, "Arial", title_size,
              center_text_x(title, title_size, window_width),
              window_height / 5 - 15);

    // Level options with colors
    const int option_size = 30;
    const int option_start_y = window_height / 2 - 95;
    const int option_spacing = 50;

    const char *level_names[] = {"LEVEL 1", "LEVEL 2", "LEVEL 3", "LEVEL 4", "LEVEL 5"};
    const color level_colors[] = {COLOR_BLUE, COLOR_GREEN, COLOR_PURPLE, COLOR_RED, COLOR_YELLOW};

    for (int i = 0; i < 5; i++)
    {
        color option_color = (i == selected_option_) ? level_colors[i] : COLOR_WHITE;
        std::string prefix = (i == selected_option_) ? "> " : "  ";
        std::string option_text = prefix + level_names[i];

        int y_pos = option_start_y + i * option_spacing;
        draw_text(option_text, option_color, "Arial", option_size,
                  center_text_x(option_text, option_size, window_width), y_pos);
    }

    // Instructions
    const std::string nav_text = "Use UP/DOWN arrows to select, YELLOW to confirm";
    const int nav_size = 16;
    draw_text(nav_text, COLOR_GRAY, "Arial", nav_size,
              center_text_x(nav_text, nav_size, window_width),
              window_height - 95);

    // Back instruction
    const std::string back_text = "Press RED to go back";
    const int back_size = 16;
    draw_text(back_text, COLOR_GRAY, "Arial", back_size,
              center_text_x(back_text, back_size, window_width),
              window_height - 65);

    refresh_screen(60);
}

/**
 * @brief Render the high scores screen
 */
void Menu::render_high_scores_screen()
{
    const int window_width = MAZE_COLS * CELL_SIZE;
    const int window_height = MAZE_ROWS * CELL_SIZE;

    clear_screen(COLOR_BLACK);

    // Title
    const std::string title = "HIGH SCORES";
    const int title_size = 40;
    draw_text(title, COLOR_YELLOW, "Arial", title_size,
              center_text_x(title, title_size, window_width),
              80);

    // Display top 10 scores
    if (high_scores_.empty())
    {
        const std::string message = "No scores yet!";
        const int msg_size = 25;
        draw_text(message, COLOR_WHITE, "Arial", msg_size,
                  center_text_x(message, msg_size, window_width),
                  window_height / 2 - 15);
    }
    else
    {
        const int entry_size = 22;
        const int entry_spacing = 35;
        const int start_y = 140;
        const int name_x = window_width / 2 - 150;
        const int score_x = window_width / 2 + 50;

        // Header
        draw_text("RANK", COLOR_YELLOW, "Arial", entry_size,
                  name_x - 80, start_y);
        draw_text("NAME", COLOR_YELLOW, "Arial", entry_size,
                  name_x, start_y);
        draw_text("SCORE", COLOR_YELLOW, "Arial", entry_size,
                  score_x, start_y);

        // Entries
        for (size_t i = 0; i < high_scores_.size() && i < 10; i++)
        {
            int y_pos = start_y + (i + 1) * entry_spacing;
            color entry_color = (i < 3) ? COLOR_YELLOW : COLOR_WHITE;

            // Rank
            std::string rank = std::to_string(i + 1) + ".";
            draw_text(rank, entry_color, "Arial", entry_size,
                      name_x - 80, y_pos);

            // Name
            draw_text(high_scores_[i].name, entry_color, "Arial", entry_size,
                      name_x, y_pos);

            // Score
            std::string score_str = std::to_string(high_scores_[i].score);
            draw_text(score_str, entry_color, "Arial", entry_size,
                      score_x, y_pos);
        }
    }

    // Back instruction
    const std::string back_text = "Press RED or YELLOW to go back";
    const int back_size = 16;
    draw_text(back_text, COLOR_GRAY, "Arial", back_size,
              center_text_x(back_text, back_size, window_width),
              window_height - 20);

    refresh_screen(60);
}

/**
 * @brief Get the selected Pac-Man palette name
 * @return The currently selected palette name
 */
const char *Menu::get_selected_pacman_palette() const
{
    // Available Pac-Man color palettes (must match render/input)
    static const char *pacman_palettes[] = {
        "YELLOW_PINK_SKY",
        "RED_BLUE_WHITE",
        "PINK_BLUE_WHITE",
        "ORANGE_BLUE_WHITE",
        "SKY_BLUE_WHITE",
        "PEACH_BLUE_GREEN",
        "WHITE_ORANGE_RED",
        "WHITE_GREEN_RED",
        "TAN_GREEN_ORANGE"};

    return pacman_palettes[selected_palette_index_];
}

/**
 * @brief Get the speed multiplier for the current difficulty
 * @return Speed multiplier (0.75, 1.0, 1.25, or 2.0)
 */
double Menu::get_difficulty_speed_multiplier() const
{
    switch (difficulty_level_)
    {
    case DifficultyLevel::EASY:
        return 0.75;
    case DifficultyLevel::MEDIUM:
        return 1.0;
    case DifficultyLevel::HARD:
        return 1.25;
    case DifficultyLevel::CRAZY:
        return 2.0;
    default:
        return 1.0;
    }
}

/**
 * @brief Load high scores from file
 */
void Menu::load_high_scores()
{
    high_scores_.clear();
    std::ifstream file("Resources/high_scores.txt");

    if (!file.is_open())
    {
        return; // File doesn't exist yet, that's okay
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Format: NAME SCORE
        if (line.length() >= 5) // At least "A 0"
        {
            size_t space_pos = line.find(' ');
            if (space_pos != std::string::npos)
            {
                std::string name = line.substr(0, space_pos);
                int score = std::stoi(line.substr(space_pos + 1));
                high_scores_.push_back({name, score});
            }
        }
    }
    file.close();

    // Sort by score (descending)
    std::sort(high_scores_.begin(), high_scores_.end(),
              [](const HighScoreEntry &a, const HighScoreEntry &b)
              {
                  return a.score > b.score;
              });
}

/**
 * @brief Save high scores to file
 */
void Menu::save_high_scores()
{
    std::ofstream file("Resources/high_scores.txt");

    if (!file.is_open())
    {
        std::cerr << "Failed to save high scores!" << std::endl;
        return;
    }

    for (const auto &entry : high_scores_)
    {
        file << entry.name << " " << entry.score << std::endl;
    }
    file.close();
}

/**
 * @brief Add a new high score entry
 */
void Menu::add_high_score(const std::string &name, int score)
{
    high_scores_.push_back({name, score});

    // Sort by score (descending)
    std::sort(high_scores_.begin(), high_scores_.end(),
              [](const HighScoreEntry &a, const HighScoreEntry &b)
              {
                  return a.score > b.score;
              });

    // Keep only top 10
    if (high_scores_.size() > 10)
    {
        high_scores_.resize(10);
    }

    save_high_scores();
}

/**
 * @brief Initiate high score name entry
 */
void Menu::start_name_entry(int score)
{
    pending_score_ = score;
    name_letters_[0] = 'A';
    name_letters_[1] = 'A';
    name_letters_[2] = 'A';
    name_cursor_position_ = 0;
    name_entry_complete_ = false;
    current_state_ = MenuState::NAME_ENTRY;
    selected_option_ = 0;
}

/**
 * @brief Handle input for the name entry screen
 */
void Menu::handle_name_entry_input()
{
    bool input_handled = false;

    // Navigate up - increment letter
    if (key_typed(UP_KEY))
    {
        name_letters_[name_cursor_position_]++;
        if (name_letters_[name_cursor_position_] > 'Z')
        {
            name_letters_[name_cursor_position_] = 'A';
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Navigate down - decrement letter
    else if (key_typed(DOWN_KEY))
    {
        name_letters_[name_cursor_position_]--;
        if (name_letters_[name_cursor_position_] < 'A')
        {
            name_letters_[name_cursor_position_] = 'Z';
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Navigate right - next letter
    else if (key_typed(RIGHT_KEY))
    {
        name_cursor_position_++;
        if (name_cursor_position_ > 2)
        {
            name_cursor_position_ = 0;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Navigate left - previous letter
    else if (key_typed(LEFT_KEY))
    {
        name_cursor_position_--;
        if (name_cursor_position_ < 0)
        {
            name_cursor_position_ = 2;
        }
        // Play navigation sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT2_SOUND_NAME);
        }
        input_handled = true;
    }
    // Confirm name entry
    else if (key_typed(SPACE_KEY) || key_typed(RETURN_KEY))
    {
        // Play selection sound
        if (sound_manager_)
        {
            play_sound_effect(SoundConfig::DOT1_SOUND_NAME);
        }

        std::string player_name = "";
        player_name += name_letters_[0];
        player_name += name_letters_[1];
        player_name += name_letters_[2];

        add_high_score(player_name, pending_score_);
        name_entry_complete_ = true;
        current_state_ = MenuState::HIGH_SCORES;
        input_handled = true;
    }

    if (input_handled)
    {
        last_input_time_ = current_ticks() / 1000.0;
    }
}

/**
 * @brief Render the name entry screen
 */
void Menu::render_name_entry_screen()
{
    const int window_width = MAZE_COLS * CELL_SIZE;
    const int window_height = MAZE_ROWS * CELL_SIZE;

    clear_screen(COLOR_BLACK);

    // Title
    const std::string title = "NEW HIGH SCORE!";
    const int title_size = 40;
    draw_text(title, COLOR_YELLOW, "Arial", title_size,
              center_text_x(title, title_size, window_width),
              window_height / 5 - 15);

    // Score display
    const std::string score_text = "SCORE: " + std::to_string(pending_score_);
    const int score_size = 30;
    draw_text(score_text, COLOR_WHITE, "Arial", score_size,
              center_text_x(score_text, score_size, window_width),
              window_height / 3);

    // Instruction
    const std::string instruction = "ENTER YOUR NAME:";
    const int instr_size = 25;
    draw_text(instruction, COLOR_WHITE, "Arial", instr_size,
              center_text_x(instruction, instr_size, window_width),
              window_height / 2 - 60);

    // Draw the three letters with cursor
    const int letter_size = 60;
    const int letter_spacing = 80;
    const int start_x = window_width / 2 - letter_spacing;
    const int letter_y = window_height / 2;

    for (int i = 0; i < 3; i++)
    {
        std::string letter(1, name_letters_[i]);
        color letter_color = (i == name_cursor_position_) ? COLOR_YELLOW : COLOR_WHITE;

        int x_pos = start_x + i * letter_spacing;
        draw_text(letter, letter_color, "Arial", letter_size,
                  x_pos, letter_y);

        // Draw cursor indicator under current letter
        if (i == name_cursor_position_)
        {
            const std::string cursor = "^";
            draw_text(cursor, COLOR_YELLOW, "Arial", 40,
                      x_pos + 10, letter_y + 60);
        }
    }

    // Instructions
    const std::string nav_text = "UP/DOWN: Change letter  |  LEFT/RIGHT: Move cursor";
    const int nav_size = 14;
    draw_text(nav_text, COLOR_GRAY, "Arial", nav_size,
              center_text_x(nav_text, nav_size, window_width),
              window_height - 50);

    // Confirm instruction
    const std::string confirm_text = "Press RED or YELLOW to confirm";
    const int confirm_size = 16;
    draw_text(confirm_text, COLOR_GRAY, "Arial", confirm_size,
              center_text_x(confirm_text, confirm_size, window_width),
              window_height - 20);

    refresh_screen(60);
}
