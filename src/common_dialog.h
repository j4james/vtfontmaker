// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <filesystem>
#include <string_view>

namespace common_dialog {

    std::filesystem::path open();
    std::filesystem::path save(const std::filesystem::path& filepath);

    enum id {
        yes = 1,
        no = 2,
        ok = 4,
        cancel = 8
    };

    int message_box(const std::wstring_view title, const std::wstring_view message, const int buttons);

}  // namespace common_dialog
