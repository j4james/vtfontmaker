// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "status.h"

#include "capabilities.h"
#include "charsets.h"
#include "vt.h"

#include <format>

namespace {

    static constexpr auto status_color = {0, 1, 36, 42};  // BrightWhite on DarkerBlue

}  // namespace

status::status(const capabilities& caps)
    : _width{caps.width}, _height{caps.height}
{
    character_set("B", 94);
}

void status::render()
{
    vtout.deccara(_height, {}, {}, {}, status_color);
}

const std::wstring& status::filename() const
{
    return _filename;
}

void status::filename(const std::wstring_view filename)
{
    const auto pad = 1 + int(_filename.length()) - int(filename.length());
    _filename = filename;
    vtout.sgr(status_color);
    vtout.cup(_height, 2);
    vtout.write(_filename);
    vtout.write_spaces(pad);
}

void status::character_set(const std::string_view id, const int size)
{
    const auto set_values = [&](auto& cs) {
        _char_index = -1;
        _char_values = cs.glyphs();
        if (cs.size() == 94) {
            _char_values.insert(_char_values.begin(), L' ');
            _char_values.push_back(L'\177');
        }
    };
    for (auto& cs : charset::all) {
        if (cs.size() == size && cs.id() == id && !cs.glyphs().empty()) {
            set_values(cs);
            return;
        }
    }
    // If no match, use ASCII
    set_values(charset::all[2]);
}

void status::index(const int index)
{
    using namespace std::string_literals;
    if (_char_index != index) {
        _char_index = index;

        vtout.sgr(status_color);
        vtout.cup(_height, _width - 8);

        const auto ch = _char_values[index];
        if (ch == L'\177')
            vtout.write("DEL");
        else if (ch == L' ' || ch == L'\240')
            vtout.write(" SP");
        else {
            vtout.write("  ");
            vtout.write(ch);
        }

        vtout.write(std::format(" 0x{:02X}", 0x20 + index));
    }
}

int status::index() const
{
    return _char_index;
}

void status::dirty(const bool dirty)
{
    if (_dirty != dirty) {
        _dirty = dirty;
        vtout.sgr(status_color);
        vtout.cup(_height, 2 + int(_filename.length()));
        vtout.write(_dirty ? "*" : " ");
    }
}

bool status::dirty() const
{
    return _dirty;
}
