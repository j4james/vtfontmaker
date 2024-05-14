// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "dialog.h"

#include "capabilities.h"
#include "macros.h"
#include "vt.h"

#include <chrono>
#include <numeric>

namespace color {

    constexpr auto title = {0, 1, 7, 47, 30};       // White on DarkestBlue
    constexpr auto basic = {0, 1, 7, 40, 36};       // Black on BrightWhite
    constexpr auto borders = {1, 46, 36};           // LightGray on BrightWhite
    constexpr auto input = {1, 40, 37};             // Black on BrightestWhite (assumes reverse)
    constexpr auto input_label = {36};              // BrightWhite background (assumes bright reverse)
    constexpr auto list_header = {22, 36};          // LightGray background  (assumes reverse)
    constexpr auto selected = {22, 35};             // LighterBlue background (assumes reverse)
    constexpr auto unselected = {1, 37};            // BrightestWhite background (assumes reverse)
    constexpr auto button = {22, 7, 40, 36};        // Black on LightGray
    constexpr auto button_focus = {1, 27, 36, 42};  // BrightWhite on DarkerBlue

}  // namespace color

namespace {

    namespace macros {

        int draw_frame()
        {
            static int frame_macro = macro_manager::create([](auto& macro) {
                macro.sgr(color::basic);
                macro.decfra(' ', {}, {}, {}, {});

                macro.ls1();
                macro.decfra('[', {}, {}, {}, 1);
                macro.decfra(']', {}, 99, {}, 99);
                macro.decfra('-', 99, 1, 99, 99);
                macro.cup(99, {});
                macro.write('`');
                macro.cuf(99);
                macro.write('\'');
                macro.ls0();

                macro.sgr(color::title);
                macro.decfra(' ', {}, {}, 1, {});
            });
            return frame_macro;
        }

    }  // namespace macros

    std::wstring pad(const std::wstring& s, const int width)
    {
        auto padded = s.substr(0, width);
        padded.insert(padded.length(), width - padded.length(), ' ');
        return padded;
    }

    static auto search_string = std::wstring{};
    static auto last_search_time = std::chrono::system_clock::time_point{};

    std::optional<int> search(const wchar_t ch, const int current, int size, std::function<const std::wstring(const int)> supplier)
    {
        using namespace std::chrono_literals;
        const auto now = std::chrono::system_clock::now();
        auto base_index = current;
        if (now > last_search_time + 500ms) {
            search_string.clear();
            base_index++;
        }
        last_search_time = now;
        search_string += std::tolower(ch);
        for (auto i = 0; i < size; i++) {
            const auto index = (base_index + i) % size;
            const auto item = supplier(index);
            auto match = 0;
            while (match < item.length() && match < search_string.length()) {
                if (std::tolower(item[match]) != search_string[match]) break;
                match++;
            }
            if (match == search_string.length())
                return index;
        }
        return {};
    }

    static int screen_height = 24;
    static int screen_width = 80;

}  // namespace

borders::borders(const int top, const int left, const int bottom, const int right)
    : _top{top}, _left{left}, _height{bottom - top + 1}, _width{right - left + 1},
      _horizontal(_height * _width)
{
}

borders::~borders()
{
    if (!_vertical.empty()) {
        vtout.decsc();
        vtout.sgr(color::borders);
        vtout.ls1();
        for (auto y = 0, off = 0; y < _height; y++) {
            for (auto x = 0; x < _width;) {
                const auto type = _horizontal[x + off];
                auto x2 = x + 1;
                while (x2 < _width && _horizontal[x2 + off] == type)
                    x2++;
                if (type != 0) {
                    const auto top = _top + y;
                    const auto left = _left + x;
                    const auto right = left + (x2 - x) - 1;
                    const auto ch = " -~="[type];
                    vtout.decfra(ch, top, left, top, right);
                }
                x = x2;
            }
            off += _width;
        }
        vtout.sgr({37});
        for (auto [column, top, bottom, ch] : _vertical) {
            if (bottom > top)
                vtout.decfra(ch, top, column, bottom, column);
            else {
                vtout.cup(top, column);
                vtout.write(ch);
            }
        }
        vtout.decrc();
    }
}

void borders::horizontal(const int row, const int left, const int right, const int type)
{
    auto off = (row - _top) * _width + (left - _left);
    for (auto x = left; x <= right; x++)
        _horizontal[off++] |= type;
}

void borders::vertical(const int column, const int top, const int bottom, const char ch)
{
    _vertical.emplace_back(column, top, bottom, ch);
}

void borders::all(const int top, const int left, const int bottom, const int right, const char lch, const char rch, const bool include_top)
{
    if (include_top)
        horizontal(top - 1, left, right, 1);
    horizontal(bottom + 1, left, right, 2);
    vertical(left, top, bottom, lch);
    vertical(right, top, bottom, rch);
}

control::control(layout& parent, const bool can_focus)
    : _parent{parent}, _can_focus{can_focus}
{
}

void control::on_key_press(const std::function<bool(key)> handler)
{
    _key_handler = handler;
}

void control::on_change(const std::function<void()> handler)
{
    _change_handler = handler;
}

int control::_min_height() const
{
    return 1;
}

int control::_min_width() const
{
    return 1;
}

void control::_reposition(const int row, const int col, const int height, const int width)
{
    _height = height;
    _width = width;
    _top = row;
    _left = col;
    _bottom = _top + _height - 1;
    _right = _left + _width - 1;
}

void control::_instantiate(borders& borders)
{
}

void control::_redraw(const bool focused)
{
}

void control::_focus(const bool focused)
{
}

bool control::_handle_key(const key key_press)
{
    if (&_parent == this)
        return false;
    else if (_key_handler && _key_handler(key_press))
        return true;
    else
        return _parent._handle_key(key_press);
}

void control::_notify_change()
{
    if (_change_handler) _change_handler();
}

void control::_dirty()
{
    _parent.dlg()._dirty_control(this);
}

text_control::text_control(const std::wstring_view value, layout& parent)
    : control{parent, false}, _value{value}
{
}

void text_control::value(const std::wstring_view value)
{
    _value = value;
    _dirty();
}

int text_control::_min_width() const
{
    return _value.length();
}

void text_control::_instantiate(borders& borders)
{
    vtout.cup(_top, _left);
    vtout.sgr(color::basic);
    vtout.write(_value.substr(0, _width));
}

void text_control::_redraw(const bool focused)
{
    vtout.cup(_top, _left);
    vtout.sgr(color::basic);
    vtout.write(_value.substr(0, _width));
    vtout.write_spaces(_width - _value.length());
}

input_control::input_control(const std::wstring_view label, const int width, const int& label_space, layout& parent)
    : control{parent, true}, _label{label}, _input_width{width}, _label_space{label_space}
{
}

void input_control::value(const std::wstring_view value)
{
    _value = value;
    _cursor = 0;
    _scroll = 0;
    _dirty();
}

const std::wstring& input_control::value() const
{
    return _value;
}

int input_control::_min_width() const
{
    return _label.length() + 1 + _input_width;
}

void input_control::_reposition(const int row, const int col, const int height, const int width)
{
    _label_width = std::max<int>(_label_space, _label.length());
    control::_reposition(row, col + _label_width + 2, height, width - _label_width - 3);
}

void input_control::_instantiate(borders& borders)
{
    vtout.cup(_top, _left - _label_width - 2);
    vtout.sgr(color::input_label);
    vtout.write(_label);
    vtout.deccara(_top, _left - 1, _bottom, _right + 1, color::input);
    vtout.cup(_top, _left);
    vtout.sgr(color::input);
    vtout.write(_value.substr(0, _width));
    borders.all(_top, _left - 1, _bottom, _right + 1, '[', ']');
}

void input_control::_redraw(const bool focused)
{
    vtout.cup(_top, _left);
    vtout.sgr(color::input);
    vtout.write(_value.substr(0, _width));
    vtout.write_spaces(_width - _value.length());
}

void input_control::_focus(const bool focused)
{
    if (focused) {
        const auto off = _cursor - _scroll;
        vtout.decslrm({}, _right);
        vtout.cup(_bottom, _left + off);
        vtout.sm('?', 25);
        vtout.sm(4);
        vtout.sgr(color::input);
    } else {
        vtout.rm('?', 25);
        vtout.rm(4);
        vtout.decslrm();
    }
}

bool input_control::_handle_key(const key key_press)
{
    if (const auto ch = keyboard::printable(key_press)) {
        vtout.write(ch.value());
        _value.insert(_cursor, 1, ch.value());
        _pan_right(true);
        return true;
    } else {
        switch (key_press) {
            case key::bksp:
                if (_cursor > 0) {
                    if (_scroll > 0)
                        _erase_back(--_cursor);
                    else {
                        vtout.write('\b');
                        _erase(--_cursor);
                    }
                }
                return true;
            case key::del:
                if (_cursor < _value.length())
                    _erase(_cursor);
                return true;
            case key::left:
                if (_cursor > 0)
                    _pan_left();
                return true;
            case key::right:
                if (_cursor < _value.length())
                    _pan_right();
                return true;
            default:
                return control::_handle_key(key_press);
        }
    }
}

void input_control::_pan_left()
{
    _cursor--;
    if (_cursor < _scroll) {
        _scroll--;
        vtout.rm('?', 25);
        vtout.write(_value[_cursor]);
        vtout.write('\b');
        vtout.sm('?', 25);
    } else {
        vtout.write('\b');
    }
}

void input_control::_pan_right(const bool already_moved)
{
    _cursor++;
    if (_cursor >= _width + _scroll) {
        _scroll++;
        vtout.deccra(_bottom, _left + 1, _bottom, _right, _bottom, _left);
        vtout.write(_char_at(_cursor));
    } else if (!already_moved) {
        vtout.decfi();
    }
}

void input_control::_erase(const int index)
{
    _value.erase(index, 1);
    const auto rx1 = _left + index - _scroll;
    const auto rx2 = _left + _width - 1;
    if (rx1 < rx2)
        vtout.deccra(_bottom, rx1 + 1, _bottom, rx2, _bottom, rx1);
    vtout.decfra(_char_at(_scroll + _width - 1), _bottom, rx2, _bottom, rx2);
}

void input_control::_erase_back(const int index)
{
    _value.erase(index, 1);
    const auto rx2 = _left + index - _scroll;
    _scroll--;
    if (rx2 >= _left) {
        if (rx2 > _left)
            vtout.deccra(_bottom, _left, _bottom, rx2 - 1, _bottom, _left + 1);
        vtout.decfra(_char_at(_scroll), _bottom, _left, _bottom, _left);
    }
}

wchar_t input_control::_char_at(const int index) const
{
    if (index >= 0 && index < _value.length())
        return _value[index];
    else
        return L' ';
}

list_control::list_control(const std::initializer_list<std::wstring_view> headers, const std::initializer_list<int> widths, const int max_rows, layout& parent)
    : control{parent, true}, _widths{widths}, _max_rows{max_rows}
{
    for (auto header : headers)
        _headers.emplace_back(header);
}

void list_control::clear()
{
    _render_selection(false);
    _selection = 0;
    _scroll = 0;
    _rows.clear();
    _dirty();
    last_search_time = {};
}

void list_control::add(const std::initializer_list<std::wstring_view> values)
{
    auto& row = _rows.emplace_back();
    for (auto value : values)
        row.emplace_back(value);
    _dirty();
}

int list_control::selection() const
{
    return _selection;
}

void list_control::selection(const int index)
{
    _selection = std::max(std::min<int>(index, _rows.size() - 1), 0);
    _scroll = std::max(_selection - (_max_rows - 1), 0);
    _dirty();
    _notify_change();
}

int list_control::list_control::_min_height() const
{
    return _max_rows + 1;
}

int list_control::list_control::_min_width() const
{
    auto total = 0;
    for (auto width : _widths)
        total += (width + 2);
    return total;
}

void list_control::list_control::_instantiate(borders& borders)
{
    _render_row(0, _headers, false);
    vtout.deccara(_top, _left, _top, _right, color::list_header);
    vtout.sgr({22, 36, 47});
    vtout.ls1();
    for (auto i = 0, column = _left; i < _headers.size(); i++) {
        if (i > 0) {
            vtout.cup(_top, column);
            vtout.write('[');
            borders.vertical(column, _top + 1, _bottom, '[');
        }
        column += _widths[i] + 2;
    }
    vtout.ls0();
    vtout.deccara(_top + 1, _left, _bottom, _right, color::input);
    vtout.sgr(color::input);
    for (auto i = 0; i < _rows.size() && i < _max_rows; i++)
        _render_row(i + 1, _rows[i], false);
    borders.all(_top + 1, _left, _bottom, _right, '[', ']', false);
}

void list_control::_redraw(const bool focused)
{
    using namespace std::string_literals;
    const auto columns = _headers.size();
    const auto empty_row = std::vector<std::wstring>(columns, L""s);
    vtout.sgr(color::input);
    for (auto i = 0; i < _max_rows; i++) {
        const auto index = _scroll + i;
        if (index < _rows.size())
            _render_row(i + 1, _rows[index], true);
        else
            _render_row(i + 1, empty_row, true);
    }
    if (focused) _render_selection(true);
}

void list_control::_focus(const bool focused)
{
    _render_selection(focused);
    if (focused) _notify_change();
}

bool list_control::_handle_key(const key key_press)
{
    switch (key_press) {
        case key::up:
            _move_to(_selection - 1);
            return true;
        case key::down:
            _move_to(_selection + 1);
            return true;
        case key::home:
            _move_to(0);
            return true;
        case key::end:
            _move_to(_rows.size() - 1);
            return true;
        case key::pgup:
            if (_selection > _scroll)
                _move_to(_scroll);
            else
                _move_to(_selection - _max_rows + 1);
            return true;
        case key::pgdn:
            if (_selection + 1 < _scroll + _max_rows)
                _move_to(_scroll + _max_rows - 1);
            else
                _move_to(_selection + _max_rows - 1);
            return true;
        default:
            if (const auto ch = keyboard::printable(key_press)) {
                _search(ch.value());
                return true;
            }
            return control::_handle_key(key_press);
    }
}

void list_control::_search(const wchar_t ch)
{
    const auto match = search(ch, _selection, _rows.size(), [&](const auto index) {
        return _rows[index][0];
    });
    if (match) _move_to(match.value());
}

void list_control::_move_to(const int index)
{
    const auto new_selection = std::max(std::min<int>(index, _rows.size() - 1), 0);
    if (_selection != new_selection) {
        _render_selection(false);
        _selection = new_selection;
        if (_selection < _scroll) {
            const auto diff = _scroll - _selection;
            _scroll -= diff;
            if (diff < _max_rows)
                vtout.deccra(_top + 1, _left, _bottom - diff, _right, _top + 1 + diff, _left);
            for (auto i = 0; i < diff && i < _max_rows; i++)
                _render_row(1 + i, _rows[_scroll + i], true);
        } else if (_selection > _scroll + _max_rows - 1) {
            const auto diff = _selection - (_scroll + _max_rows - 1);
            _scroll += diff;
            if (diff < _max_rows)
                vtout.deccra(_top + 1 + diff, _left, _bottom, _right, _top + 1, _left);
            for (auto i = std::max(_max_rows - diff, 0); i < _max_rows; i++)
                _render_row(1 + i, _rows[_scroll + i], true);
        }
        _render_selection(true);
        _notify_change();
    }
}

void list_control::list_control::_render_row(const int y, const std::vector<std::wstring>& values, const bool fill)
{
    vtout.cup(_top + y, _left + 1);
    for (auto i = 0; i < values.size(); i++) {
        const auto width = _widths[i];
        const auto value = fill ? pad(values[i], width) : values[i].substr(0, width);
        vtout.write(value);
        if (i + 1 < values.size())
            vtout.cuf(int(width - value.length() + 2));
    }
}

void list_control::_render_selection(const bool selected)
{
    const auto y = _top + 1 + _selection - _scroll;
    const auto attrs = selected ? color::selected : color::unselected;
    vtout.deccara(y, _left, y, _right, attrs);
}

dropdown_control::dropdown_control(const std::wstring_view label, const std::vector<std::wstring>& options, const int& label_space, layout& parent)
    : control{parent, true}, _label{label}, _label_space{label_space}
{
    for (auto option : options)
        _options.emplace_back(option);
}

void dropdown_control::options(const std::vector<std::wstring>& options)
{
    _options.clear();
    for (auto option : options)
        _options.emplace_back(option);
    _selection = 0;
    _dirty();
    _notify_change();
    last_search_time = {};
}

int dropdown_control::selection() const
{
    return _selection;
}

void dropdown_control::selection(const int index)
{
    _selection = std::max(std::min<int>(index, _options.size() - 1), 0);
    _dirty();
    _notify_change();
}

int dropdown_control::_min_width() const
{
    const auto max = [](auto acc, auto& option) { return std::max<int>(acc, option.length()); };
    const auto input_width = std::accumulate(_options.begin(), _options.end(), 0, max) + 3;
    return _label.length() + 1 + input_width;
}

void dropdown_control::_reposition(const int row, const int col, const int height, const int width)
{
    _label_width = std::max<int>(_label_space, _label.length());
    control::_reposition(row, col + _label_width + 1, height, width - _label_width - 1);
}

void dropdown_control::_instantiate(borders& borders)
{
    vtout.cup(_top, _left - _label_width - 1);
    vtout.sgr(color::input_label);
    vtout.write(_label);
    vtout.sgr(color::input);
    _redraw(false);
    borders.all(_top, _left, _bottom, _right, '[', 'v');
}

void dropdown_control::_redraw(const bool focused)
{
    const auto option = pad(_options[_selection], _width - 2);
    _focus(focused);
    vtout.sgr(focused ? color::selected : color::unselected);
    vtout.cup(_bottom, _left + 1);
    vtout.write(option);
}

void dropdown_control::_focus(const bool focused)
{
    const auto attrs = focused ? color::selected : color::unselected;
    vtout.deccara(_top, _left, _top, _right - 1, attrs);
}

bool dropdown_control::_handle_key(const key key_press)
{
    switch (key_press) {
        case key::up:
            _move_to(_selection - 1);
            return true;
        case key::down:
            _move_to(_selection + 1);
            return true;
        case key::home:
            _move_to(0);
            return true;
        case key::end:
            _move_to(_options.size() - 1);
            return true;
        default:
            if (const auto ch = keyboard::printable(key_press)) {
                _search(ch.value());
                return true;
            }
            return control::_handle_key(key_press);
    }
}

void dropdown_control::_search(const wchar_t ch)
{
    const auto match = search(ch, _selection, _options.size(), [&](const auto index) {
        return _options[index];
    });
    if (match) _move_to(match.value());
}

void dropdown_control::_move_to(const int index)
{
    const auto new_selection = std::max(std::min<int>(index, _options.size() - 1), 0);
    if (_selection != new_selection) {
        _selection = new_selection;
        _redraw(true);
        _notify_change();
    }
}

button_control::button_control(const std::wstring_view label, const int id, layout& parent)
    : control{parent, true}, _label{label}, _id{id}
{
}

int button_control::_min_width() const
{
    const auto pad = std::max<int>(10 - _label.length(), 2) & ~1;
    return _label.length() + pad;
}

void button_control::_instantiate(borders& borders)
{
    _focus(false);
    const int indent = (_width - _label.length()) / 2;
    vtout.cup(_top, _left + indent);
    vtout.sgr(color::button);
    vtout.write(_label);
}

void button_control::_focus(const bool focused)
{
    const auto attrs = focused ? color::button_focus : color::button;
    vtout.deccara(_top, _left, _top, _right, attrs);
}

bool button_control::_handle_key(const key key_press)
{
    switch (key_press) {
        case key::enter:
        case key::space:
            _parent.dlg().close(_id);
            return true;
        default:
            return control::_handle_key(key_press);
    }
}

layout::layout(dialog& dlg, layout& parent)
    : control{parent, false}, _dlg{dlg}
{
}

dialog& layout::dlg()
{
    return _dlg;
}

text_control& layout::add_text(const std::wstring_view value)
{
    return _add_control<text_control>(value, *this);
}

input_control& layout::add_input(const std::wstring_view label, const int width)
{
    if (_direction == direction::top_to_bottom && _controls.size()) add_gap();
    auto& control = _add_control<input_control>(label, width, _label_width, *this);
    _track_label_width(label, control);
    return control;
}

list_control& layout::add_list(const std::initializer_list<std::wstring_view> headers, const std::initializer_list<int> widths, const int height)
{
    return _add_control<list_control>(headers, widths, height, *this);
}

dropdown_control& layout::add_dropdown(const std::wstring_view label, const std::vector<std::wstring>& options)
{
    if (_direction == direction::top_to_bottom && _controls.size()) add_gap();
    auto& control = _add_control<dropdown_control>(label, options, _label_width, *this);
    _track_label_width(label, control);
    return control;
}

button_control& layout::add_button(const std::wstring_view label, const int id, const bool is_default)
{
    auto& button = _add_control<button_control>(label, id, *this);
    if (is_default) _dlg._default_return_code = id;
    return button;
}

layout& layout::add_group(const alignment halign)
{
    auto& control_ref = _add_control<layout>(_dlg, *this);
    control_ref._direction = direction(1 - int(_direction));
    control_ref._valign = alignment::top;
    control_ref._halign = halign;
    if (_controls.size() > 1 && _direction == direction::top_to_bottom)
        control_ref._margin_top = 1;
    return control_ref;
}

void layout::add_gap()
{
    _add_control<control>(*this, false);
}

int layout::_min_height() const
{
    if (_direction == direction::top_to_bottom) {
        const auto sum = [](auto acc, auto& control) { return acc + control->_min_height(); };
        return _margin_top + std::accumulate(_controls.begin(), _controls.end(), 0, sum) + _margin_bottom;
    } else {
        const auto max = [](auto acc, auto& control) { return std::max(acc, control->_min_height()); };
        return _margin_top + std::accumulate(_controls.begin(), _controls.end(), 0, max) + _margin_bottom;
    }
}

int layout::_min_width() const
{
    const auto init = _label_width + _input_width;
    if (_direction == direction::top_to_bottom) {
        const auto max = [](auto acc, auto& control) { return std::max(acc, control->_min_width()); };
        return _margin_left + std::accumulate(_controls.begin(), _controls.end(), init, max) + _margin_right;
    } else {
        const auto sum = [](auto acc, auto& control) { return acc + control->_min_width() + 1; };
        return _margin_left + std::accumulate(_controls.begin(), _controls.end(), init, sum) - 1 + _margin_right;
    }
}

void layout::_reposition(const int row, const int col, const int height, const int width)
{
    const auto used_height = _valign == alignment::fill ? height : this->_min_height();
    const auto used_width = _halign == alignment::fill ? width : this->_min_width();
    const auto top = row + _offset(height, used_height, _valign);
    const auto left = col + _offset(width, used_width, _halign);
    control::_reposition(top, left, used_height, used_width);

    auto r = _top + _margin_top;
    auto c = _left + _margin_left;
    const auto inner_width = _width - _margin_left - _margin_right;
    const auto inner_height = _height - _margin_top - _margin_bottom;
    for (auto& control : _controls) {
        if (_direction == direction::top_to_bottom) {
            control->_reposition(r, c, control->_min_height(), inner_width);
            r += control->_min_height();
        } else {
            control->_reposition(r, c, inner_height, control->_min_width());
            c += control->_min_width() + 1;
        }
    }
}

void layout::_instantiate(borders& borders)
{
    for (auto& control : _controls) {
        if (control->_can_focus) {
            _arrow_order.push_back(control.get());
            _dlg._tab_order.push_back(control.get());
        }
        control->_instantiate(borders);
    }
}

bool layout::_handle_key(const key key_press)
{
    const auto forward = _direction == direction::left_to_right ? key::right : key::down;
    if (key_press == forward) {
        auto index = _arrow_index() + 1;
        if (index < _arrow_order.size())
            _dlg._focus_control(_arrow_order[index]);
        return true;
    }
    const auto back = _direction == direction::left_to_right ? key::left : key::up;
    if (key_press == back) {
        auto index = _arrow_index() - 1;
        if (index >= 0)
            _dlg._focus_control(_arrow_order[index]);
        return true;
    }
    return control::_handle_key(key_press);
}

template <typename T, class... Types>
T& layout::_add_control(Types&&... args)
{
    auto control_ptr = std::make_unique<T>(std::forward<Types>(args)...);
    auto& control_ref = *control_ptr;
    _controls.push_back(std::move(control_ptr));
    return control_ref;
}

void layout::_track_label_width(const std::wstring_view label, const control& control)
{
    if (_direction == direction::top_to_bottom) {
        _label_width = std::max<int>(_label_width, label.length());
        _input_width = std::max<int>(_input_width, control._min_width() - label.length());
    }
}

int layout::_offset(const int available, const int used, const alignment align)
{
    switch (align) {
        case alignment::left:
            return 0;
        case alignment::center:
            return (available - used) / 2;
        case alignment::right:
            return available - used;
        default:
            return 0;
    }
}

int layout::_arrow_index() const
{
    const auto position = std::find(_arrow_order.begin(), _arrow_order.end(), &_dlg.focus());
    return std::distance(_arrow_order.begin(), position);
}

void dialog::initialize(const capabilities& caps)
{
    screen_height = caps.height;
    screen_width = caps.width;

    // Force macro instantiation.
    macros::draw_frame();
}

dialog::dialog(const std::wstring_view title)
    : layout{*this, *this}, _title{title}
{
    _margin_top = 2;
    _margin_bottom = 1;
    _margin_left = 2;
    _margin_right = 2;
}

int dialog::show()
{
    _reposition(1, 1, screen_height, screen_width);

    static int page = 1;
    page++;

    vtout.decstbm(_top, _bottom);
    vtout.decslrm(_left, _right);
    vtout.deccra({}, {}, {}, {}, 1, {}, {}, page);
    vtout.decinvm(macros::draw_frame());

    vtout.cup({}, int(screen_width - _title.length()) / 2 + 2 - _left);
    vtout.write(_title);
    vtout.sgr(color::basic);

    vtout.decslrm();
    vtout.decstbm();

    {
        auto b = borders{_top, _left, _bottom, _right};
        _instantiate(b);
    }

    _focus_control(_initial_focused_control ? _initial_focused_control : _tab_order[0]);
    _return_code = {};
    while (!_return_code) {
        _dirty_controls.clear();
        _focused_control->_handle_key(keyboard::read());
        for (auto control : _dirty_controls)
            control->_redraw(_focused_control == control);
    }
    _focus_control(nullptr);

    vtout.deccra(_top, _left, _bottom, _right, page, _top, _left, 1);
    page--;

    return _return_code.value();
}

void dialog::close(const int id)
{
    _return_code = id;
    if (_validate_handler) {
        if (_focused_control) _focused_control->_focus(false);
        if (!_validate_handler(id)) {
            _return_code = {};
            if (_focused_control) _focused_control->_focus(true);
        }
    }
}

void dialog::focus(control& control)
{
    _initial_focused_control = &control;
}

const control& dialog::focus() const
{
    return *_focused_control;
}

int dialog::max_width() const
{
    return std::min(screen_width * 80 / 100, 50);
}

void dialog::on_validate(const std::function<bool(int)> handler)
{
    _validate_handler = handler;
}

bool dialog::_handle_key(const key key_press)
{
    switch (key_press) {
        case key::tab: {
            const auto index = _tab_index() + 1;
            _focus_control(_tab_order[index % _tab_order.size()]);
            return true;
        }
        case key::shift + key::tab: {
            const auto index = _tab_index() + _tab_order.size() - 1;
            _focus_control(_tab_order[index % _tab_order.size()]);
            return true;
        }
        case key::enter:
            if (_default_return_code)
                close(_default_return_code.value());
            return true;
        default:
            return layout::_handle_key(key_press);
    }
}

int dialog::_tab_index() const
{
    const auto position = std::find(_tab_order.begin(), _tab_order.end(), _focused_control);
    return std::distance(_tab_order.begin(), position);
}

void dialog::_focus_control(control* control)
{
    if (_focused_control != control) {
        if (_focused_control) _focused_control->_focus(false);
        _focused_control = control;
        if (_focused_control) _focused_control->_focus(true);
    }
}

void dialog::_dirty_control(control* control)
{
    if (std::find(_dirty_controls.begin(), _dirty_controls.end(), control) == _dirty_controls.end())
        _dirty_controls.push_back(control);
}
