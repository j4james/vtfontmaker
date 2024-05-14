// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

class glyph_manager;

class glyph {
public:
    glyph(const std::string_view sixels);
    const std::string& str() const;
    std::vector<int8_t> pixels(const int cell_width, const int cell_height) const;
    void pixels(const int cell_width, const int cell_height, const std::vector<int8_t>& pixels);
    bool used() const;

private:
    friend glyph_manager;
    std::string _sixels;
    int _used_width = 0;
    int _used_height = 0;
};

class glyph_reference {
public:
    glyph_reference(glyph_manager& manager, const int index);
    glyph_reference& operator=(const std::vector<int8_t>& pixels);
    operator std::vector<int8_t>() const;
    bool used() const;

private:
    glyph_manager& _manager;
    int _index;
};

class glyph_manager {
public:
    class parameters;

    glyph_manager();
    void clear();
    void clear(const std::vector<int>& params, const std::string_view id);
    bool load(const std::filesystem::path& path);
    bool save(const std::filesystem::path& path);
    parameters& params();
    const parameters& params() const;
    const std::string& id() const;
    void id(const std::string_view id);
    const std::optional<bool> c1_controls() const;
    void c1_controls(const bool c1_8bit);
    int size() const;
    int first_used() const;
    int cell_width() const;
    int cell_height() const;
    int pixel_aspect_ratio() const;
    glyph_reference operator[](const int index);

private:
    friend glyph_reference;

    std::vector<int8_t> _glyph_pixels(const int index);
    void _glyph_pixels(const int _index, const std::vector<int8_t>& pixels);
    std::tuple<int, int, int> _detect_dimensions();

    static constexpr int max_width = 16;
    static constexpr int max_height = 32;
    std::string _prefix;
    std::string _suffix;
    std::string _introducer;
    std::string _terminator;
    std::string _id;
    std::string _sixel_prefix;
    std::string _sixel_suffix;
    std::unique_ptr<parameters> _parms;
    std::vector<glyph> _glyphs;
    int _size;
    int _first_index;
    int _cell_width;
    int _cell_height;
    int _pixel_aspect_ratio;
};

class glyph_manager::parameters {
public:
    parameters(const std::string_view str);
    parameters(const std::vector<int>& values);
    const std::string& str() const;
    std::optional<int> pfn() const;
    void pfn(const std::optional<int> value);
    std::optional<int> pcn() const;
    void pcn(const std::optional<int> value);
    std::optional<int> pe() const;
    void pe(const std::optional<int> value);
    std::optional<int> pcmw() const;
    std::optional<int> pss() const;
    std::optional<int> pu() const;
    std::optional<int> pcmh() const;
    std::optional<int> pcss() const;

private:
    void _rebuild();

    std::string _str;
    std::vector<std::optional<int>> _values;
    int _values_used = 0;
};
