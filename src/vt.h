// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#pragma once

#include <array>
#include <initializer_list>
#include <iostream>
#include <string_view>

using vt_parm = int;
using vt_parms = std::initializer_list<vt_parm>;

class vt_stream {
public:
    vt_stream(std::ostream& stream);
    ~vt_stream();
    void ls0();
    void ls1();
    void ls2();
    void ls3();
    void cup(const vt_parm row = {}, const vt_parm col = {});
    void cuf(const vt_parm cols = {});
    void cub(const vt_parm cols = {});
    void ed(const vt_parm type = {});
    void il(const vt_parm count = {});
    void sgr(const vt_parms attrs = {});
    void ppa(const vt_parm page = {});
    void sm(const vt_parm mode);
    void rm(const vt_parm mode);
    void sm(const char prefix, const vt_parm mode);
    void rm(const char prefix, const vt_parm mode);
    void sm(const vt_parms modes);
    void rm(const vt_parms modes);
    void sm(const char prefix, const vt_parms modes);
    void rm(const char prefix, const vt_parms modes);
    void dsr(const vt_parm id);
    void dsr(const char prefix, const vt_parm id);
    void scs(const int gset, const std::string_view id);
    void scs96(const int gset, const std::string_view id);
    void da();
    void s7c1t();
    void decsc();
    void decrc();
    void decfi();
    void decic(const vt_parm count = {});
    void decsace(const vt_parm extent = {});
    void decrqm(const char prefix, const vt_parm mode);
    void decac(const vt_parm a, const vt_parm b, const vt_parm c);
    void decctr(const vt_parm type);
    void decstbm(const vt_parm top = {}, const vt_parm bottom = {});
    void decslrm(const vt_parm left = {}, const vt_parm right = {});
    void decfra(const vt_parm ch, const vt_parm top, const vt_parm left, const vt_parm bottom, const vt_parm right);
    void deccra(const vt_parm top, const vt_parm left, const vt_parm bottom, const vt_parm right, const vt_parm dtop, const vt_parm dleft);
    void deccra(const vt_parm top, const vt_parm left, const vt_parm bottom, const vt_parm right, const vt_parm page, const vt_parm dtop, const vt_parm dleft, const vt_parm dpage);
    void deccara(const vt_parm top, const vt_parm left, const vt_parm bottom, const vt_parm right, const vt_parms attrs);
    void decdmac(const vt_parm id, const vt_parm dt, const vt_parm encoding, const std::string_view data = "");
    void decinvm(const vt_parm id);
    void decswt(const std::string_view s = {});
    void dcs(const std::string_view s);
    void write(const std::string_view s);
    void write(const char ch);
    void write(const std::wstring_view s);
    void write(const wchar_t ch);
    void write_spaces(const int count);
    void flush();

private:
    void _csi();
    void _final(const std::string_view chars);
    void _parm(const vt_parm value);
    void _parms(const vt_parms parms, const bool compact = true);
    void _number(const int n);
    void _string(const std::string_view s);
    void _char(const char ch);

    std::ostream& _stream;
    std::array<char, 8192> _buffer = {};
    int _buffer_index = 0;
};

extern vt_stream vtout;
