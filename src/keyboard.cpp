// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "keyboard.h"

#include "capabilities.h"
#include "os.h"
#include "vt.h"

#include <vector>

namespace {

    key make_key(const key base, const int offset)
    {
        return static_cast<key>(static_cast<int>(base) + offset);
    }

    key remove_modifier(const key base, const key modifier)
    {
        return static_cast<key>(static_cast<int>(base) & ~static_cast<int>(modifier));
    }

    bool has_modifier(const key base, const key modifier)
    {
        return (static_cast<int>(base) & static_cast<int>(modifier)) != 0;
    }

    key make_modifier(const std::vector<int>& parms)
    {
        auto modifier = key::unmodified;
        if (parms.size() >= 2) {
            const auto modifier_parm = parms[1] - 1;
            if (modifier_parm > 0) {
                if (modifier_parm & 1)
                    modifier = modifier + key::shift;
                if (modifier_parm & 2)
                    modifier = modifier + key::alt;
                if (modifier_parm & 4)
                    modifier = modifier + key::ctrl;
            }
        }
        return modifier;
    }

    bool has_pc_keyboard = true;

    std::string key_label(const std::string pc_label, const std::string vt_label)
    {
        return has_pc_keyboard ? pc_label : vt_label;
    }

}  // namespace

void keyboard::initialize(const capabilities& caps)
{
    has_pc_keyboard = caps.has_pc_keyboard;
}

key keyboard::read()
{
    enum {
        ground,
        esc,
        csi,
        ss3
    } state = ground;
    auto parm = 0;
    auto parm_list = std::vector<int>{};
    vtout.flush();
    for (;;) {
        const auto ch = os::getch();
        switch (state) {
            case ground:
                parm = 0;
                parm_list.clear();
                switch (ch) {
                    case '\177': return key::bksp;
                    case '\b': return key::bksp;
                    case '\t': return key::tab;
                    case '\r': return key::enter;
                    case '\n': return key::enter;
                    case ' ': return key::space;
                    case '\033': state = esc; break;
                }
                if (ch >= 1 && ch <= 26)
                    return make_key(key::ctrl + key::a, ch - 1);
                if (ch >= 'A' && ch <= 'Z')
                    return make_key(key::shift + key::a, ch - 'A');
                if (ch >= ' ' && ch < 127)
                    return make_key(key::space, ch - ' ');
                break;
            case esc:
                if (ch >= 'a' && ch <= 'z')
                    return make_key(key::alt + key::a, ch - 'a');
                switch (ch) {
                    case '[': state = csi; break;
                    case 'O': state = ss3; break;
                    case '\033': state = esc; break;
                    default: state = ground; break;
                }
                break;
            case csi:
                if (ch >= '0' && ch <= '9') {
                    parm = parm * 10 + (ch - '0');
                } else if (ch == ';') {
                    parm_list.push_back(parm);
                    parm = 0;
                } else {
                    state = ground;
                    parm_list.push_back(parm);
                    auto modifier = make_modifier(parm_list);
                    switch (ch) {
                        case 'Z': return key::shift + key::tab;
                        case 'A': return modifier + key::up;
                        case 'B': return modifier + key::down;
                        case 'C': return modifier + key::right;
                        case 'D': return modifier + key::left;
                        case 'H': return modifier + key::home;
                        case 'F': return modifier + key::end;
                        case '~':
                            switch (parm_list.front()) {
                                case 1: return modifier + key::home;  // VT Find
                                case 2: return modifier + key::ins;   // VT Insert Here
                                case 3: return modifier + key::del;   // VT Remove
                                case 4: return modifier + key::end;   // VT Select
                                case 5: return modifier + key::pgup;  // VT Prev Screen
                                case 6: return modifier + key::pgdn;  // VT Next Screen
                                case 7: return modifier + key::left;
                                case 8: return modifier + key::down;
                                case 9: return modifier + key::up;
                                case 10: return modifier + key::right;
                                case 11: return modifier + key::f1;
                                case 12: return modifier + key::f2;
                                case 13: return modifier + key::f3;
                                case 14: return modifier + key::f4;
                                case 15: return modifier + key::f5;
                                case 17: return modifier + key::f6;
                                case 18: return modifier + key::f7;
                                case 19: return modifier + key::f8;
                                case 20: return modifier + key::f9;
                                case 21: return modifier + key::f10;
                                case 28: return modifier + key::help;
                            }
                            break;
                    }
                }
                break;
            case ss3:
                state = ground;
                switch (ch) {
                    case 'P': return key::pf1;
                    case 'Q': return key::pf2;
                    case 'R': return key::pf3;
                    case 'S': return key::pf4;
                }
                break;
        }
    }
}

std::optional<wchar_t> keyboard::printable(const key key_press)
{
    if (key_press >= key::space && key_press <= key::tilde)
        return static_cast<wchar_t>(key_press);
    const auto unshifted_key = remove_modifier(key_press, key::shift);
    if (unshifted_key >= key::a && unshifted_key <= key::z)
        return static_cast<wchar_t>(unshifted_key) + ('A' - 'a');
    return {};
}

std::string keyboard::to_string(const key key_press)
{
    using namespace std::string_literals;
    auto modifiers = ""s;
    if (has_modifier(key_press, key::ctrl)) modifiers += "Ctrl+";
    if (has_modifier(key_press, key::alt)) modifiers += "Alt+";
    if (has_modifier(key_press, key::shift)) modifiers += "Shift+";
    const auto k = remove_modifier(key_press, key::ctrl + key::alt + key::shift);
    if (k == key::pgup)
        return modifiers + key_label("PgUp", "Prev");
    if (k == key::pgdn)
        return modifiers + key_label("PgDn", "Next");
    if (k == key::del)
        return modifiers + key_label("Del", "Remove");
    if (k == key::tab)
        return modifiers + "Tab"s;
    if (k >= key::pf1 && k <= key::pf4)
        return modifiers + key_label("F", "PF") + std::to_string(k - key::pf1 + 1);
    if (k >= key::f1 && k <= key::f10)
        return modifiers + "F"s + std::to_string(k - key::f1 + 1);
    if (k >= key::a && k <= key::z)
        return modifiers + static_cast<char>('A' + (k - key::a));
    return ""s;
}
