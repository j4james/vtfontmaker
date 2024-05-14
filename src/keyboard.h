// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <optional>
#include <string>

enum class key {
    unmodified = 0,

    up,
    down,
    left,
    right,
    home,
    end,
    pgup,
    pgdn,

    pf1,
    pf2,
    pf3,
    pf4,

    f1,
    f2,
    f3,
    f4,
    f5,
    f6,
    f7,
    f8,
    f9,
    f10,
    help,

    enter,
    bksp,
    ins,
    del,
    tab,
    space = ' ',
    tilde = '~',

    a = 'a',
    b,
    c,
    d,
    e,
    f,
    g,
    h,
    i,
    j,
    k,
    l,
    m,
    n,
    o,
    p,
    q,
    r,
    s,
    t,
    u,
    v,
    w,
    x,
    y,
    z,

    alt = 0x10000,
    ctrl = 0x20000,
    shift = 0x40000,
};

constexpr key operator+(const key left, const key right)
{
    return static_cast<key>(static_cast<int>(left) + static_cast<int>(right));
}

constexpr int operator-(const key left, const key right)
{
    return static_cast<int>(left) - static_cast<int>(right);
}

class capabilities;

class keyboard {
public:
    static void initialize(const capabilities& caps);
    static key read();
    static std::optional<wchar_t> printable(const key key_press);
    static std::string to_string(const key key_press);
};
