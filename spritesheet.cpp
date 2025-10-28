#include "spritesheet.h"
#include <cmath>

SpriteSheet::SpriteSheet(const std::string &bitmap_name, const std::string &file_path, int frame_w, int frame_h,
                         int border_v, int border_h, int sprite_border, int tile_border)
    : _frame_w(frame_w), _frame_h(frame_h),
      _border_v(border_v), _border_h(border_h), _sprite_border(sprite_border), _tile_border(tile_border)
{
    _sheet = load_bitmap(bitmap_name, file_path);
    // Try to load a pre-flipped copy (both axes flipped). This file should
    // live next to the regular spritesheet and be named with the suffix
    // "_flipped.png" (caller must ensure it exists). If not present,
    // _flipped_sheet remains nullptr and runtime flipping will be used.
    std::string flipped_path = file_path;
    // naive substitution: replace ".png" with "_flipped.png" if present
    size_t pos = flipped_path.rfind(".png");
    if (pos != std::string::npos)
        flipped_path.insert(pos, "_flipped");
    _flipped_sheet = load_bitmap((bitmap_name + "_flipped").c_str(), flipped_path.c_str());
}

void SpriteSheet::draw_sprite_at_pixel(const std::string &palette_name, int local_col, int local_row, double x, double y, double scale, bool flip_x, bool flip_y, bool trim)
{
    int px, py;
    get_sprite_pixel_coords(palette_name, local_col, local_row, px, py);

    // Diagnostic: print coords for Pacman palette to help debug 'garbage' output
    if (palette_name == std::string(PACMAN_PALETTE_NAME))
    {
    }

    // Decide which source bitmap to use. If the caller requested flipping and
    // we have a pre-flipped sheet available, use it and translate the source
    // pixel coordinates into the flipped sheet's coordinate space. Otherwise
    // draw directly from the normal sheet and apply runtime flips.
    bitmap src_sheet = _sheet;
    int src_px = px, src_py = py;
    if ((flip_x || flip_y) && _flipped_sheet != nullptr)
    {
        src_sheet = _flipped_sheet;
        // Use the actual flipped sheet dimensions to compute mirrored coords.
        int flip_w = bitmap_width(_flipped_sheet);
        int flip_h = bitmap_height(_flipped_sheet);
        // translate px/py into the flipped sheet by mirroring around the
        // full-sheet dimensions. Subtract the source rectangle's right/bottom
        // edges from the sheet size to get the mirrored top-left.
        src_px = flip_w - (px + _frame_w);
        src_py = flip_h - (py + _frame_h);
        // we've consumed the flip flags by using the flipped sheet
        flip_x = false;
        flip_y = false;
    }

    // If trimming is not requested, draw directly from the chosen src_sheet.
    if (!trim)
    {
        drawing_options opts = option_part_bmp(src_px, src_py, _frame_w, _frame_h);
        opts.scale_x = scale;
        opts.scale_y = scale;
        if (flip_x)
            opts = option_flip_x(opts);
        if (flip_y)
            opts = option_flip_y(opts);

        // Align to integer pixels using rounded half-size to avoid 1px jumps
        int half_w = (int)std::lround((_frame_w * scale) / 2.0);
        int half_h = (int)std::lround((_frame_h * scale) / 2.0);
        draw_bitmap(src_sheet, x - half_w, y - half_h, opts);
        return;
    }

#ifdef SPLASHKIT_RENDER_TARGETS
    // Preferred path: render the exact 16x16 into a persistent temp bitmap,
    // then draw the 15x15 top-left portion from that temp bitmap. This
    // preserves the exact sampling origin.
    ensure_temp_bitmap();

    // Save current render target and set temp as target (API names may vary)
    bitmap old_target = get_render_target();
    set_render_target(_temp16);

    // Clear the temp bitmap (transparent) then draw the exact 16x16 from sheet
    clear_screen(COLOR_TRANSPARENT);
    drawing_options copy_opts = option_part_bmp(src_px, src_py, _frame_w, _frame_h);
    // draw into temp at 0,0
    draw_bitmap(src_sheet, 0, 0, copy_opts);

    // Restore previous render target
    set_render_target(old_target);

    // Now draw the trimmed 15x15 from the temp bitmap to the screen
    int draw_w = std::max(1, _frame_w - 1);
    int draw_h = std::max(1, _frame_h - 1);
    drawing_options final_opts = option_part_bmp(0, 0, draw_w, draw_h);
    final_opts.scale_x = scale;
    final_opts.scale_y = scale;
    if (flip_x)
        final_opts = option_flip_x(final_opts);
    if (flip_y)
        final_opts = option_flip_y(final_opts);

    // Center using the full frame size so trimming doesn't bias the image to
    // the top-left of the logical sprite cell. Apply a 1px visual nudge
    // right and down to improve optical centering.
    int half_w = (int)std::lround((_frame_w * scale) / 2.0);
    int half_h = (int)std::lround((_frame_h * scale) / 2.0);
    // If we're drawing from the pre-flipped sheet (flipped in both axes),
    // the sprite asymmetry will be inverted; use the opposite nudge so the
    // visual center stays consistent when switching sheets.
    int nudge_x = (src_sheet == _flipped_sheet) ? TRIM_NUDGE_FLIPPED_X : TRIM_NUDGE_NORMAL_X;
    int nudge_y = (src_sheet == _flipped_sheet) ? TRIM_NUDGE_FLIPPED_Y : TRIM_NUDGE_NORMAL_Y;
    draw_bitmap(_temp16, x - half_w + nudge_x, y - half_h + nudge_y, final_opts);
#else
    // Fallback: draw a 15x15 portion directly from the spritesheet using the
    // same origin px,py. This keeps the sampling origin identical to the
    // 16x16 extraction but uses a smaller source rectangle.
    int draw_w = std::max(1, _frame_w - 1);
    int draw_h = std::max(1, _frame_h - 1);
    drawing_options opts = option_part_bmp(src_px, src_py, draw_w, draw_h);
    opts.scale_x = scale;
    opts.scale_y = scale;
    if (flip_x)
        opts = option_flip_x(opts);
    if (flip_y)
        opts = option_flip_y(opts);

    // Use the full frame size when positioning so the trimmed sprite remains
    // centered inside the original 16x16 cell. Apply a 1px nudge right/down
    // for optical alignment.
    int half_w = (int)std::lround((_frame_w * scale) / 2.0);
    int half_h = (int)std::lround((_frame_h * scale) / 2.0);
    int nudge_x = (src_sheet == _flipped_sheet) ? TRIM_NUDGE_FLIPPED_X : TRIM_NUDGE_NORMAL_X;
    int nudge_y = (src_sheet == _flipped_sheet) ? TRIM_NUDGE_FLIPPED_Y : TRIM_NUDGE_NORMAL_Y;
    draw_bitmap(src_sheet, x - half_w + nudge_x, y - half_h + nudge_y, opts);
#endif
}

#ifdef SPLASHKIT_RENDER_TARGETS
void SpriteSheet::ensure_temp_bitmap()
{
    if (_temp16 == nullptr)
    {
        // create a persistent 16x16 bitmap to use as a render target
        _temp16 = create_bitmap(_frame_w, _frame_h);
    }
}
#endif

// ============== Sprite Utility Functions ==============

int get_palette_cell_col(const char *name)
{
    for (const auto &entry : PALETTE_CELL_MAP)
    {
        if (entry.name && std::string(entry.name) == name)
            return entry.col;
    }
    return 0;
}

int get_palette_cell_row(const char *name)
{
    for (const auto &entry : PALETTE_CELL_MAP)
    {
        if (entry.name && std::string(entry.name) == name)
            return entry.row;
    }
    return 0;
}

void get_sprite_pixel_coords(const std::string &palette_name, int local_col, int local_row, int &px, int &py)
{
    int cell_col = get_palette_cell_col(palette_name.c_str());
    int cell_row = get_palette_cell_row(palette_name.c_str());

    // Sprite sheet layout constants (must match playground.cpp)
    const int SPRITE_W = 16, SPRITE_H = 16;
    const int SPRITES_X = 10, SPRITES_Y = 6;
    const int TILES_X = 22, TILES_Y = 9;
    const int TILE_W = 8, TILE_H = 8;
    const int BORDER_Y = 2;
    const int PALETTE_CELL_W = TILES_X * TILE_W; // 176px
    const int PALETTE_CELL_H = TILES_Y * TILE_H + BORDER_Y + SPRITES_Y * SPRITE_H + (SPRITES_Y - 1) * 1 + 11;
    const int SPRITE_OFFSET_X = 0;
    const int SPRITE_OFFSET_Y = TILES_Y * TILE_H + BORDER_Y;
    const int PALETTE_CELL_GAP_X = 24;

    int cell_origin_x = cell_col * (PALETTE_CELL_W + PALETTE_CELL_GAP_X);
    int cell_origin_y = cell_row * PALETTE_CELL_H;

    // Note: swap local indices so calls using (x,y) map correctly into the
    // underlying sheet layout. The first parameter is treated as the
    // horizontal index and the second as the vertical index.
    px = cell_origin_x + SPRITE_OFFSET_X + local_row * (SPRITE_W + 1) + 1;
    py = cell_origin_y + SPRITE_OFFSET_Y + local_col * (SPRITE_H + 1) + 9;
}
