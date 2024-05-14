// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class charset {
public:
    static const std::array<charset, 31> all;

    charset(const std::wstring_view name, const std::string_view id, const int size, const std::wstring_view glyphs);
    const std::wstring& name() const;
    const std::string& id() const;
    const int size() const;
    const std::wstring& glyphs() const;
    static std::vector<std::wstring> names();
    static std::vector<std::wstring> names_for_size(const int size);
    static std::optional<int> index_of(const std::string_view id, const int size);
    static const charset* from_index(const int index, const std::optional<int> size = {});

private:
    std::wstring _name;
    std::string _id;
    int _size;
    std::wstring _glyphs;
};
