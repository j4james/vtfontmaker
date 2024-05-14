// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <filesystem>

class os {
public:
    os();
    ~os();
    static int getch();
    static bool is_file_hidden(const std::filesystem::path& filepath);
};
