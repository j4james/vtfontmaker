// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include "vt.h"

#include <functional>
#include <sstream>
#include <string>

class macro_stream;
using macro_callback = std::function<void(macro_stream&)>;

class macro_manager {
public:
    static int reserve_id();
    static int create(macro_callback callback);
    static int create(const int id, macro_callback callback);
};

class macro_buffer : public std::stringbuf {
public:
    virtual int sync();
    const std::string& encoded() const;
    std::string& encoded();

private:
    std::string _encoded;
};

class macro_stream : public vt_stream {
public:
    macro_stream(macro_buffer& buffer, std::ostream& buffer_stream);
    void repeat(const int count, macro_callback callback);

private:
    macro_buffer& _buffer;
};
