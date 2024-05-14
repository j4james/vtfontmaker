// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "menu.h"

#include "macros.h"

namespace color {

    constexpr auto primary_init = {0, 1, 7, 37, 40};  // Black on BrightestWhite
    constexpr auto primary_normal = {1, 37};          // BrightestWhite background
    constexpr auto primary_focus = {22, 35};          // LighterBlue background

    constexpr auto secondary_init = {0, 1, 7, 36, 40};  // Black on BrightWhite
    constexpr auto secondary_normal = {1, 36};          // BrightWhite background
    constexpr auto secondary_focus = {22, 34};          // LightBlue background
    constexpr auto secondary_disabled = {46};           // LightGray foreground

}  // namespace color

namespace {

    std::string generate_accelerator_name(const std::optional<key> accelerator)
    {
        if (accelerator)
            return keyboard::to_string(accelerator.value());
        else
            return "";
    }

    std::wstring markup_label(const std::wstring_view name)
    {
        const auto ampersand = name.find('&');
        if (ampersand == std::wstring::npos)
            return std::wstring{name};
        else {
            auto result = std::wstring{name.substr(0, ampersand)};
            result += L"\033[4m";
            result += name[ampersand + 1];
            result += L"\033[24m";
            result += name.substr(ampersand + 2);
            return result;
        }
    }

    int calculate_shortcut(const std::wstring_view name)
    {
        const auto pos = name.find('&');
        if (pos != std::string::npos) {
            const auto ch = std::toupper(name[pos + 1]);
            if (ch >= 'A' && ch <= 'Z') return ch - 'A';
        }
        return -1;
    }

}  // namespace

int menu_group::_close_macro = -1;

menu_group::menu_group(const std::wstring_view name, const int xoffset, const std::unordered_set<int>& disabled_ids)
    : _name{name}, _left{xoffset}, _disabled_ids{disabled_ids}
{
    _open_macro = macro_manager::reserve_id();
    if (_close_macro == -1) {
        _close_macro = macro_manager::create([](auto& macro) {
            macro.deccra({}, {}, {}, {}, 2, {}, {}, 1);
            macro.decstbm();
            macro.decslrm();
            macro.deccara({}, {}, 1, {}, color::primary_normal);
        });
    }
}

int menu_group::left() const
{
    return _left;
}

int menu_group::right() const
{
    return _left + _name.length();
}

int menu_group::macro_id() const
{
    return _open_macro;
}

void menu_group::add(const int entry_id, const std::wstring_view entry_name)
{
    _entry_ids.push_back(entry_id);
    _shortcuts.emplace(calculate_shortcut(entry_name), entry_id);
    _entry_count++;
}

void menu_group::render() const
{
    vtout.cup(1, _left + 1);
    vtout.write(markup_label(_name));
}

void menu_group::open()
{
    vtout.decinvm(_open_macro);
    for (auto i = 0; i < _entry_ids.size(); i++)
        if (_disabled(i))
            vtout.deccara(i + 1, {}, i + 1, {}, color::secondary_disabled);
    _focus_index = 0;
    while (_disabled(_focus_index))
        _focus_index++;
    vtout.deccara(_focus_index + 1, {}, _focus_index + 1, {}, color::secondary_focus);
}

void menu_group::close()
{
    vtout.decinvm(_close_macro);
}

std::optional<int> menu_group::process_key(const key keypress)
{
    switch (keypress) {
        case key::up:
            _move_focus(-1);
            return {};
        case key::down:
            _move_focus(+1);
            return {};
        default:
            if (auto id = _selection_for_key(keypress))
                if (!_disabled_ids.contains(id.value()))
                    return id;
            return {};
    }
}

bool menu_group::_disabled(const int entry_index) const
{
    return _disabled_ids.contains(_entry_ids[entry_index]);
}

void menu_group::_move_focus(const int offset)
{
    vtout.deccara(_focus_index + 1, {}, _focus_index + 1, {}, color::secondary_normal);
    do {
        _focus_index = (_focus_index + offset + _entry_count) % _entry_count;
    } while (_disabled(_focus_index));
    vtout.deccara(_focus_index + 1, {}, _focus_index + 1, {}, color::secondary_focus);
}

std::optional<int> menu_group::_selection_for_key(const key k) const
{
    if (k == key::enter)
        return _entry_ids[_focus_index];
    if (k >= key::a && k <= key::z)
        if (const auto match = _shortcuts.find(k - key::a); match != _shortcuts.end())
            return match->second;
    if (k >= key::alt + key::a && k <= key::alt + key::z)
        if (const auto match = _shortcuts.find(k - (key::alt + key::a)); match != _shortcuts.end())
            return match->second;
    return {};
}

menu_builder::menu_builder(menu_group& group, std::unordered_map<key, int>& accelerators)
    : _group{group}, _accelerators{accelerators}
{
}

menu_builder::~menu_builder()
{
    macro_manager::create(_group.macro_id(), [&](auto& macro) {
        macro.deccara({}, _group.left(), 1, _group.right(), color::primary_focus);
        macro.decslrm(_group.left(), _group.left() + _width);
        macro.decstbm(2, _entries.size() + 1);
        macro.deccra({}, {}, {}, {}, 1, {}, {}, 2);
        macro.sgr(color::secondary_init);
        auto yoffset = 1;
        for (auto& entry : _entries) {
            const auto& [name, accelerator_name, has_separator] = entry;
            macro.cup(yoffset++);
            if (has_separator) macro.sgr({53});
            macro.write(' ');
            macro.write(markup_label(name));
            macro.write_spaces(_width - name.length() - accelerator_name.length());
            macro.write(accelerator_name);
            macro.write(' ');
            if (has_separator) macro.sgr({55});
        }
    });
}

void menu_builder::separator()
{
    _want_separator = true;
}

void menu_builder::add(const int id, const std::wstring_view name, const std::optional<key> accelerator, const std::optional<key> accelerator2)
{
    const auto accelerator_name = generate_accelerator_name(accelerator);
    const auto accelerator_width = !accelerator_name.empty() ? accelerator_name.length() + 3 : 0;
    _group.add(id, name);
    _width = std::max<int>(_width, name.length() + accelerator_width);
    _entries.emplace_back(name, accelerator_name, _want_separator);
    if (accelerator)
        _accelerators.emplace(accelerator.value(), id);
    if (accelerator2)
        _accelerators.emplace(accelerator2.value(), id);
    _want_separator = false;
}

menu_builder menu::add(const std::wstring_view name)
{
    const auto index = static_cast<int>(_groups.size());
    const auto xoffset = _width_used + 2;
    const auto& group = _groups.emplace_back(std::make_unique<menu_group>(name, xoffset, _disabled_ids));
    _shortcuts.emplace(calculate_shortcut(name), index);
    _width_used += name.length() + 1;
    return {*group, _accelerators};
}

void menu::render() const
{
    vtout.deccara({}, {}, 1, {}, color::primary_init);
    vtout.sgr(color::primary_init);
    vtout.cup();
    for (const auto& group : _groups)
        group->render();
}

void menu::enable(const int entry_id, const bool enabled)
{
    if (enabled)
        _disabled_ids.erase(entry_id);
    else
        _disabled_ids.insert(entry_id);
}

std::optional<int> menu::process_key(const key keypress)
{
    if (keypress == key::f10)
        _open_group(0);
    else if (auto group_index = _group_for_key(keypress))
        _open_group(group_index);
    else if (auto selection = _selection_for_key(keypress))
        return selection;
    else
        return {};
    auto selection = std::optional<int>{};
    auto width = _groups.size();
    while (!selection.has_value()) {
        const auto k = keyboard::read();
        switch (k) {
            case key::right:
                _open_group(_index() + 1);
                break;
            case key::left:
                _open_group(_index() - 1);
                break;
            case key::f10:
            case key::bksp:
                selection = -1;
                break;
            default:
                if (auto group_index = _group_for_key(k))
                    _open_group(group_index);
                else
                    selection = _groups[_index()]->process_key(k);
                break;
        }
    }
    _open_group({});
    return selection;
}

std::optional<int> menu::_selection_for_key(const key k) const
{
    if (const auto match = _accelerators.find(k); match != _accelerators.end())
        if (!_disabled_ids.contains(match->second))
            return match->second;
    return {};
}

std::optional<int> menu::_group_for_key(const key k) const
{
    if (k >= key::alt + key::a && k <= key::alt + key::z)
        if (const auto match = _shortcuts.find(k - (key::alt + key::a)); match != _shortcuts.end())
            return match->second;
    return {};
}

void menu::_open_group(const std::optional<int> new_index)
{
    if (_open_index)
        _groups[_index()]->close();
    if (new_index)
        _open_index = int((new_index.value() + _groups.size()) % _groups.size());
    else
        _open_index = {};
    if (_open_index)
        _groups[_index()]->open();
}

int menu::_index() const
{
    return _open_index.value();
}
