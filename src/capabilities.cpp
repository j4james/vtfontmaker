// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "capabilities.h"

#include "os.h"
#include "vt.h"

#include <cstring>

using namespace std::string_literals;

capabilities::capabilities()
{
    // Save the cursor position.
    vtout.decsc();
    // Request 7-bit C1 controls from the terminal.
    vtout.s7c1t();
    // Determine the screen size.
    vtout.cup(999, 999);
    vtout.dsr(6);
    const auto size = _query(R"(\x1B\[(\d+);(\d+)R)", false);
    if (!size.empty()) {
        height = std::stoi(size[1]);
        width = std::stoi(size[2]);
    }
    // Retrieve the device attributes report.
    _query_device_attributes();
    // Retrieve the keyboard type.
    _query_keyboard_type();
    // Disable scrollback and page coupling.
    _original_decrpl = query_mode(112);
    _original_decpccm = query_mode(64);
    vtout.rm('?', {112, 64});
    // Try and move to page 3 and check the result with DECXCPR.
    vtout.ppa(3);
    vtout.dsr('?', 6);
    const auto page = _query(R"(\x1B\[\??\d+;\d+(?:;(\d+))?R)", true);
    if (!page.empty() && page[1].matched)
        has_pages = std::stoi(page[1]) == 3;
    // Restore the cursor position.
    vtout.decrc();
    // Make sure we've returned to page 1.
    vtout.ppa(1);
    vtout.sm('?', 64);
    vtout.rm('?', 64);
}

capabilities::~capabilities()
{
    // Restore the original DECPCCM and DECRPL modes.
    if (_original_decpccm == true)
        vtout.sm('?', 64);
    else if (_original_decpccm == false)
        vtout.rm('?', 64);
    if (_original_decrpl == true)
        vtout.sm('?', 112);
    else if (_original_decrpl == false)
        vtout.rm('?', 112);
    vtout.flush();
}

std::optional<bool> capabilities::query_mode(const int mode) const
{
    vtout.decrqm('?', mode);
    const auto report = _query(R"(\x1B\[\?(\d+);(\d+)\$y)", true);
    if (!report.empty()) {
        const auto returned_mode = std::stoi(report[1]);
        const auto status = std::stoi(report[2]);
        if (returned_mode == mode) {
            if (status == 1) return true;
            if (status == 2) return false;
        }
    }
    return {};
}

std::string capabilities::query_color_table() const
{
    vtout.decctr(2);
    const auto report = _query(R"(\x1BP2\$s(.*)\x1B\\)", true);
    if (!report.empty())
        return report[1];
    else
        return {};
}

void capabilities::_query_keyboard_type()
{
    vtout.dsr('?', 26);
    const auto report = _query(R"(\x1B\[\?27;\d+;\d+;(\d+)n)", true);
    if (!report.empty()) {
        // Likely a PC layout if type is LK443 (2) or PCXAL (5).
        const auto type = std::stoi(report[1]);
        has_pc_keyboard = type == 2 || type == 5;
    } else {
        // If no type found, likely an older terminal with LK201.
        has_pc_keyboard = false;
    }
}

void capabilities::_query_device_attributes()
{
    vtout.da();
    // The Reflection Desktop terminal sometimes uses comma separators
    // instead of semicolons in their DA report, so we allow for either.
    const auto report = _query(R"(\x1B\[\?(\d+)([;,\d]*)c)", false);
    if (!report.empty()) {
        // The first parameter indicates the terminal conformance level.
        const auto level = std::stoi(report[1]);
        // Level 4+ conformance implies support for features 28 and 32.
        if (level >= 64) {
            has_rectangle_ops = true;
            has_macros = true;
        }
        // The remaining parameters indicate additional feature extensions.
        const auto features = report[2].str();
        const auto digits = std::regex(R"(\d+)");
        auto it = std::sregex_iterator(features.begin(), features.end(), digits);
        while (it != std::sregex_iterator()) {
            const auto feature = std::stoi(it->str());
            switch (feature) {
                case 7: has_soft_fonts = true; break;
                case 21: has_horizontal_scrolling = true; break;
                case 22: has_color = true; break;
                case 28: has_rectangle_ops = true; break;
                case 32: has_macros = true; break;
            }
            it++;
        }
    }
}

std::smatch capabilities::_query(const char* pattern, const bool may_not_work)
{
    auto final_char = pattern[strlen(pattern) - 1];
    if (may_not_work) {
        // If we're uncertain this query is supported, we'll send an extra DA
        // or DSR-CPR query to make sure that we get some kind of response.
        if (final_char == 'R') {
            final_char = 'c';
            vtout.da();
        } else {
            final_char = 'R';
            vtout.dsr(6);
        }
    }
    vtout.flush();
    // This needs to be static so the returned smatch can reference it.
    static auto response = std::string{};
    response.clear();
    auto last_escape = 0;
    for (;;) {
        const auto ch = os::getch();
        // Ignore XON, XOFF
        if (ch == '\021' || ch == '\023')
            continue;
        // If we've sent an extra query, the last escape should be the
        // start of that response, which we'll ultimately drop.
        if (may_not_work && ch == '\033')
            last_escape = response.length();
        response += ch;
        if (ch == final_char) break;
    }
    // Drop the extra response if one was requested.
    if (may_not_work)
        response = response.substr(0, last_escape);
    auto match = std::smatch{};
    std::regex_match(response, match, std::regex(pattern));
    return match;
}
