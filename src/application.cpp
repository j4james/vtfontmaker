// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "application.h"

#include "charsets.h"
#include "common_dialog.h"
#include "dialog.h"

#include <format>

namespace {

    enum id {
        file_new = 0,
        file_open,
        file_save,
        file_save_as,
        file_properties,
        file_exit,

        edit_undo,
        edit_cut,
        edit_copy,
        edit_paste,
        edit_delete,
        edit_select_all,

        view_next,
        view_prev,
        view_next_used,
        view_prev_used,
        view_double,
        view_reverse,

        transform_invert,
        transform_flip_h,
        transform_flip_v,

        help_view,
        help_about
    };

    using namespace std::literals;

    const auto devices = std::vector{L"VT5xx/VT420 (10x16)"s, L"VT382 (12x30)"s, L"VT340 (10x20)"s, L"VT320 (15x12)"s, L"VT2x0 (10x10)"s, L"Non-standard (16x32)"s};
    const auto screen_sizes = std::vector{L"80x24"s, L"132x24"s, L"80x36"s, L"132x36"s, L"80x48"s, L"132x48"s};
    const auto usages = std::vector{L"Text"s, L"Full cell"s};
    const auto buffers = std::vector{L"First empty buffer"s, L"Buffer #1"s, L"Buffer #2"s};
    const auto erase_types = std::vector{L"All of this buffer"s, L"Only the used characters"s, L"All buffers"s};
    const auto c1_types = std::vector{L"7-bit controls"s, L"8-bit controls"s};

    constexpr auto widths = std::array{10, 12, 10, 15, 10, 16};
    constexpr auto heights = std::array{16, 30, 20, 12, 10, 32};
    constexpr auto screens_values = std::array{0, 2, 11, 12, 21, 22};

    const auto find_value = [](const auto& values, const auto search) {
        const auto match_pos = std::find(values.begin(), values.end(), search);
        return match_pos != values.end() ? std::distance(values.begin(), match_pos) : 0;
    };

}  // namespace

application::application(capabilities& caps, const std::filesystem::path& filepath)
    : _caps{caps}, _status{caps}, _canvas{caps, _glyphs, _status}
{
    _init_menu();
    _menu.render();
    _canvas.render();
    _status.render();
    if (!_open(filepath))
        _new(true);
}

void application::run()
{
    for (auto exit = false; !exit;) {
        _menu.enable(id::edit_undo, _canvas.can_undo());
        _menu.enable(id::edit_paste, _canvas.can_paste());
        const auto key_press = keyboard::read();
        const auto selection = _menu.process_key(key_press);
        if (selection) {
            switch (selection.value()) {
                case id::file_new: _new(); break;
                case id::file_open: _open(); break;
                case id::file_save: _save(); break;
                case id::file_save_as: _save_as(); break;
                case id::file_properties: _properties(); break;
                case id::file_exit: exit = _exit(); break;

                case id::edit_undo: _canvas.undo(); break;
                case id::edit_cut: _canvas.cut_selection(); break;
                case id::edit_copy: _canvas.copy_selection(); break;
                case id::edit_paste: _canvas.paste(); break;
                case id::edit_delete: _canvas.delete_selection(); break;
                case id::edit_select_all: _canvas.select_all(); break;

                case id::view_next: _canvas.next_char(); break;
                case id::view_prev: _canvas.prev_char(); break;
                case id::view_next_used: _canvas.next_char(true); break;
                case id::view_prev_used: _canvas.prev_char(true); break;
                case id::view_double: _canvas.toggle_double_width(); break;
                case id::view_reverse: _canvas.toggle_reverse_screen(); break;

                case id::transform_invert: _canvas.invert(); break;
                case id::transform_flip_h: _canvas.flip_horizontally(); break;
                case id::transform_flip_v: _canvas.flip_vertically(); break;

                case id::help_about: _about(); break;
            }
        } else {
            _canvas.process_key(key_press);
        }
    }
}

void application::_init_menu()
{
    auto file_menu = _menu.add(L"&File");
    file_menu.add(id::file_new, L"&New...", key::ctrl + key::n);
    file_menu.add(id::file_open, L"&Open...", key::ctrl + key::o);
    file_menu.add(id::file_save, L"&Save", key::ctrl + key::s);
    file_menu.add(id::file_save_as, L"Save &As...");
    file_menu.separator();
    file_menu.add(id::file_properties, L"&Properties");
    file_menu.separator();
    file_menu.add(id::file_exit, L"E&xit");
    auto edit_menu = _menu.add(L"&Edit");
    edit_menu.add(id::edit_undo, L"&Undo", key::ctrl + key::z);
    edit_menu.separator();
    edit_menu.add(id::edit_cut, L"Cu&t", key::ctrl + key::x, key::shift + key::del);
    edit_menu.add(id::edit_copy, L"&Copy", key::ctrl + key::c, key::ctrl + key::ins);
    edit_menu.add(id::edit_paste, L"&Paste", key::ctrl + key::v, key::shift + key::ins);
    edit_menu.add(id::edit_delete, L"De&lete", key::del);
    edit_menu.separator();
    edit_menu.add(id::edit_select_all, L"Select &All", key::ctrl + key::a);
    auto view_menu = _menu.add(L"&View");
    view_menu.add(id::view_next, L"&Next Glyph", key::pgdn);
    view_menu.add(id::view_prev, L"&Previous Glyph", key::pgup);
    view_menu.add(id::view_next_used, L"Next &Used Glyph", key::ctrl + key::pgdn);
    view_menu.add(id::view_prev_used, L"Previous U&sed Glyph", key::ctrl + key::pgup);
    view_menu.separator();
    view_menu.add(id::view_double, L"&Double Width");
    view_menu.add(id::view_reverse, L"&Reverse Video");
    auto transform_menu = _menu.add(L"&Transform");
    transform_menu.add(id::transform_invert, L"&Invert Pixels");
    transform_menu.add(id::transform_flip_h, L"Flip &Horizontally");
    transform_menu.add(id::transform_flip_v, L"Flip &Vertically");
    auto help_menu = _menu.add(L"&Help");
    help_menu.add(id::help_view, L"&View Help", key::pf1, key::help);
    help_menu.separator();
    help_menu.add(id::help_about, std::format(L"&About {}", wname));
}

bool application::_can_clear()
{
    _canvas.flush();
    if (!_status.dirty())
        return true;
    else {
        const auto filename = _status.filename();
        const auto message = L"Do you want to save changes to " + filename + L"?";
        using id = common_dialog::id;
        switch (common_dialog::message_box(wname, message, id::yes | id::no | id::cancel)) {
            case id::yes: return _save();
            case id::no: return true;
            default: return false;
        }
    }
}

bool application::_clear(const bool use_defaults)
{
    if (use_defaults) {
        _glyphs.clear();
        return true;
    }

    auto dlg = dialog{L"New"};
    auto& device_field = dlg.add_dropdown(L"Target device", devices);
    auto& screen_size_field = dlg.add_dropdown(L"Target screen", screen_sizes);
    auto& usage_field = dlg.add_dropdown(L"Font usage", usages);
    auto& charset_field = dlg.add_dropdown(L"Character set", charset::names());
    auto& buttons = dlg.add_group(dialog::alignment::right);
    buttons.add_button(L"OK", 1, true);
    buttons.add_button(L"Cancel", 2);

    device_field.on_change([&] {
        const auto device = device_field.selection();
        const auto screen = screen_size_field.selection();
        // VT5xx/VT420 supports all screen size, the "custom" device only
        // supports 80x24, and everything else support 80x24 and 132x24.
        if (device == 0)
            screen_size_field.options(screen_sizes);
        else if (device == 5)
            screen_size_field.options({L"80x24"});
        else
            screen_size_field.options({L"80x24", L"132x24"});
        screen_size_field.selection(screen);
    });
    screen_size_field.on_change([&] {
        const auto device = device_field.selection();
        const auto screen = screen_size_field.selection();
        const auto usage = usage_field.selection();
        // VT2x0 only supports text usage at 80x24.
        if (device == 4 && screen == 0)
            usage_field.options({L"Text"});
        else
            usage_field.options(usages);
        // VT2x0 only supports 94-glyph character sets.
        if (device == 4)
            charset_field.options(charset::names_for_size(94));
        else
            charset_field.options(charset::names());
        usage_field.selection(usage);
    });

    const auto& current = _glyphs.params();
    device_field.selection(current.pss() <= 2 ? find_value(heights, _glyphs.cell_height()) : 0);
    screen_size_field.selection(find_value(screens_values, current.pss()));
    usage_field.selection(current.pu() == 2 ? 1 : 0);

    if (dlg.show() == 2) return false;

    const auto device = device_field.selection();
    auto pcmw = widths[device];
    auto pcmh = heights[device];

    const auto size = screen_size_field.selection();
    if (size % 2 == 1) pcmw = pcmw * 80 / 132;
    if (size / 2 == 1) pcmh = 10;
    if (size / 2 == 2) pcmh = 8;
    const auto usage = usage_field.selection();
    if (usage == 0) pcmw = (pcmw * 8 + 5) / 10;
    const auto pcms = pcmw >> 1;

    const auto cs_index = charset_field.selection();
    const auto cs = device == 4 ? charset::from_index(cs_index, 94) : charset::from_index(cs_index);

    auto params = std::vector<int>{};
    params.push_back(0);  // pfn
    params.push_back(0);  // pcn
    params.push_back(0);  // pe
    params.push_back(device == 4 ? pcms : pcmw);
    params.push_back(screens_values[size]);  // pss
    params.push_back(usage ? 2 : 0);         // pu
    if (device != 4) {
        const auto pcss = cs->size() == 96 ? 1 : 0;
        params.push_back(pcmh);
        params.push_back(pcss);
    }

    _glyphs.clear(params, cs->id());
    return true;
}

void application::_new(const bool use_defaults)
{
    if (_can_clear() && _clear(use_defaults)) {
        _filepath.clear();
        _status.filename(L"Untitled");
        _status.character_set(_glyphs.id(), _glyphs.size());
        _status.dirty(false);
        _canvas.refresh();
    }
}

bool application::_save()
{
    if (_filepath.empty())
        return _save_as();
    else {
        _canvas.flush();
        const auto success = _glyphs.save(_filepath);
        if (success) _status.dirty(false);
        return success;
    }
}

bool application::_save_as()
{
    _canvas.flush();
    auto filepath = _filepath;
    if (filepath.empty()) {
        filepath = std::filesystem::current_path();
        filepath.append(L"Untitled.fnt");
    }
    const auto new_filepath = common_dialog::save(filepath);
    if (new_filepath.empty())
        return false;
    else {
        const auto success = _glyphs.save(new_filepath);
        if (success) {
            _filepath = new_filepath;
            _status.filename(_filepath.filename().wstring());
            _status.dirty(false);
        }
        return success;
    }
}

bool application::_open()
{
    if (_can_clear())
        return _open(common_dialog::open());
    else
        return false;
}

bool application::_open(const std::filesystem::path& filepath)
{
    using id = common_dialog::id;
    if (filepath.empty())
        return false;
    const auto filename = filepath.filename().wstring();
    if (!std::filesystem::exists(filepath)) {
        const auto message = filename + L"\nFile not found.";
        common_dialog::message_box(wname, message, id::ok);
        return false;
    }
    if (!_glyphs.load(filepath)) {
        const auto message = filename + L"\nThis is not a valid font file or its format\nis not currently supported.";
        common_dialog::message_box(wname, message, id::ok);
        return false;
    }
    _filepath = filepath;
    _status.filename(filepath.filename().wstring());
    _status.character_set(_glyphs.id(), _glyphs.size());
    _status.dirty(false);
    _canvas.refresh();
    return true;
}

void application::_properties()
{
    auto charsets = charset::names_for_size(_glyphs.size());
    auto charset_index = charset::index_of(_glyphs.id(), _glyphs.size()).value_or(-1);
    if (charset_index < 0) {
        const auto id = std::wstring{_glyphs.id().begin(), _glyphs.id().end()};
        charsets.push_back(std::format(L"Other: {}", id));
        charset_index = charsets.size() - 1;
    }
    const auto& current = _glyphs.params();
    const auto buffer_index = current.pfn().value_or(0);
    const auto erase_index = current.pe().value_or(0);
    const auto c1_type = _glyphs.c1_controls();
    const auto c1_index = c1_type ? int(c1_type.value()) : -1;

    auto dlg = dialog{L"Properties"};
    auto& charset_field = dlg.add_dropdown(L"Character set", charsets);
    auto& buffer_field = dlg.add_dropdown(L"Target buffer", buffers);
    auto& erase_field = dlg.add_dropdown(L"Erased range", erase_types);
    auto* c1_field = c1_type.has_value() ? &dlg.add_dropdown(L"Sequence format", c1_types) : nullptr;
    auto& buttons = dlg.add_group(dialog::alignment::right);
    buttons.add_button(L"Save", 1, true);
    buttons.add_button(L"Cancel", 2);

    charset_field.selection(charset_index);
    buffer_field.selection(buffer_index);
    erase_field.selection(erase_index);
    if (c1_field) c1_field->selection(c1_index);

    if (dlg.show() == 1) {
        if (charset_field.selection() != charset_index) {
            const auto cs = charset::from_index(charset_field.selection(), _glyphs.size());
            if (cs) {
                _glyphs.id(cs->id());
                const auto status_index = _status.index();
                _status.character_set(_glyphs.id(), _glyphs.size());
                _status.index(status_index);
                _status.dirty(true);
            }
        }
        if (buffer_field.selection() != buffer_index) {
            _glyphs.params().pfn(buffer_field.selection());
            _status.dirty(true);
        }
        if (erase_field.selection() != erase_index) {
            _glyphs.params().pe(erase_field.selection());
            _status.dirty(true);
        }
        if (c1_field && c1_field->selection() != c1_index) {
            _glyphs.c1_controls(c1_field->selection());
            _status.dirty(true);
        }
    }
}

bool application::_exit()
{
    return _can_clear();
}

void application::_about()
{
    auto dlg = dialog{L"About"};
    auto& group = dlg.add_group();
    auto& left = group.add_group();
    auto& right = group.add_group();
    left.add_text(L" \uE041\uE042\uE043");
    left.add_text(L"\uE044\uE045\uE046 ");
    right.add_text(wname);
    right.add_text(std::format(L"Version {}.{}.{}", major_version, minor_version, patch_number));
    dlg.add_gap();
    dlg.add_text(L"©2024 James Holderness");
    dlg.add_text(L"All Rights Reserved");
    auto& buttons = dlg.add_group(dialog::alignment::right);
    buttons.add_button(L"OK", 0, true);
    dlg.show();
}
