// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <string>
#include <string_view>

class vt_stream;

class iso2022 {
public:
    iso2022(const wchar_t c);
    iso2022(const std::wstring_view s);
    void write(vt_stream& stream) const;

private:
    std::wstring _s;
};
