// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "common_dialog.h"

#include "dialog.h"
#include "os.h"

#include <chrono>
#include <format>

using direction = dialog::direction;
using alignment = dialog::alignment;

namespace {

    class file_entry {
    public:
        using time_type = std::filesystem::file_time_type;

        file_entry(const std::wstring_view name, const std::filesystem::path& path);
        file_entry(const std::filesystem::directory_entry& entry, const bool is_directory);
        bool is_file() const;
        bool is_directory() const;
        const std::wstring& name() const;
        void add_to_list(list_control& list) const;

    private:
        std::wstring _name;
        bool _is_directory = false;
        time_type _time = {};
        uintmax_t _size = 0;
    };

    file_entry::file_entry(const std::wstring_view name, const std::filesystem::path& path)
        : _name{name}, _is_directory{true}
    {
        try {
            _time = std::filesystem::last_write_time(path);
        } catch (...) {
        }
    }

    file_entry::file_entry(const std::filesystem::directory_entry& entry, const bool is_directory)
        : _name{entry.path().filename().wstring()}, _is_directory{is_directory}
    {
        try {
            _time = entry.last_write_time();
            if (!is_directory) _size = entry.file_size();
        } catch (...) {
        }
    }

    bool file_entry::is_file() const
    {
        return !_is_directory;
    }

    bool file_entry::is_directory() const
    {
        return _is_directory;
    }

    const std::wstring& file_entry::name() const
    {
        return _name;
    }

    void file_entry::add_to_list(list_control& list) const
    {
        const auto time = std::format(L"{:%Y/%m/%d}", _time);
        if (_is_directory) {
            list.add({_name, time, L""});
        } else {
            static const auto us = std::locale("");
            const auto size = std::format(us, L"{:7L} KB", (_size + 1023) / 1024);
            list.add({_name, time, size});
        }
    }

    class file_dialog {
    public:
        enum class type {
            save,
            open
        };

        file_dialog(const type type);
        std::filesystem::path show(const std::filesystem::path& folder, const std::wstring_view name);

    private:
        bool _load_entries(const std::filesystem::path& folder);
        bool _handle_key(const key key_press);
        void _handle_list_change();
        bool _validate_selection() const;
        std::filesystem::path _selected_path() const;
        static std::wstring _path_string(const std::filesystem::path& path, const size_t max_length);

        type _type;
        dialog _dlg;
        text_control& _folder_field;
        list_control& _list_field;
        input_control& _name_field;
        std::vector<file_entry> _entries;
        std::filesystem::path _selected_folder;
    };

    file_dialog::file_dialog(const type type)
        : _type{type},
          _dlg{type == type::open ? L"Open" : L"Save As"},
          _folder_field{_dlg.add_text(L"")},
          _list_field{_dlg.add_list({L"Name", L"Date", L"Size"}, {24, 10, 10}, 9)},
          _name_field{_dlg.add_input(L"Filename", 40)}
    {
        auto& buttons = _dlg.add_group(alignment::right);
        buttons.add_button(type == type::open ? L"Open" : L"Save", 1, true);
        buttons.add_button(L"Cancel", 2);

        _list_field.on_key_press([&](const auto key_press) {
            return _handle_key(key_press);
        });
        _list_field.on_change([&] {
            return _handle_list_change();
        });
        _dlg.on_validate([&](const int return_code) {
            return return_code == 2 || _validate_selection();
        });
        _dlg.focus(_name_field);
    }

    std::filesystem::path file_dialog::show(const std::filesystem::path& folder, const std::wstring_view name)
    {
        _name_field.value(name);
        if (_load_entries(folder)) {
            if (_dlg.show() == 1) {
                const auto selected_path = _selected_path();
                if (selected_path.empty()) return {};
                std::filesystem::current_path(_selected_folder);
                return selected_path;
            }
        }
        return {};
    }

    bool file_dialog::_load_entries(const std::filesystem::path& folder)
    try {
        auto folders = std::vector<file_entry>{};
        if (folder.has_parent_path() && folder.has_relative_path())
            folders.emplace_back(L"..", folder.parent_path());

        auto files = std::vector<file_entry>{};
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            if (!os::is_file_hidden(entry.path())) {
                if (entry.is_directory())
                    folders.emplace_back(entry, true);
                else
                    files.emplace_back(entry, false);
            }
        }

        const auto selected_folder = std::find_if(folders.begin(), folders.end(), [&](const auto& folder) {
            return folder.name() == _selected_folder.filename();
        });

        _entries = folders;
        _entries.insert(_entries.end(), files.begin(), files.end());
        _selected_folder = folder;
        _folder_field.value(_path_string(folder, 50));
        _list_field.clear();
        for (const auto& entry : _entries)
            entry.add_to_list(_list_field);

        if (selected_folder != folders.end()) {
            const auto selected_offset = std::distance(folders.begin(), selected_folder);
            _list_field.selection(selected_offset);
        }

        return true;
    } catch (...) {
        using id = common_dialog::id;
        const auto title = folder.filename().wstring();
        const auto message = L"You don't currently have permission to\naccess this folder.";
        common_dialog::message_box(title, message, id::cancel);
        return false;
    }

    bool file_dialog::_handle_key(const key key_press)
    {
        if (key_press == key::enter) {
            const auto selection = _list_field.selection();
            const auto& selected_entry = _entries[selection];
            if (selected_entry.is_directory()) {
                auto selected_path = _selected_folder;
                if (selected_entry.name() == L"..")
                    selected_path = selected_path.parent_path();
                else
                    selected_path.append(selected_entry.name());
                _load_entries(selected_path);
                return true;
            }
        } else if (key_press == key::bksp) {
            if (_selected_folder.has_parent_path() && _selected_folder.has_relative_path()) {
                _load_entries(_selected_folder.parent_path());
                return true;
            }
        }
        return false;
    }

    void file_dialog::_handle_list_change()
    {
        const auto selection = _list_field.selection();
        const auto& selected_entry = _entries[selection];
        if (selected_entry.is_file())
            _name_field.value(selected_entry.name());
    }

    bool file_dialog::_validate_selection() const
    {
        using id = common_dialog::id;
        const auto selected_path = _selected_path();
        if (selected_path.empty()) return false;
        const auto name = selected_path.filename().wstring();
        const auto exists = std::filesystem::exists(selected_path);
        if (_type == type::open && !exists) {
            const auto title = L"Open";
            const auto message = name + L"\nFile not found.\nCheck the filename and try again.";
            common_dialog::message_box(title, message, id::ok);
            return false;
        }
        if (_type == type::save && exists) {
            const auto title = L"Confirm Save As";
            const auto message = name + L" already exists.\nDo you want to replace it?";
            const auto choice = common_dialog::message_box(title, message, id::yes | id::no);
            return choice == id::yes;
        }
        return true;
    }

    std::filesystem::path file_dialog::_selected_path() const
    {
        const auto selected_name = _name_field.value();
        if (selected_name.empty()) return {};
        auto selected_path = _selected_folder;
        return selected_path.append(selected_name);
    }

    std::wstring file_dialog::_path_string(const std::filesystem::path& path, const size_t max_length)
    {
        constexpr auto prefix = L"> ";
        auto full_path = prefix + path.wstring();
        if (full_path.length() <= max_length)
            return full_path;
        else {
            auto segments = std::vector<std::wstring>();
            for (auto segment : path.relative_path())
                segments.emplace_back(segment.wstring());

            const auto separator = static_cast<wchar_t>(path.preferred_separator);
            auto root = prefix + path.root_name().wstring() + separator + L"...";
            auto total_length = root.length();
            auto keep = segments.size();
            while (keep > 0 && total_length + segments[keep - 1].length() < max_length)
                total_length += segments[--keep].length() + 1;

            for (auto i = keep; i < segments.size(); i++)
                root += separator + segments[i];
            return root.substr(0, max_length);
        }
    }

}  // namespace

namespace common_dialog {

    std::filesystem::path open()
    {
        const auto folder = std::filesystem::current_path();
        auto dlg = file_dialog{file_dialog::type::open};
        return dlg.show(folder, L"");
    }

    std::filesystem::path save(const std::filesystem::path& filepath)
    {
        const auto folder = filepath.parent_path();
        const auto name = filepath.filename().wstring();
        auto dlg = file_dialog{file_dialog::type::save};
        return dlg.show(folder, name);
    }

    int message_box(const std::wstring_view title, const std::wstring_view message, const int buttons)
    {
        auto dlg = dialog{title};
        const auto max_width = dlg.max_width() - 2;
        const auto wrap_text = [&dlg, max_width](const auto text) {
            auto off = 0;
            while (text.length() > off + max_width) {
                dlg.add_text(text.substr(off, max_width));
                off += max_width;
            }
            dlg.add_text(text.substr(off));
        };
        for (auto start = 0, end = 0; end = message.find('\n', start); start = end + 1) {
            if (end == std::wstring::npos) {
                wrap_text(message.substr(start));
                break;
            }
            wrap_text(message.substr(start, end - start));
        }
        auto& group = dlg.add_group(alignment::center);
        if (buttons & id::yes)
            group.add_button(L"Yes", id::yes, true);
        if (buttons & id::no)
            group.add_button(L"No", id::no);
        if (buttons & id::ok)
            group.add_button(L"OK", id::ok, true);
        if (buttons & id::cancel)
            group.add_button(L"Cancel", id::cancel);
        return dlg.show();
    }

}  // namespace common_dialog
