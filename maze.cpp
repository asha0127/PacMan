#include "maze.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace MazeConfig;

// ============== Maze Implementation ==============

Maze::Maze(int level) : level_(level)
{
    // Construct the filename based on level number
    std::string filename = "Resources/Maps/level" + std::to_string(level) + ".csv";

    // Load maze from CSV
    if (!load_from_csv(filename))
    {
        // Fallback to hardcoded layout if CSV loading fails
        std::cerr << "Failed to load level " << level << ", using fallback layout" << std::endl;
        maze_layout_ = {
            // row 0
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            // row 1
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            // row 2
            {1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
            // row 3
            {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
            // row 4
            {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1},
            // row 5
            {1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1},
            // row 6
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            // row 7
            {1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1},
            // row 8
            {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1},
            // row 9
            {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
            // row 10
            {1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
            // row 11
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            // row 12
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    }
}

bool Maze::load_from_csv(const std::string &filename)
{
    std::ofstream debug("maze_debug.txt");
    debug << "Attempting to load: " << filename << std::endl;

    std::ifstream file(filename);
    if (!file.is_open())
    {
        debug << "Failed to open maze file: " << filename << std::endl;
        std::cerr << "Failed to open maze file: " << filename << std::endl;
        debug.close();
        return false;
    }

    debug << "File opened successfully!" << std::endl;

    maze_layout_.clear();
    std::string line;
    int line_number = 0;

    debug << "Starting to read lines..." << std::endl;
    debug.flush();

    while (std::getline(file, line))
    {
        line_number++;

        // Remove UTF-8 BOM from first line if present
        if (line_number == 1 && line.length() >= 3)
        {
            if ((unsigned char)line[0] == 0xEF &&
                (unsigned char)line[1] == 0xBB &&
                (unsigned char)line[2] == 0xBF)
            {
                line = line.substr(3);
                debug << "Removed UTF-8 BOM from first line" << std::endl;
                debug.flush();
            }
        }

        debug << "Read line " << line_number << " with length: " << line.length() << std::endl;
        debug.flush();

        // Skip empty lines or lines with only whitespace
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            debug << "Skipping empty line " << line_number << std::endl;
            debug.flush();
            continue;
        }

        debug << "Parsing line " << line_number << std::endl;
        debug << "Line content: [" << line << "]" << std::endl;
        debug.flush();

        std::vector<int> row;

        debug << "Creating stringstream..." << std::endl;
        debug.flush();

        std::stringstream ss(line);

        debug << "Stringstream created, starting cell loop..." << std::endl;
        debug.flush();

        std::string cell;
        int cell_num = 0;

        while (std::getline(ss, cell, ','))
        {
            cell_num++;
            debug << "  Cell " << cell_num << ": '" << cell << "'" << std::endl;
            debug.flush();

            // Trim whitespace from cell - safer version
            size_t start = cell.find_first_not_of(" \t\r\n");
            if (start == std::string::npos)
            {
                debug << "  Cell " << cell_num << " is all whitespace, skipping" << std::endl;
                debug.flush();
                continue; // Cell is all whitespace
            }

            size_t end = cell.find_last_not_of(" \t\r\n");
            cell = cell.substr(start, end - start + 1);

            debug << "  Cell " << cell_num << " after trim: '" << cell << "'" << std::endl;
            debug.flush();

            // Skip empty cells
            if (cell.empty())
            {
                debug << "  Cell " << cell_num << " is empty after trim, skipping" << std::endl;
                debug.flush();
                continue;
            }

            // Convert string to int
            try
            {
                int value = std::stoi(cell);
                row.push_back(value);
                debug << "  Cell " << cell_num << " converted to: " << value << std::endl;
                debug.flush();
            }
            catch (const std::exception &e)
            {
                debug << "  ERROR converting cell " << cell_num << ": " << e.what() << std::endl;
                debug.flush();
            }
        }

        if (!row.empty())
        {
            debug << "Line " << line_number << ": parsed " << row.size() << " columns" << std::endl;
            debug.flush();
            maze_layout_.push_back(row);
        }
    }

    debug << "Finished reading. Total rows parsed: " << maze_layout_.size() << std::endl;
    debug.flush();

    file.close();

    // Validate maze dimensions
    if (maze_layout_.size() != MAZE_ROWS)
    {
        debug << "Invalid maze row count: " << maze_layout_.size() << " (expected " << MAZE_ROWS << ")" << std::endl;
        debug.flush();
        debug.close();
        return false;
    }

    for (size_t i = 0; i < maze_layout_.size(); i++)
    {
        if (maze_layout_[i].size() != MAZE_COLS)
        {
            debug << "Invalid maze column count on row " << i << ": " << maze_layout_[i].size() << " (expected " << MAZE_COLS << ")" << std::endl;
            debug.flush();
            debug.close();
            return false;
        }
    }

    debug << "Maze loaded successfully!" << std::endl;
    debug.flush();
    debug.close();
    return true;
}

// ============== Token Implementation ==============

Token::Token(int row, int col) : row_(row), col_(col), collected_(false) {}

double Token::get_x() const
{
    return Maze::get_cell_center_x(col_);
}

double Token::get_y() const
{
    return Maze::get_cell_center_y(row_);
}

void Token::draw() const
{
    if (!collected_)
    {
        fill_circle(COLOR_YELLOW, get_x(), get_y(), TOKEN_RADIUS);
    }
}

// ============== PowerPellet Implementation ==============

PowerPellet::PowerPellet(int row, int col) : row_(row), col_(col), collected_(false) {}

double PowerPellet::get_x() const
{
    return Maze::get_cell_center_x(col_);
}

double PowerPellet::get_y() const
{
    return Maze::get_cell_center_y(row_);
}

void PowerPellet::draw() const
{
    if (!collected_)
    {
        // Draw pulsing power pellet
        static int pulse_timer = 0;
        pulse_timer++;
        double pulse = 1.0 + 0.3 * sin(pulse_timer * 0.2);
        double radius = POWER_PELLET_RADIUS * pulse;

        fill_circle(COLOR_YELLOW, get_x(), get_y(), radius);
        draw_circle(COLOR_WHITE, get_x(), get_y(), radius + 1);
    }
}

// ============== GameState Implementation ==============

GameState::GameState() : score_(0), tokens_collected_(0), total_tokens_(0), token_just_collected_(false) {}

void GameState::add_token(int row, int col)
{
    tokens_.emplace_back(row, col);
    total_tokens_++;
}

void GameState::add_power_pellet(int row, int col)
{
    power_pellets_.emplace_back(row, col);
}

int GameState::count_collected_power_pellets() const
{
    int count = 0;
    for (const auto &power_pellet : power_pellets_)
    {
        if (power_pellet.is_collected())
            count++;
    }
    return count;
}

bool GameState::check_token_collection(double pacman_x, double pacman_y)
{
    bool collected_any = false;

    for (auto &token : tokens_)
    {
        if (!token.is_collected())
        {
            double dx = pacman_x - token.get_x();
            double dy = pacman_y - token.get_y();
            double distance = sqrt(dx * dx + dy * dy);

            if (distance <= COLLECTION_DISTANCE)
            {
                token.collect();
                add_score(TOKEN_POINTS);
                tokens_collected_++;
                collected_any = true;
                token_just_collected_ = true; // Set flag for sound effect
            }
        }
    }

    return collected_any;
}

bool GameState::check_power_pellet_collection(double pacman_x, double pacman_y)
{
    bool collected_any = false;

    for (auto &power_pellet : power_pellets_)
    {
        if (!power_pellet.is_collected())
        {
            double dx = pacman_x - power_pellet.get_x();
            double dy = pacman_y - power_pellet.get_y();
            double distance = sqrt(dx * dx + dy * dy);

            if (distance <= COLLECTION_DISTANCE)
            {
                power_pellet.collect();
                add_score(POWER_PELLET_POINTS);
                // Power pellet collected - ghosts will be set to scared in game loop
                collected_any = true;
            }
        }
    }

    return collected_any;
}

void GameState::draw_tokens() const
{
    for (const auto &token : tokens_)
    {
        token.draw();
    }
}

void GameState::draw_power_pellets() const
{
    for (const auto &power_pellet : power_pellets_)
    {
        power_pellet.draw();
    }
}

void GameState::draw_score() const
{
    std::string score_text = "SCORE: " + std::to_string(score_);
    draw_text(score_text, COLOR_WHITE, "Arial", 24, 10, 10);

    std::string tokens_text = "PELLETS: " + std::to_string(tokens_collected_) + "/" + std::to_string(total_tokens_);
    draw_text(tokens_text, COLOR_WHITE, "Arial", 16, 10, 40);
}

// ============== Maze Implementation ==============

color Maze::get_level_color() const
{
    switch (level_)
    {
    case 1:
        return COLOR_BLUE;
    case 2:
        return COLOR_GREEN;
    case 3:
        return COLOR_PURPLE;
    case 4:
        return COLOR_RED;
    case 5:
        return COLOR_ORANGE; // Using orange for level 5
    default:
        return COLOR_BLUE; // Default fallback
    }
}

void Maze::draw() const
{
    color wall_color = get_level_color();
    for (int r = 0; r < MAZE_ROWS; r++)
    {
        for (int c = 0; c < MAZE_COLS; c++)
        {
            if (maze_layout_[r][c] == 1)
            {
                fill_rectangle(wall_color, c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            }
        }
    }
}

bool Maze::is_empty(int row, int col) const
{
    return is_valid_position(row, col) && maze_layout_[row][col] == 0;
}

bool Maze::can_move_to(double x, double y) const
{
    const double radius = CELL_SIZE / 2.0 - PACMAN_RADIUS_OFFSET;

    const int left_col = static_cast<int>((x - radius) / CELL_SIZE);
    const int right_col = static_cast<int>((x + radius) / CELL_SIZE);
    const int top_row = static_cast<int>((y - radius) / CELL_SIZE);
    const int bottom_row = static_cast<int>((y + radius) / CELL_SIZE);

    return is_empty_or_tunnel(top_row, left_col) &&
           is_empty_or_tunnel(top_row, right_col) &&
           is_empty_or_tunnel(bottom_row, left_col) &&
           is_empty_or_tunnel(bottom_row, right_col);
}

double Maze::get_cell_center_x(int col)
{
    return col * CELL_SIZE + CELL_SIZE / 2.0;
}

double Maze::get_cell_center_y(int row)
{
    return row * CELL_SIZE + CELL_SIZE / 2.0;
}

void Maze::initialize_tokens(GameState &game_state, int spawn_row, int spawn_col) const
{
    for (int r = 0; r < MAZE_ROWS; r++)
    {
        for (int c = 0; c < MAZE_COLS; c++)
        {
            if (is_empty(r, c))
            {
                // Skip the spawn position and immediate surrounding area
                int dr = abs(r - spawn_row);
                int dc = abs(c - spawn_col);
                if (dr <= 1 && dc <= 1)
                {
                    continue; // Skip spawn area
                }
                game_state.add_token(r, c);
            }
        }
    }
}

void Maze::initialize_power_pellets(GameState &game_state) const
{
    // Place power pellets in the four corners of open areas
    std::vector<std::pair<int, int>> power_pellet_positions = {
        {1, 1}, {1, MAZE_COLS - 2}, {MAZE_ROWS - 2, 1}, {MAZE_ROWS - 2, MAZE_COLS - 2}};

    for (const auto &pos : power_pellet_positions)
    {
        int r = pos.first;
        int c = pos.second;
        // Only place power pellet if position is empty
        if (is_empty(r, c))
        {
            game_state.add_power_pellet(r, c);
        }
    }
}

bool Maze::is_empty_or_tunnel(int row, int col) const
{
    // If we're on the tunnel row (row 6), allow positions outside bounds
    if (row == 6)
    {
        // Allow positions just outside the left and right edges
        if (col < 0 || col >= MAZE_COLS)
        {
            return true; // Treat as empty for tunnel wrapping
        }
    }

    // For all other cases, use normal bounds checking
    return is_empty(row, col);
}

bool Maze::is_valid_position(int row, int col) const
{
    return row >= 0 && row < MAZE_ROWS && col >= 0 && col < MAZE_COLS;
}

std::pair<int, int> Maze::find_spawn_position(const Maze &maze, int target_row, int target_col)
{
    // Check if target position is already empty
    if (maze.is_empty(target_row, target_col))
    {
        return {target_row, target_col};
    }

    // Search outward in a spiral pattern
    const int max_radius = std::max(MAZE_ROWS, MAZE_COLS);

    for (int radius = 1; radius <= max_radius; radius++)
    {
        for (int dr = -radius; dr <= radius; dr++)
        {
            for (int dc = -radius; dc <= radius; dc++)
            {
                // Only check cells on the perimeter of current radius
                if (abs(dr) != radius && abs(dc) != radius)
                    continue;

                const int row = target_row + dr;
                const int col = target_col + dc;

                if (row >= 0 && row < MAZE_ROWS && col >= 0 && col < MAZE_COLS &&
                    maze.is_empty(row, col))
                {
                    return {row, col};
                }
            }
        }
    }

    // Fallback to center if no empty cell found (shouldn't happen)
    return {MAZE_ROWS / 2, MAZE_COLS / 2};
}