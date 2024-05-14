// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "canvas.h"

#include "capabilities.h"
#include "glyphs.h"
#include "macros.h"
#include "status.h"
#include "vt.h"

namespace color {

    constexpr auto wallpaper = {0, 37, 45};  // White on LighterBlue

    constexpr auto dark_grid_init = {0, 31, 40};   // DarkGray on Black
    constexpr auto dark_grid_alt_init = {30, 41};  // Black on DarkGray
    constexpr int dark_grid[] = {0, 1};            // Black, DarkGray
    constexpr int dark_grid_focus = 2;             // DarkerBlue, DarkBlue
    constexpr int dark_pixel = 7;                  // White
    constexpr int dark_pixel_focus = 5;            // LighterBlue

    constexpr auto light_grid_init = {0, 36, 47};   // LightGray on White
    constexpr auto light_grid_alt_init = {37, 46};  // White on LightGray
    constexpr int light_grid[] = {7, 6};            // White, LightGray
    constexpr int light_grid_focus = -2;            // LighterBlue, LightBlue
    constexpr int light_pixel = 0;                  // Black
    constexpr int light_pixel_focus = 2;            // DarkerBlue

}  // namespace color

canvas::canvas(const capabilities& caps, glyph_manager& glyphs, status& status)
    : _caps{caps}, _glyphs{glyphs}, _status{status}
{
    _grid_macro = macro_manager::reserve_id();
    _wallpaper_macro = macro_manager::create([&](auto& macro) {
        macro.ls1();
        macro.sgr(color::wallpaper);
        macro.decfra('@', 2, {}, caps.height - 1, {});
        macro.ls0();
    });
}

void canvas::render()
{
    vtout.decinvm(_wallpaper_macro);
    _need_wallpaper = false;
}

void canvas::refresh()
{
    _focus = {};
    _selection = {};
    _cell_height = _glyphs.cell_height();
    _cell_width = _glyphs.cell_width();
    _pixel_ar_unscaled = _glyphs.pixel_aspect_ratio();
    _calculate_dimensions();
    _pixels.clear();
    _char_index = {};
    _load_char(_glyphs.first_used(), 0);
}

void canvas::select_all()
{
    _select_range({}, {_cell_height - 1, _cell_width - 1});
}

void canvas::cut_selection()
{
    copy_selection();
    delete_selection();
}

void canvas::copy_selection()
{
    _clipboard.clear();
    _for_each_selection([&](const auto pos, auto& pixel) {
        _clipboard.push_back(pixel);
    });
    const auto copied_range = _make_range();
    _clipboard_size = copied_range.extent();
    _select_range(copied_range.origin(), copied_range.extent());
}

void canvas::delete_selection()
{
    _fill_selection(0);
}

bool canvas::_range_contains(const range& r, const coord pos)
{
    return pos.x >= r.x.first && pos.x <= r.x.second && pos.y >= r.y.first && pos.y <= r.y.second;
}

void canvas::paste()
{
    if (can_paste()) {
        _save_history();
        const auto focused_range = _make_range(_focus, _selection);
        const auto origin = focused_range.origin();
        for (auto y = origin.y, i = 0; y <= origin.y + _clipboard_size.h; y++) {
            for (auto x = origin.x; x <= origin.x + _clipboard_size.w; x++) {
                const auto is_point = _clipboard[i++];
                if (is_point && y < _cell_height && x < _cell_width) {
                    _pixel({y, x}) = 1;
                    if (_range_contains(focused_range, {y, x}))
                        _render_pixel({y, x}, true, true);
                }
            }
        }
        _select_range(origin, _clipboard_size);
    }
}

void canvas::undo()
{
    if (can_undo()) {
        const auto entry_size = 4 + _cell_width * _cell_height;
        auto offset = _history.size() - entry_size;
        const auto focus = coord{_history[offset++], _history[offset++]};
        const auto selection = size{_history[offset++], _history[offset++]};
        _select_range(focus, selection);
        const auto focused_range = _make_range(_focus, _selection);
        for (auto y = 0; y < _cell_height; y++)
            for (auto x = 0; x < _cell_width; x++) {
                auto new_pixel = _history[offset++];
                auto& pixel = _pixel({y, x});
                if (pixel != new_pixel) {
                    pixel = new_pixel;
                    _render_pixel({y, x}, new_pixel, focused_range);
                }
            }
        _history.resize(_history.size() - entry_size);
        _history.shrink_to_fit();
    }
}

bool canvas::can_paste() const
{
    return !_clipboard.empty();
}

bool canvas::can_undo() const
{
    const auto entry_size = 4 + _cell_width * _cell_height;
    return _history.size() >= entry_size;
}

void canvas::invert()
{
    _save_history();
    const auto focused_range = _make_range(_focus, _selection);
    _for_each_selection([&](const auto pos, auto& pixel) {
        pixel ^= 1;
        _render_pixel(pos, pixel, focused_range);
    });
}

void canvas::flip_horizontally()
{
    _save_history();
    const auto focused_range = _make_range(_focus, _selection);
    const auto target_range = _make_range();
    const auto origin = target_range.origin();
    const auto extent = target_range.extent();
    for (auto y = 0; y <= extent.h; y++) {
        for (auto x = 0; x < (extent.w + 1) / 2; x++) {
            const auto pos1 = coord{origin.y + y, origin.x + x};
            const auto pos2 = coord{origin.y + y, origin.x + extent.w - x};
            auto& pixel1 = _pixel(pos1);
            auto& pixel2 = _pixel(pos2);
            if (pixel1 != pixel2) {
                std::swap(pixel1, pixel2);
                _render_pixel(pos1, pixel1, focused_range);
                _render_pixel(pos2, pixel2, focused_range);
            }
        }
    }
}

void canvas::flip_vertically()
{
    _save_history();
    const auto focused_range = _make_range(_focus, _selection);
    const auto target_range = _make_range();
    const auto origin = target_range.origin();
    const auto extent = target_range.extent();
    for (auto y = 0; y < (extent.h + 1) / 2; y++) {
        for (auto x = 0; x <= extent.w; x++) {
            const auto pos1 = coord{origin.y + y, origin.x + x};
            const auto pos2 = coord{origin.y + extent.h - y, origin.x + x};
            auto& pixel1 = _pixel(pos1);
            auto& pixel2 = _pixel(pos2);
            if (pixel1 != pixel2) {
                std::swap(pixel1, pixel2);
                _render_pixel(pos1, pixel1, focused_range);
                _render_pixel(pos2, pixel2, focused_range);
            }
        }
    }
}

void canvas::next_char(const bool only_used)
{
    _load_char(_char_index.value_or(-1), +1, only_used);
}

void canvas::prev_char(const bool only_used)
{
    _load_char(_char_index.value_or(100), -1, only_used);
}

void canvas::toggle_double_width()
{
    _double_width = !_double_width;
    _calculate_dimensions();
    _render();
}

void canvas::toggle_reverse_screen()
{
    _reversed = !_reversed;
    _grid_macro_initialized = false;
    _render();
}

void canvas::process_key(const key key_press)
{
    switch (key_press) {
        case key::home:
            _load_char(0, 0);
            break;
        case key::end:
            _load_char(100, 0);
            break;
        case key::up:
            _select_range({_focus.y - 1, _focus.x});
            break;
        case key::down:
            _select_range({_focus.y + 1, _focus.x});
            break;
        case key::left:
            _select_range({_focus.y, _focus.x - 1});
            break;
        case key::right:
            _select_range({_focus.y, _focus.x + 1});
            break;
        case key::alt + key::up:
            _select_range(_focus, {_selection.h - 1, _selection.w});
            break;
        case key::alt + key::down:
            _select_range(_focus, {_selection.h + 1, _selection.w});
            break;
        case key::alt + key::left:
            _select_range(_focus, {_selection.h, _selection.w - 1});
            break;
        case key::alt + key::right:
            _select_range(_focus, {_selection.h, _selection.w + 1});
            break;
        case key::space:
            if (_selection == size{})
                _toggle_pixel(_focus);
            else
                _fill_selection(1);
            break;
    }
}

void canvas::flush()
{
    if (_char_index && _dirty) {
        _glyphs[_char_index.value()] = _pixels;
        _dirty = false;
    }
}

void canvas::_render()
{
    _render_grid();
    const auto focused_range = _make_range(_focus, _selection);
    for (auto y = 0; y < _cell_height; y++) {
        for (auto x = 0; x < _cell_width; x++) {
            const bool focused = _range_contains(focused_range, {y, x});
            if (_pixel({y, x})) {
                auto x2 = x + 1;
                while (x2 < _cell_width && _pixel({y, x2}) && focused == _range_contains(focused_range, {y, x2}))
                    x2++;
                _render_pixel_run({y, x}, x2 - x, true, focused);
                x = x2 - 1;
            } else if (focused) {
                _render_pixel({y, x}, false, true);
            }
        }
    }
}

void canvas::_render_grid()
{
    if (!_grid_macro_initialized) {
        _grid_macro_initialized = true;

        static int grid_macro_inner = macro_manager::reserve_id();
        macro_manager::create(grid_macro_inner, [&](auto& macro) {
            const auto color_grid_alt_init = _reversed ? color::light_grid_alt_init : color::dark_grid_alt_init;
            const auto pattern_height = _pixel_pattern.length();
            const auto reps = (_cell_width + 1) / 2;
            macro.decstbm(_top, _top + _render_height - 1);
            macro.il(pattern_height);
            macro.decstbm(_top, _top + pattern_height - 1);
            macro.repeat(reps, [&](auto& macro2) {
                macro2.decic(_pixel_width * 2);
                for (auto i = 0; i < pattern_height; i++)
                    macro2.decfra(_pixel_pattern[i], i + 1, {}, i + 1, _pixel_width * 2);
                macro2.deccara({}, {}, pattern_height, _pixel_width, color_grid_alt_init);
            });
        });

        macro_manager::create(_grid_macro, [&](auto& macro) {
            const auto color_grid_init = _reversed ? color::light_grid_init : color::dark_grid_init;
            const auto pattern_height = _pixel_pattern.length();
            const auto reps = (_render_height + pattern_height - 1) / pattern_height;
            macro.sgr(color_grid_init);
            macro.ls1();
            macro.decslrm(_left, _left + _render_width - 1);
            macro.repeat(reps, [&](auto& macro2) {
                macro2.decinvm(grid_macro_inner);
            });
            macro.decstbm();
            macro.decslrm();
            macro.ls0();
        });
    }
    if (_need_wallpaper) render();
    vtout.decinvm(_grid_macro);
}

void canvas::_load_char(const int start_index, const int increment, const bool only_used)
{
    const auto size = _glyphs.size();
    const auto min_index = size == 96 ? 0 : 1;
    const auto max_index = size == 96 ? 95 : 94;
    auto index = start_index;
    do {
        index = std::clamp(index + increment, min_index, max_index);
        if (index == min_index || index == max_index) break;
    } while (only_used && !_glyphs[index].used());
    if (index != _char_index) {
        flush();
        _clear_history();
        _pixels = _glyphs[index];
        _char_index = index;
        _render();
        _status.index(_char_index.value());
    }
}

void canvas::_select_range(const coord origin, const size extent)
{
    const auto new_y = std::clamp(origin.y, 0, _cell_height - 1);
    const auto new_x = std::clamp(origin.x, 0, _cell_width - 1);
    const auto new_h = std::clamp(extent.h, -new_y, _cell_height - new_y - 1);
    const auto new_w = std::clamp(extent.w, -new_x, _cell_width - new_x - 1);
    if (_focus != coord{new_y, new_x} || _selection != size{new_h, new_w}) {
        const auto new_range = _make_range({new_y, new_x}, {new_h, new_w});
        const auto old_range = _make_range(_focus, _selection);
        for (auto y = 0; y < _cell_height; y++) {
            const auto y_inside_old = y >= old_range.y.first && y <= old_range.y.second;
            const auto y_inside_new = y >= new_range.y.first && y <= new_range.y.second;
            for (auto x = 0; x < _cell_width; x++) {
                const auto inside_old = x >= old_range.x.first && x <= old_range.x.second && y_inside_old;
                const auto inside_new = x >= new_range.x.first && x <= new_range.x.second && y_inside_new;
                if (inside_old != inside_new)
                    _render_pixel({y, x}, _pixel({y, x}), inside_new);
            }
        }
        _focus = {new_y, new_x};
        _selection = {new_h, new_w};
    }
}

canvas::range canvas::_make_range() const
{
    if (_selection == size{})
        return _make_range({}, {_cell_height - 1, _cell_width - 1});
    else
        return _make_range(_focus, _selection);
}

canvas::range canvas::_make_range(const coord origin, const size extent)
{
    auto y1 = origin.y;
    auto y2 = origin.y + extent.h;
    if (y1 > y2) std::swap(y1, y2);
    auto x1 = origin.x;
    auto x2 = origin.x + extent.w;
    if (x1 > x2) std::swap(x1, x2);
    return {{y1, y2}, {x1, x2}};
}

void canvas::_render_pixel(const coord pos, const bool set, const range& focused_range)
{
    _render_pixel(pos, set, _range_contains(focused_range, pos));
}

void canvas::_render_pixel(const coord pos, const bool set, const bool focused)
{
    _render_pixel_run(pos, 1, set, focused);
}

void canvas::_render_pixel_run(const coord pos, const int length, const bool set, const bool focused)
{
    const auto top = pos.y * _pixel_ar / 100 + _top;
    const auto bottom = ((pos.y + 1) * _pixel_ar - 1) / 100 + _top;
    const auto left = pos.x * _pixel_width + _left;
    const auto right = left + (_pixel_width * length) - 1;

    const auto color_pixel = _reversed ? color::light_pixel : color::dark_pixel;
    const auto color_pixel_focus = _reversed ? color::light_pixel_focus : color::dark_pixel_focus;
    const auto color_grid = _reversed ? color::light_grid : color::dark_grid;
    const auto color_grid_focus = _reversed ? color::light_grid_focus : color::dark_grid_focus;
    const auto fg_color = focused ? color_pixel_focus : color_pixel;
    const auto bg_color = color_grid[(pos.x + pos.y) % 2] + (focused ? color_grid_focus : 0);

    auto attr = set ? fg_color : bg_color;
    attr += (pos.y % 2 ? 40 : 30);
    vtout.deccara(top, left, bottom, right, {attr});
}

void canvas::_toggle_pixel(const coord pos)
{
    _save_history();
    auto& pixel = _pixel(pos);
    pixel ^= 1;
    _render_pixel(pos, pixel, true);
}

void canvas::_fill_selection(const int fill)
{
    _save_history();
    const auto focused_range = _make_range(_focus, _selection);
    _for_each_selection([&](const auto pos, auto& pixel) {
        if (pixel != fill) {
            pixel = fill;
            _render_pixel(pos, fill, focused_range);
        }
    });
}

int8_t& canvas::_pixel(const coord pos)
{
    return _pixels[pos.y * _cell_width + pos.x];
}

void canvas::_calculate_dimensions()
{
    const auto free_height = (_caps.height - 4) * 100;
    const auto scale_down = _cell_height * _pixel_ar_unscaled > free_height;
    _pixel_ar = scale_down ? _pixel_ar_unscaled >> 1 : _pixel_ar_unscaled;
    if (_pixel_ar >= 250) {
        _pixel_pattern = R"(##^  )";
        _pixel_ar = 250;
    } else if (_pixel_ar >= 200) {
        _pixel_pattern = R"(##  )";
        _pixel_ar = 200;
    } else if (_pixel_ar >= 125) {
        _pixel_pattern = R"(#"_+ )";
        _pixel_ar = 125;
    } else if (_pixel_ar >= 100) {
        _pixel_pattern = R"(# )";
        _pixel_ar = 100;
    } else if (_pixel_ar >= 80) {
        _pixel_pattern = R"(82641735)";
        _pixel_ar = 80;
    } else {
        _pixel_pattern = R"(^^)";
        _pixel_ar = 50;
    }
    _pixel_height = (_pixel_ar + 99) / 100;
    _pixel_width = (_double_width ? 2 : 1) * (scale_down ? 1 : 2);
    _render_height = (_cell_height * _pixel_ar + 99) / 100;
    _render_width = _cell_width * _pixel_width;
    _top = std::max((_caps.height - _render_height) / 2 + 1, 1);
    _left = std::max((_caps.width - _render_width) / 2 + 1, 1);
    _grid_macro_initialized = false;
    _need_wallpaper = true;
}

void canvas::_save_history()
{
    _dirty = true;
    _status.dirty(true);
    _history.push_back(_focus.y);
    _history.push_back(_focus.x);
    _history.push_back(_selection.h);
    _history.push_back(_selection.w);
    _history.insert(_history.end(), _pixels.begin(), _pixels.end());
}

void canvas::_clear_history()
{
    _dirty = false;
    _history.clear();
    _history.shrink_to_fit();
}
