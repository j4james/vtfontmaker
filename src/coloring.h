// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <string>

class capabilities;

class coloring {
public:
    coloring(const capabilities& caps);
    ~coloring();

private:
    std::string _color_table;
};
