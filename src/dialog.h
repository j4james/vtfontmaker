// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include "keyboard.h"

#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

class capabilities;
class layout;
class dialog;

class borders {
public:
    borders(const int top, const int left, const int bottom, const int right);
    ~borders();
    void horizontal(const int row, const int left, const int right, const int type);
    void vertical(const int column, const int top, const int bottom, const char ch);
    void all(const int top, const int left, const int bottom, const int right, const char lch, const char rch, const bool include_top = true);

private:
    int _top;
    int _left;
    int _height;
    int _width;
    std::vector<int> _horizontal;
    std::vector<std::tuple<int, int, int, char>> _vertical;
};

class control {
public:
    control(layout& parent, const bool can_focus);
    virtual ~control() = default;
    void on_key_press(const std::function<bool(key)> handler);
    void on_change(const std::function<void()> handler);

protected:
    friend layout;
    friend dialog;

    virtual int _min_height() const;
    virtual int _min_width() const;
    virtual void _reposition(const int row, const int col, const int height, const int width);
    virtual void _instantiate(borders& borders);
    virtual void _redraw(const bool focused);
    virtual void _focus(const bool focused);
    virtual bool _handle_key(const key key_press);

    void _notify_change();
    void _dirty();

    layout& _parent;
    bool _can_focus = false;
    int _top = 0;
    int _left = 0;
    int _bottom = 0;
    int _right = 0;
    int _height = 0;
    int _width = 0;
    std::function<bool(key)> _key_handler;
    std::function<void()> _change_handler;
};

class text_control : public control {
public:
    text_control(const std::wstring_view value, layout& parent);
    void value(const std::wstring_view value);

private:
    virtual int _min_width() const;
    virtual void _instantiate(borders& borders);
    virtual void _redraw(const bool focused);

    std::wstring _value;
};

class input_control : public control {
public:
    input_control(const std::wstring_view label, const int width, const int& label_space, layout& parent);
    void value(const std::wstring_view value);
    const std::wstring& value() const;

private:
    virtual int _min_width() const;
    virtual void _reposition(const int row, const int col, const int height, const int width);
    virtual void _instantiate(borders& borders);
    virtual void _redraw(const bool focused);
    virtual void _focus(const bool focused);
    virtual bool _handle_key(const key key_press);

    void _pan_left();
    void _pan_right(const bool already_moved = false);
    void _erase(const int index);
    void _erase_back(const int index);
    wchar_t _char_at(const int index) const;

    const int& _label_space;
    std::wstring _label;
    std::wstring _value;
    int _label_width = 0;
    int _input_width = 0;
    int _cursor = 0;
    int _scroll = 0;
};

class list_control : public control {
public:
    list_control(const std::initializer_list<std::wstring_view> headers, const std::initializer_list<int> widths, const int max_rows, layout& parent);
    void clear();
    void add(const std::initializer_list<std::wstring_view> values);
    int selection() const;
    void selection(const int index);

private:
    virtual int _min_height() const;
    virtual int _min_width() const;
    virtual void _instantiate(borders& borders);
    virtual void _redraw(const bool focused);
    virtual void _focus(const bool focused);
    virtual bool _handle_key(const key key_press);

    void _search(const wchar_t ch);
    void _move_to(const int index);
    void _render_row(const int y, const std::vector<std::wstring>& values, const bool fill);
    void _render_selection(const bool selected);

    std::vector<int> _widths;
    std::vector<std::wstring> _headers;
    std::vector<std::vector<std::wstring>> _rows;
    int _max_rows = 0;
    int _selection = 0;
    int _scroll = 0;
};

class dropdown_control : public control {
public:
    dropdown_control(const std::wstring_view label, const std::vector<std::wstring>& options, const int& label_space, layout& parent);
    void options(const std::vector<std::wstring>& options);
    int selection() const;
    void selection(const int index);

private:
    virtual int _min_width() const;
    virtual void _reposition(const int row, const int col, const int height, const int width);
    virtual void _instantiate(borders& borders);
    virtual void _redraw(const bool focused);
    virtual void _focus(const bool focused);
    virtual bool _handle_key(const key key_press);

    void _search(const wchar_t ch);
    void _move_to(const int index);

    const int& _label_space;
    std::wstring _label;
    std::vector<std::wstring> _options;
    int _label_width = 0;
    int _selection = 0;
};

class button_control : public control {
public:
    button_control(const std::wstring_view label, const int id, layout& parent);

private:
    virtual int _min_width() const;
    virtual void _instantiate(borders& borders);
    virtual void _focus(const bool focused);
    virtual bool _handle_key(const key key_press);

    std::wstring _label;
    int _id;
};

class layout : public control {
public:
    enum class direction {
        top_to_bottom,
        left_to_right
    };
    enum class alignment {
        left,
        top = left,
        center,
        right,
        bottom = right,
        fill
    };

    layout(dialog& dlg, layout& parent);
    dialog& dlg();
    text_control& add_text(const std::wstring_view value);
    input_control& add_input(const std::wstring_view label, const int width);
    list_control& add_list(const std::initializer_list<std::wstring_view> headers, const std::initializer_list<int> widths, const int height);
    dropdown_control& add_dropdown(const std::wstring_view label, const std::vector<std::wstring>& options);
    button_control& add_button(const std::wstring_view label, const int id, const bool is_default = false);
    layout& add_group(const alignment halign = alignment::left);
    void add_gap();

protected:
    friend control;

    virtual int _min_height() const;
    virtual int _min_width() const;
    virtual void _reposition(const int row, const int col, const int height, const int width);
    virtual void _instantiate(borders& borders);
    virtual bool _handle_key(const key key_press);

    int _margin_top = 0;
    int _margin_left = 0;
    int _margin_bottom = 0;
    int _margin_right = 0;

private:
    template <typename T, class... Types>
    T& _add_control(Types&&... args);
    void _track_label_width(const std::wstring_view label, const control& control);
    static int _offset(const int available, const int used, const alignment align);
    int _arrow_index() const;

    dialog& _dlg;
    std::vector<std::unique_ptr<control>> _controls;
    std::vector<control*> _arrow_order;
    direction _direction = direction::top_to_bottom;
    alignment _valign = alignment::center;
    alignment _halign = alignment::center;
    int _label_width = 0;
    int _input_width = 0;
};

class dialog : public layout {
public:
    static void initialize(const capabilities& caps);

    dialog(const std::wstring_view title);
    int show();
    void close(const int id);
    void focus(control& control);
    const control& focus() const;
    int max_width() const;
    void on_validate(const std::function<bool(int)> handler);

private:
    friend layout;
    friend control;

    virtual bool _handle_key(const key key_press);

    int _tab_index() const;
    void _focus_control(control* control);
    void _dirty_control(control* control);

    std::wstring _title;
    std::vector<control*> _tab_order;
    control* _focused_control = nullptr;
    control* _initial_focused_control = nullptr;
    std::vector<control*> _dirty_controls;
    std::optional<int> _default_return_code;
    std::optional<int> _return_code;
    std::function<bool(int)> _validate_handler;
    std::vector<int> _borders;
};
