// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include "keyboard.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class menu_group {
public:
    menu_group(const std::wstring_view name, const int xoffset, const std::unordered_set<int>& disabled_ids);
    int left() const;
    int right() const;
    int macro_id() const;
    void add(const int entry_id, const std::wstring_view entry_name);
    void render() const;
    void open();
    void close();
    std::optional<int> process_key(const key keypress);

private:
    bool _disabled(const int entry_index) const;
    void _move_focus(const int offset);
    std::optional<int> _selection_for_key(const key k) const;

    std::wstring _name;
    int _left;
    const std::unordered_set<int>& _disabled_ids;
    std::vector<int> _entry_ids;
    std::unordered_map<int, int> _shortcuts;
    int _entry_count = 0;
    int _focus_index = 0;
    int _open_macro;
    static int _close_macro;
};

class menu_builder {
public:
    menu_builder(menu_group& group, std::unordered_map<key, int>& accelerators);
    ~menu_builder();
    void separator();
    void add(const int id, const std::wstring_view name, const std::optional<key> accelerator = {}, const std::optional<key> accelerator2 = {});

private:
    menu_group& _group;
    std::unordered_map<key, int>& _accelerators;
    std::vector<std::tuple<std::wstring, std::string, bool>> _entries;
    int _width = 0;
    bool _want_separator = false;
};

class menu {
public:
    menu_builder add(const std::wstring_view name);
    void render() const;
    void enable(const int entry_id, const bool enabled = true);
    std::optional<int> process_key(const key keypress);

private:
    std::optional<int> _selection_for_key(const key k) const;
    std::optional<int> _group_for_key(const key k) const;
    void _open_group(const std::optional<int> new_index);
    int _index() const;

    int _width_used = 0;
    std::vector<std::unique_ptr<menu_group>> _groups;
    std::unordered_map<int, int> _shortcuts;
    std::unordered_map<key, int> _accelerators;
    std::unordered_set<int> _disabled_ids;
    std::optional<int> _open_index;
};
