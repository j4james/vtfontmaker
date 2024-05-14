// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "iso2022.h"

#include "charsets.h"
#include "vt.h"

#include <cstdint>
#include <unordered_map>

namespace {

    constexpr int soft_font = 99;

    std::unordered_map<wchar_t, uint16_t> build_charset_map()
    {
        auto charset_map = std::unordered_map<wchar_t, uint16_t>{};
        for (auto i = 0; i < charset::all.size(); i++) {
            const auto& cs = charset::all[i];
            const auto& glyphs = cs.glyphs();
            const auto base = cs.size() == 94 ? '!' : ' ';
            for (auto j = 0; j < glyphs.length(); j++) {
                const auto wide_char = glyphs[j];
                auto& ref = charset_map[wide_char];
                if (ref == 0)
                    ref = base + j + (i << 8);
            }
        }
        for (auto j = '!'; j <= '~'; j++) {
            const auto wide_char = L'\uE000' + j;
            auto& ref = charset_map[wide_char];
            ref = j + (soft_font << 8);
        }
        return charset_map;
    }

}  // namespace

iso2022::iso2022(const wchar_t c)
    : _s{&c, 1}
{
}

iso2022::iso2022(const std::wstring_view s)
    : _s{s}
{
}

void iso2022::write(vt_stream& stream) const
{
    static auto charset_map = build_charset_map();
    static auto last_cs = -1;
    auto last_gset = 0;
    auto locking_shift = [&](auto gset) {
        if (last_gset != gset) {
            last_gset = gset;
            switch (gset) {
                case 0: stream.ls0(); break;
                case 1: stream.ls1(); break;
                case 2: stream.ls2(); break;
                case 3: stream.ls3(); break;
            }
        }
    };
    for (auto wch : _s) {
        if (wch < ' ') {
            stream.write(char(wch));
        } else if (wch >= ' ' && wch <= '~') {
            locking_shift(0);
            stream.write(char(wch));
        } else if (auto search = charset_map.find(wch); search != charset_map.end()) {
            const auto ch = char(search->second & 0xFF);
            const auto cs = search->second >> 8;
            if (last_cs != cs) {
                last_cs = cs;
                if (cs == soft_font)
                    stream.scs(2, " @");
                else if (charset::all[cs].size() == 94)
                    stream.scs(2, charset::all[cs].id());
                else
                    stream.scs96(2, charset::all[cs].id());
            }
            locking_shift(2);
            stream.write(ch);
        } else {
            locking_shift(0);
            stream.write('?');
        }
    }
    locking_shift(0);
}
