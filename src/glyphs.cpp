// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "glyphs.h"

#include <fstream>
#include <regex>

namespace {

    template <typename T>
    void split(const std::string_view s, const char separator, const T&& callback)
    {
        auto start = 0;
        for (;;) {
            const auto slash = s.find(separator, start);
            if (slash != std::string::npos) {
                callback(s.substr(start, slash - start));
                start = slash + 1;
            } else {
                callback(s.substr(start));
                break;
            }
        }
    }

    bool is_sixel_char(const char ch)
    {
        return ch >= '?' && ch <= '~';
    }

    bool is_non_blank_sixel_char(const char ch)
    {
        return ch >= '@' && ch <= '~';
    }

}  // namespace

glyph::glyph(const std::string_view sixels)
    : _sixels{sixels}
{
    split(_sixels, '/', [&](const auto sixel_row) {
        _used_height += 6;
        _used_width = std::max<int>(_used_width,
            std::count_if(sixel_row.begin(), sixel_row.end(), is_sixel_char));
    });
}

const std::string& glyph::str() const
{
    return _sixels;
}

std::vector<int8_t> glyph::pixels(const int cell_width, const int cell_height) const
{
    auto pixels = std::vector<int8_t>(cell_width * cell_height, 0);
    auto y = 0;
    split(_sixels, '/', [&](const auto sixel_row) {
        auto x = 0;
        for (auto ch : sixel_row)
            if (is_sixel_char(ch)) {
                ch -= '?';
                if (x >= cell_width) break;
                for (auto i = 0; i < 6; i++) {
                    if (y + i >= cell_height) break;
                    pixels[(y + i) * cell_width + x] = ch & 1;
                    ch >>= 1;
                }
                x++;
            }
        y += 6;
    });
    return pixels;
}

void glyph::pixels(const int cell_width, const int cell_height, const std::vector<int8_t>& pixels)
{
    // If there are any whitespace characters surrounding the original
    // sixel content, we try and preserve that when updating the glyph.
    const auto start = std::find_if(_sixels.begin(), _sixels.end(), is_sixel_char);
    const auto end = std::find_if(_sixels.rbegin(), _sixels.rend(), is_sixel_char);
    const auto prefix = _sixels.substr(0, start - _sixels.begin());
    const auto suffix = end != _sixels.rend() ? _sixels.substr(_sixels.rend() - end) : "";
    _sixels = prefix;
    for (auto y = 0; y < cell_height; y += 6) {
        if (y) _sixels += "/";
        for (auto x = 0; x < cell_width; x++) {
            auto value = '?';
            for (auto i = 0; i < 6; i++) {
                if (y + i >= cell_height) break;
                if (pixels[(y + i) * cell_width + x]) value += 1 << i;
            }
            _sixels += value;
        }
    }
    _sixels += suffix;
}

bool glyph::used() const
{
    return std::count_if(_sixels.begin(), _sixels.end(), is_non_blank_sixel_char) > 0;
}

glyph_reference::glyph_reference(glyph_manager& manager, const int index)
    : _manager{manager}, _index{index}
{
}

glyph_reference& glyph_reference::operator=(const std::vector<int8_t>& pixels)
{
    _manager._glyph_pixels(_index, pixels);
    return *this;
}

glyph_reference::operator std::vector<int8_t>() const
{
    return _manager._glyph_pixels(_index);
}

bool glyph_reference::used() const
{
    const auto internal_index = _index - _manager._first_index;
    if (internal_index >= 0 && internal_index < _manager._glyphs.size())
        return _manager._glyphs[internal_index].used();
    else
        return false;
}

glyph_manager::glyph_manager()
{
    clear();
}

void glyph_manager::clear()
{
    clear({0, 0, 0, 10, 0, 2, 16, 0}, " @");
}

void glyph_manager::clear(const std::vector<int>& params, const std::string_view id)
{
    c1_controls(false);
    _prefix.clear();
    _suffix.clear();
    _id = id;
    _sixel_prefix.clear();
    _sixel_suffix.clear();
    _parms = std::make_unique<parameters>(params);
    _glyphs.clear();
    _size = _parms->pcss().value_or(0) == 1 ? 96 : 94;
    _first_index = _parms->pcn().value_or(_size == 96 ? 0 : 1);
    std::tie(_cell_width, _cell_height, _pixel_aspect_ratio) = _detect_dimensions();
}

bool glyph_manager::load(const std::filesystem::path& path)
{
    auto file = std::ifstream{path, std::ios::binary};
    const auto size = file_size(path);
    auto contents = std::string(size, '\0');
    file.read(contents.data(), size);

    const auto pattern = R"EGEX((\x1BP|\x90|R"\()([\d\s;]*)\{([\s!-/]*[0-~])(\s*)([\s/;?-~]+?)(\s*)(\x1B|\x9C|\)";))EGEX";
    auto match = std::smatch{};
    if (std::regex_search(contents, match, std::regex(pattern))) {
        _prefix = match.prefix().str();
        _suffix = match.suffix().str();
        _introducer = match[1].str();
        _parms = std::make_unique<parameters>(match[2].str());
        _id = match[3].str();
        _sixel_prefix = match[4].str();
        _sixel_suffix = match[6].str();
        _terminator = match[7].str();
        _glyphs.clear();
        split(match[5].str(), ';', [&](const auto glyph_sixels) {
            _glyphs.emplace_back(glyph_sixels);
        });
        _size = _parms->pcss().value_or(0) == 1 ? 96 : 94;
        _first_index = _parms->pcn().value_or(_size == 96 ? 0 : 1);
        std::tie(_cell_width, _cell_height, _pixel_aspect_ratio) = _detect_dimensions();
        return true;
    }
    return false;
}

bool glyph_manager::save(const std::filesystem::path& path)
{
    auto contents = _prefix;
    contents += _introducer;
    contents += _parms->str();
    contents += "{";
    contents += _id;
    contents += _sixel_prefix;

    for (auto i = 0; i < _glyphs.size(); i++) {
        if (i) contents += ";";
        contents += _glyphs[i].str();
    }

    contents += _sixel_suffix;
    contents += _terminator;
    contents += _suffix;

    auto file = std::ofstream{path, std::ios::binary};
    file << contents;

    return true;
}

glyph_manager::parameters& glyph_manager::params()
{
    return *_parms;
}

const glyph_manager::parameters& glyph_manager::params() const
{
    return *_parms;
}

const std::string& glyph_manager::id() const
{
    return _id;
}

void glyph_manager::id(const std::string_view id)
{
    _id = id;
}

const std::optional<bool> glyph_manager::c1_controls() const
{
    if (_introducer == "\x90")
        return true;
    else if (_introducer == "\x1BP")
        return false;
    else
        return {};
}

void glyph_manager::c1_controls(const bool c1_8bit)
{
    if (c1_8bit) {
        _introducer = "\x90";
        _terminator = "\x9C";
    } else {
        _introducer = "\x1BP";
        _terminator = "\x1B\\";
    }
}

int glyph_manager::size() const
{
    return _size;
}

int glyph_manager::first_used() const
{
    return _first_index;
}

int glyph_manager::cell_width() const
{
    return _cell_width;
}

int glyph_manager::cell_height() const
{
    return _cell_height;
}

int glyph_manager::pixel_aspect_ratio() const
{
    return _pixel_aspect_ratio;
}

glyph_reference glyph_manager::operator[](const int index)
{
    return {*this, index};
}

std::vector<int8_t> glyph_manager::_glyph_pixels(const int index)
{
    const auto internal_index = index - _first_index;
    if (internal_index < 0 || internal_index >= _glyphs.size())
        return std::vector<int8_t>(_cell_width * _cell_height, 0);
    else
        return _glyphs[internal_index].pixels(_cell_width, _cell_height);
}

void glyph_manager::_glyph_pixels(const int index, const std::vector<int8_t>& pixels)
{
    while (index < _first_index) {
        _glyphs.insert(_glyphs.begin(), glyph{""});
        _parms->pcn(--_first_index);
    }
    const auto internal_index = index - _first_index;
    while (internal_index >= _glyphs.size())
        _glyphs.emplace_back("");
    _glyphs[internal_index].pixels(_cell_width, _cell_height, pixels);
}

std::tuple<int, int, int> glyph_manager::_detect_dimensions()
{
    auto screen_properties = std::make_tuple(80, 24, 200);
    switch (_parms->pss().value_or(0)) {
        case 2: screen_properties = std::make_tuple(132, 24, 334); break;
        case 11: screen_properties = std::make_tuple(80, 36, 125); break;
        case 12: screen_properties = std::make_tuple(132, 36, 209); break;
        case 21: screen_properties = std::make_tuple(80, 48, 100); break;
        case 22: screen_properties = std::make_tuple(132, 48, 167); break;
    }
    const auto [cpp, lpp, cell_aspect_ratio] = screen_properties;
    auto declared_width = _parms->pcmw().value_or(0);
    auto declared_height = _parms->pcmh().value_or(0);
    auto declared_as_matrix = declared_width >= 2 && declared_width <= 4;
    if (declared_as_matrix) {
        // If size declared as a matrix, it's assumed to be targetting a VT2xx
        // with a 2:1 pixel AR. The cell size is 8x10, 6x10, or 5x10, for matrix
        // values 4, 3, and 2, although 80 column mode is always 8x10.
        if (cpp == 80 || declared_width == 4)
            return {8, 10, 200};
        else if (declared_width == 3)
            return {6, 10, 200};
        else
            return {5, 10, 200};
    }
    const auto text_usage = _parms->pu() != 2;
    const auto text_adjust = [=](const auto full_width) {
        if (text_usage && declared_width)
            return std::min(declared_width, full_width);
        else
            return full_width;
    };
    if (lpp != 24) {
        // If LPP isn't 24, assume VT420/VT5xx with 1.25:1 pixel AR.
        const auto cell_width = cpp == 132 ? 6 : 10;
        const auto cell_height = lpp == 48 ? 8 : 10;
        if (declared_width <= cell_width && declared_height <= cell_height)
            return {text_adjust(cell_width), cell_height, 125};
    }
    if (declared_width && declared_height && !text_usage) {
        // If size is explicit, calculate the pixel AR, relative to the cell AR.
        const auto pixel_aspect_ratio = declared_width * cell_aspect_ratio / declared_height;
        return {declared_width, declared_height, pixel_aspect_ratio};
    }
    auto used_width = 0, used_height = 0;
    for (auto glyph : _glyphs) {
        used_width = std::max(used_width, glyph._used_width);
        used_height = std::max(used_height, glyph._used_height);
    }
    const auto in_range = [=](const auto cell_width, const auto cell_height) {
        const auto sixel_height = (cell_height + 5) / 6 * 6;
        const auto height_in_range = declared_height ? declared_height <= cell_height : used_height <= sixel_height;
        const auto width_in_range = declared_width ? declared_width <= cell_width : used_width <= cell_width;
        return height_in_range && width_in_range;
    };
    const auto unspecified_size = declared_width == 0 && declared_height == 0;
    if (cpp == 80) {
        if (in_range(8, 10) && unspecified_size)
            return {8, 10, 200};  // VT2xx, 2:1 pixel AR
        else if (in_range(15, 12))
            return {text_adjust(15), 12, 250};  // VT320, 2.5:1 pixel AR
        else if (in_range(10, 16))
            return {text_adjust(10), 16, 125};  // VT420 & VT5xx, 1.25:1 pixel AR
        else if (in_range(10, 20))
            return {text_adjust(10), 20, 100};  // VT340, 1:1 pixel AR
        else if (in_range(12, 30))
            return {text_adjust(12), 30, 80};  // VT382, 0.8:1 pixel AR
        else
            return {text_adjust(max_width), max_height, 100};
    } else {
        if (in_range(6, 10) && unspecified_size)
            return {6, 10, 200};  // VT240, 2:1 AR
        else if (in_range(9, 12))
            return {text_adjust(9), 12, 250};  // VT320, 2.5:1 pixel AR
        else if (in_range(6, 16))
            return {text_adjust(6), 16, 125};  // VT420 & VT5xx, 1.25:1 pixel AR
        else if (in_range(6, 20))
            return {text_adjust(6), 20, 100};  // VT340, 1:1 pixel AR
        else if (in_range(7, 30))
            return {text_adjust(7), 30, 80};  // VT382, 0.8:1 pixel AR
        else
            return {text_adjust(max_width), max_height, 100};
    }
}

glyph_manager::parameters::parameters(const std::string_view str)
    : _str{str}
{
    auto value = std::optional<int>{};
    for (auto ch : str) {
        if (ch >= '0' && ch <= '9')
            value = value.value_or(0) * 10 + (ch - '0');
        else if (ch == ';') {
            _values.push_back(value);
            value = {};
        }
    }
    _values.push_back(value);
    _values_used = _values.size();
    while (_values.size() < 8)
        _values.push_back({});
}

glyph_manager::parameters::parameters(const std::vector<int>& values)
{
    for (auto value : values)
        _values.emplace_back(value);
    _values_used = _values.size();
    while (_values.size() < 8)
        _values.push_back({});
    _rebuild();
}

const std::string& glyph_manager::parameters::str() const
{
    return _str;
}

std::optional<int> glyph_manager::parameters::pfn() const
{
    return _values[0];
}

void glyph_manager::parameters::pfn(const std::optional<int> value)
{
    _values[0] = value;
    _rebuild();
}

std::optional<int> glyph_manager::parameters::pcn() const
{
    return _values[1];
}

void glyph_manager::parameters::pcn(const std::optional<int> value)
{
    _values[1] = value;
    _rebuild();
}

std::optional<int> glyph_manager::parameters::pe() const
{
    return _values[2];
}

void glyph_manager::parameters::pe(const std::optional<int> value)
{
    _values[2] = value;
    _rebuild();
}

std::optional<int> glyph_manager::parameters::pcmw() const
{
    return _values[3];
}

std::optional<int> glyph_manager::parameters::pss() const
{
    return _values[4];
}

std::optional<int> glyph_manager::parameters::pu() const
{
    return _values[5];
}

std::optional<int> glyph_manager::parameters::pcmh() const
{
    return _values[6];
}

std::optional<int> glyph_manager::parameters::pcss() const
{
    return _values[7];
}

void glyph_manager::parameters::_rebuild()
{
    const auto last = std::find_if(_values.rbegin(), _values.rend(), [](const auto value) {
        return value.has_value();
    });
    _values_used = std::max<int>(_values_used, std::distance(last, _values.rend()));
    auto str = std::string{};
    for (auto i = 0; i < _values_used; i++) {
        if (i > 0) str += ";";
        if (_values[i]) str += std::to_string(_values[i].value());
    }
    _str = str;
}
