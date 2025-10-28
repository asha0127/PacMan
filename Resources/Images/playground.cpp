#include "splashkit.h"
#include <string>
#include <sstream>

// --- Sprite sheet layout constants ---
const int TILE_W = 8, TILE_H = 8;
const int TILES_X = 22, TILES_Y = 9;
const int SPRITE_W = 16, SPRITE_H = 16;
const int SPRITES_X = 10, SPRITES_Y = 6;
const int BORDER_Y = 2; // 2px border between tiles and sprites
const int PALETTE_CELLS_X = 5, PALETTE_CELLS_Y = 4;
const int PALETTE_CELL_W = TILES_X * TILE_W; // 176px
// Add 1px border between sprite rows, except after last row
const int PALETTE_CELL_H = TILES_Y * TILE_H + BORDER_Y + SPRITES_Y * SPRITE_H + (SPRITES_Y - 1) * 1 + 11;
const int SPRITE_OFFSET_X = 0;                           // sprites start at left edge of cell
const int SPRITE_OFFSET_Y = TILES_Y * TILE_H + BORDER_Y; // below tiles + border
const int PALETTE_CELL_GAP_X = 24;                       // horizontal gap between palette cells
const int WINDOW_W = 480;
const int WINDOW_H = 480;
const double SPRITE_SCALE = 3.0;

int main()
{
    open_window("Sprite Playground", WINDOW_W, WINDOW_H);
    bitmap sheet = load_bitmap("pacman_spritemap", "pacman_spritemap.png");

    int cell_col = 0, cell_row = 0;
    int sprite_col = 0, sprite_row = 0;

    while (!window_close_requested("Sprite Playground"))
    {
        process_events();
        // Navigation
        if (key_typed(RIGHT_KEY))
        {
            sprite_col++;
            if (sprite_col >= SPRITES_X)
            {
                sprite_col = 0;
                cell_col = (cell_col + 1) % PALETTE_CELLS_X;
            }
        }
        if (key_typed(LEFT_KEY))
        {
            sprite_col--;
            if (sprite_col < 0)
            {
                sprite_col = SPRITES_X - 1;
                cell_col = (cell_col - 1 + PALETTE_CELLS_X) % PALETTE_CELLS_X;
            }
        }
        if (key_typed(DOWN_KEY))
        {
            sprite_row++;
            if (sprite_row >= SPRITES_Y)
            {
                sprite_row = 0;
                cell_row = (cell_row + 1) % PALETTE_CELLS_Y;
            }
        }
        if (key_typed(UP_KEY))
        {
            sprite_row--;
            if (sprite_row < 0)
            {
                sprite_row = SPRITES_Y - 1;
                cell_row = (cell_row - 1 + PALETTE_CELLS_Y) % PALETTE_CELLS_Y;
            }
        }

        // --- Calculate global pixel position of the sprite ---
        int cell_origin_x = cell_col * (PALETTE_CELL_W + PALETTE_CELL_GAP_X);
        int cell_origin_y = cell_row * PALETTE_CELL_H;
        int src_x = cell_origin_x + SPRITE_OFFSET_X + sprite_col * (SPRITE_W + 1) + 1; // +1 for left border, +1 per col
        int src_y = cell_origin_y + SPRITE_OFFSET_Y + sprite_row * (SPRITE_H + 1) + 9; // +9 for top border, +1 per row

        clear_screen(COLOR_BLACK);
        // Draw the selected sprite in the center, scaled up for easier viewing
        draw_bitmap(sheet, WINDOW_W / 2 - (SPRITE_W * SPRITE_SCALE) / 2, WINDOW_H / 2 - (SPRITE_H * SPRITE_SCALE) / 2,
                    option_part_bmp(src_x, src_y, SPRITE_W, SPRITE_H, option_scale_bmp(SPRITE_SCALE, SPRITE_SCALE)));

        // Display coordinates and info
        std::ostringstream oss;
        oss << "cell: (" << cell_col << "," << cell_row << ")  sprite: (" << sprite_col << "," << sprite_row << ")  px: (" << src_x << "," << src_y << ")";
        draw_text(oss.str(), COLOR_WHITE, 10, 20);

        refresh_screen(60);
    }
    return 0;
}
