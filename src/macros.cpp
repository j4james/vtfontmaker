// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "macros.h"

#include "vt.h"

int macro_manager::reserve_id()
{
    static int _next_id = 0;
    // Clear out all existing macros on first use.
    if (_next_id == 0) vtout.decdmac({}, 1, {});
    return _next_id++;
}

int macro_manager::create(macro_callback callback)
{
    return create(reserve_id(), callback);
}

int macro_manager::create(const int id, macro_callback callback)
{
    auto buffer = macro_buffer{};
    auto buffer_stream = std::ostream(&buffer);
    auto vt_stream = macro_stream{buffer, buffer_stream};
    callback(vt_stream);
    vt_stream.flush();
    vtout.decdmac(id, {}, 1, buffer.encoded());
    return id;
}

int macro_buffer::sync()
{
    const auto& text = str();
    _encoded.reserve(_encoded.size() + text.length() * 2);
    for (auto i = 0, offset = 0; i < text.length(); i++) {
        static constexpr auto hex = "0123456789ABCDEF";
        _encoded += hex[(text[i] >> 4) & 0x0F];
        _encoded += hex[text[i] & 0x0F];
    }
    str("");
    return 0;
}

const std::string& macro_buffer::encoded() const
{
    return _encoded;
}

std::string& macro_buffer::encoded()
{
    return _encoded;
}

macro_stream::macro_stream(macro_buffer& buffer, std::ostream& buffer_stream)
    : vt_stream{buffer_stream}, _buffer{buffer}
{
}

void macro_stream::repeat(const int count, macro_callback callback)
{
    auto& encoded = _buffer.encoded();
    flush();
    encoded.push_back('!');
    encoded.append(std::to_string(count));
    encoded.push_back(';');
    callback(*this);
    flush();
    encoded.push_back(';');
}
