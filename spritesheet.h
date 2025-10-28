#pragma once
#include "splashkit.h"
#include "direction.h"
#include <string>
#include <optional>
#include <iostream>
#include <map>

class SpriteSheet
{
public:
    // border_v: vertical border between palette cells (px)
    // border_h: horizontal border between palette cells (px)
    // sprite_border: border between sprites (px)
    // tile_border: border between tiles and sprites (px, vertical only)
    SpriteSheet(const std::string &bitmap_name, const std::string &file_path, int frame_w, int frame_h,
                int border_v = 4, int border_h = 3, int sprite_border = 1, int tile_border = 2);
    // Draw using palette name and local coordinates
    // If trim is true, draw a (frame_w-1) x (frame_h-1) portion (removes rightmost column and bottom row)
    void draw_sprite_at_pixel(const std::string &palette_name, int local_col, int local_row, double x, double y, double scale = 1.0, bool flip_x = false, bool flip_y = false, bool trim = false);
    int frame_width() const { return _frame_w; }
    int frame_height() const { return _frame_h; }

private:
    bitmap _sheet;
    // optional pre-flipped spritesheet (flipped in both axes). When present
    // we'll use this for left/up sprite lookups instead of performing a
    // runtime flip.
    bitmap _flipped_sheet = nullptr;
    int _frame_w, _frame_h;
    int _border_v, _border_h, _sprite_border, _tile_border;
#ifdef SPLASHKIT_RENDER_TARGETS
    // Persistent temporary bitmap used for extracting a full 16x16 frame
    // and then drawing a trimmed 15x15 portion from it. This block is
    // guarded because some SplashKit builds do not expose render-target
    // APIs; define SPLASHKIT_RENDER_TARGETS when your build supports them.
    bitmap _temp16;
    void ensure_temp_bitmap();
#endif
};

// Configurable nudges (pixels) applied after trimming. You can tweak these
// values to shift trimmed sprites visually without changing drawing logic.
constexpr int TRIM_NUDGE_NORMAL_X = 6;  // Move right by 2 pixels
constexpr int TRIM_NUDGE_NORMAL_Y = 5;  // Move down by 2 pixels
constexpr int TRIM_NUDGE_FLIPPED_X = 6; // Keep consistent for flipped sprites
constexpr int TRIM_NUDGE_FLIPPED_Y = 5;

// ...existing code...

// ...existing code...

// ...existing code...

// Palette cell color mapping (single definition, correct order)
struct PaletteCellInfo
{
    int row, col;
    const char *name;
};

constexpr PaletteCellInfo PALETTE_CELL_MAP[] = {
    {0, 0, "RED_BLUE_WHITE"},
    {1, 0, "RED_WHITE_GREEN"},
    {2, 0, "RED_PEACH_WHITE"},
    {3, 0, "WHITE_GREEN_TEAL"},
    {0, 1, "PINK_BLUE_WHTE"},
    {1, 1, "BLACK_BLUE_WHITE"},
    {2, 1, "PINK_BLUE_WHITE"},
    {3, 1, "YELLOW_RED_BLUE"},
    {0, 2, "SKY_BLUE_WHITE"},
    {1, 2, "YELLOW_PINK_SKY"},
    {2, 2, "WHITE_ORANGE_RED"},
    {3, 2, "WHITE_BLUE_YELLOW"},
    {0, 3, "ORANGE_BLUE_WHITE"},
    {1, 3, "BLUE_BLACK_PEACH"},
    {2, 3, "WHITE_GREEN_RED"},
    {3, 3, "PEACH_BLACK_WHITE"},
    {0, 4, "PEACH_BLUE_GREEN"},
    {1, 4, "WHITE_BLACK_PEACH"},
    {2, 4, "TAN_GREEN_ORANGE"},
    {3, 4, nullptr}};

constexpr const char *PACMAN_PALETTE_NAME = "YELLOW_PINK_SKY";

// Ghost sprite coordinates (col, row) in the spritesheet
namespace GhostSprites
{
    // Scared ghost sprites (when Pac-Man has power pellet)
    constexpr int SCARED_1_COL = 5, SCARED_1_ROW = 0;
    constexpr int SCARED_2_COL = 5, SCARED_2_ROW = 1;

    // Directional ghost sprites with animation frames
    constexpr int RIGHT_1_COL = 0, RIGHT_1_ROW = 0;
    constexpr int RIGHT_2_COL = 1, RIGHT_2_ROW = 0;
    constexpr int DOWN_1_COL = 2, DOWN_1_ROW = 0;
    constexpr int DOWN_2_COL = 3, DOWN_2_ROW = 0;
    constexpr int LEFT_1_COL = 4, LEFT_1_ROW = 0;
    constexpr int LEFT_2_COL = 5, LEFT_2_ROW = 0;
    constexpr int UP_1_COL = 6, UP_1_ROW = 0;
    constexpr int UP_2_COL = 7, UP_2_ROW = 0;
}
// Sprite utility function declarations (implemented in spritesheet.cpp)
int get_palette_cell_col(const char *name);
int get_palette_cell_row(const char *name);
void get_sprite_pixel_coords(const std::string &palette_name, int local_col, int local_row, int &px, int &py);
