// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <string>
#include <string_view>

class capabilities;

class status {
public:
    status(const capabilities& caps);
    void render();
    const std::wstring& filename() const;
    void filename(const std::wstring_view filename);
    void character_set(const std::string_view id, const int size);
    void index(const int index);
    int index() const;
    void dirty(const bool dirty);
    bool dirty() const;

private:
    int _width;
    int _height;
    std::wstring _filename;
    bool _dirty = false;
    int _char_index;
    std::wstring _char_values;
};
