// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include "keyboard.h"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

class capabilities;
class glyph_manager;
class status;

class canvas {
public:
    canvas(const capabilities& caps, glyph_manager& glyphs, status& status);
    void render();
    void refresh();
    void select_all();
    void cut_selection();
    void copy_selection();
    void delete_selection();
    void paste();
    void undo();
    bool can_paste() const;
    bool can_undo() const;
    void invert();
    void flip_horizontally();
    void flip_vertically();
    void next_char(const bool only_used = false);
    void prev_char(const bool only_used = false);
    void toggle_double_width();
    void toggle_reverse_screen();
    void process_key(const key key_press);
    void flush();

private:
    struct coord {
        int y = 0;
        int x = 0;
        auto operator<=>(const coord&) const = default;
    };
    struct size {
        int h = 0;
        int w = 0;
        auto operator<=>(const size&) const = default;
    };
    struct range {
        std::pair<int, int> y;
        std::pair<int, int> x;
        coord origin() const { return {y.first, x.first}; }
        size extent() const { return {y.second - y.first, x.second - x.first}; }
    };

    void _render();
    void _render_grid();
    void _load_char(const int start_index, const int increment, const bool only_used = false);
    void _select_range(const coord origin, const size extent = {0, 0});
    range _make_range() const;
    static range _make_range(const coord origin, const size extent);
    static bool _range_contains(const range& r, const coord pos);
    void _render_pixel(const coord pos, const bool set, const range& focused_range);
    void _render_pixel(const coord pos, const bool set, const bool focused);
    void _render_pixel_run(const coord pos, const int length, const bool set, const bool focused);
    void _toggle_pixel(const coord pos);
    void _fill_selection(const int fill);
    int8_t& _pixel(const coord pos);
    void _calculate_dimensions();
    void _save_history();
    void _clear_history();

    template <typename T>
    void _for_each_selection(T&& lambda)
    {
        const auto r = _make_range();
        for (auto yindex = r.y.first; yindex <= r.y.second; yindex++)
            for (auto xindex = r.x.first; xindex <= r.x.second; xindex++)
                lambda(coord{yindex, xindex}, _pixel({yindex, xindex}));
    }

    const capabilities& _caps;
    glyph_manager& _glyphs;
    status& _status;
    bool _grid_macro_initialized = false;
    int _grid_macro;
    int _wallpaper_macro;
    int _cell_height = 16;
    int _cell_width = 10;
    int _render_height = 20;
    int _render_width = 20;
    int _pixel_height = 2;
    int _pixel_width = 2;
    int _pixel_ar = 125;
    int _pixel_ar_unscaled = 125;
    std::string _pixel_pattern;
    int _top = 1;
    int _left = 1;
    bool _double_width = false;
    bool _reversed = false;
    bool _need_wallpaper = true;
    coord _focus;
    size _selection;
    std::vector<int8_t> _pixels;
    std::vector<int8_t> _history;
    std::vector<int8_t> _clipboard;
    size _clipboard_size;
    std::optional<int> _char_index;
    bool _dirty = false;
};
