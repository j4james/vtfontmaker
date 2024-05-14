// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include "canvas.h"
#include "glyphs.h"
#include "menu.h"
#include "status.h"

#include <filesystem>

class capabilities;

class application {
public:
    static constexpr auto name = "VT Font Maker";
    static constexpr auto wname = L"VT Font Maker";
    static constexpr auto major_version = 1;
    static constexpr auto minor_version = 0;
    static constexpr auto patch_number = 0;

    application(capabilities& caps, const std::filesystem::path& filepath);
    void run();

private:
    void _init_menu();
    bool _can_clear();
    bool _clear(const bool use_defaults);
    void _new(const bool use_defaults = false);
    bool _save();
    bool _save_as();
    bool _open();
    bool _open(const std::filesystem::path& filepath);
    void _properties();
    bool _exit();
    void _about();

    const capabilities& _caps;
    menu _menu;
    status _status;
    glyph_manager _glyphs;
    canvas _canvas;
    std::filesystem::path _filepath;
};
