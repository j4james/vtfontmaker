// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "vt.h"

#include "iso2022.h"

vt_stream vtout{std::cout};

vt_stream::vt_stream(std::ostream& stream)
    : _stream{stream}
{
}

vt_stream::~vt_stream()
{
    flush();
}

void vt_stream::ls0()
{
    _char('\017');
}

void vt_stream::ls1()
{
    _char('\016');
}

void vt_stream::ls2()
{
    _string("\033n");
}

void vt_stream::ls3()
{
    _string("\033o");
}

void vt_stream::cup(const vt_parm row, const vt_parm col)
{
    _csi();
    _parms({row, col});
    _final("H");
}

void vt_stream::cuf(const vt_parm cols)
{
    _csi();
    _parm(cols);
    _final("C");
}

void vt_stream::cub(const vt_parm cols)
{
    _csi();
    _parm(cols);
    _final("D");
}

void vt_stream::ed(const vt_parm type)
{
    _csi();
    _parm(type);
    _final("J");
}

void vt_stream::il(const vt_parm count)
{
    _csi();
    _parm(count);
    _final("L");
}

void vt_stream::ppa(const vt_parm page)
{
    _csi();
    _parm(page);
    _final(" P");
}

void vt_stream::sgr(const vt_parms attrs)
{
    _csi();
    _parms(attrs);
    _final("m");
}

void vt_stream::sm(const vt_parm mode)
{
    sm({mode});
}

void vt_stream::rm(const vt_parm mode)
{
    rm({mode});
}

void vt_stream::sm(const char prefix, const vt_parm mode)
{
    sm(prefix, {mode});
}

void vt_stream::rm(const char prefix, const vt_parm mode)
{
    rm(prefix, {mode});
}

void vt_stream::sm(const vt_parms modes)
{
    _csi();
    _parms(modes);
    _final("h");
}

void vt_stream::rm(const vt_parms modes)
{
    _csi();
    _parms(modes);
    _final("l");
}

void vt_stream::sm(const char prefix, const vt_parms modes)
{
    _csi();
    _char(prefix);
    _parms(modes);
    _final("h");
}

void vt_stream::rm(const char prefix, const vt_parms modes)
{
    _csi();
    _char(prefix);
    _parms(modes);
    _final("l");
}

void vt_stream::dsr(const vt_parm id)
{
    _csi();
    _parm(id);
    _final("n");
}

void vt_stream::dsr(const char prefix, const vt_parm id)
{
    _csi();
    _char(prefix);
    _parm(id);
    _final("n");
}

void vt_stream::scs(const int gset, const std::string_view id)
{
    _char('\033');
    _char("()*+"[gset]);
    _string(id);
}

void vt_stream::scs96(const int gset, const std::string_view id)
{
    _char('\033');
    _char(",-./"[gset]);
    _string(id);
}

void vt_stream::da()
{
    _string("\033[c");
}

void vt_stream::s7c1t()
{
    _string("\033 F");
}

void vt_stream::decsc()
{
    _string("\0337");
}

void vt_stream::decrc()
{
    _string("\0338");
}

void vt_stream::decfi()
{
    _string("\0339");
}

void vt_stream::decic(const vt_parm count)
{
    _csi();
    _parm(count);
    _final("'}");
}

void vt_stream::decsace(const vt_parm extent)
{
    _csi();
    _parm(extent);
    _final("*x");
}

void vt_stream::decrqm(const char prefix, const vt_parm mode)
{
    _csi();
    _char(prefix);
    _parm(mode);
    _final("$p");
}

void vt_stream::decac(const vt_parm a, const vt_parm b, const vt_parm c)
{
    _csi();
    _parms({a, b, c});
    _final(",|");
}

void vt_stream::decctr(const vt_parm type)
{
    _csi();
    _parms({2, type});
    _final("$u");
}

void vt_stream::decstbm(const vt_parm top, const vt_parm bottom)
{
    _csi();
    _parms({top, bottom});
    _final("r");
}

void vt_stream::decslrm(const vt_parm left, const vt_parm right)
{
    _csi();
    _parms({left, right});
    _final("s");
}

void vt_stream::decfra(const vt_parm ch, const vt_parm top, const vt_parm left, const vt_parm bottom, const vt_parm right)
{
    _csi();
    _parms({ch, top, left, bottom, right});
    _final("$x");
}

void vt_stream::deccra(const vt_parm top, const vt_parm left, const vt_parm bottom, const vt_parm right, const vt_parm dtop, const vt_parm dleft)
{
    _csi();
    _parms({top, left, bottom, right, {}, dtop, dleft, {}});
    _final("$v");
}

void vt_stream::deccra(const vt_parm top, const vt_parm left, const vt_parm bottom, const vt_parm right, const vt_parm page, const vt_parm dtop, const vt_parm dleft, const vt_parm dpage)
{
    _csi();
    _parms({top, left, bottom, right, page, dtop, dleft, dpage});
    _final("$v");
}

void vt_stream::deccara(const vt_parm top, const vt_parm left, const vt_parm bottom, const vt_parm right, const vt_parms attrs)
{
    _csi();
    _parms({top, left, bottom, right}, false);
    _char(';');
    _parms(attrs);
    _final("$r");
}

void vt_stream::decdmac(const vt_parm id, const vt_parm dt, const vt_parm encoding, const std::string_view data)
{
    _string("\033P");
    _parms({id, dt, encoding});
    _string("!z");
    _string(data);
    _string("\033\\");
}

void vt_stream::decinvm(const vt_parm id)
{
    _csi();
    _parm(id);
    _final("*z");
}

void vt_stream::decswt(const std::string_view s)
{
    _string("\033]21;");
    write(s);
    _string("\033\\");
}

void vt_stream::dcs(const std::string_view s)
{
    _string("\033P");
    _string(s);
    _string("\033\\");
}

void vt_stream::write(const std::string_view s)
{
    _string(s);
}

void vt_stream::write(const char ch)
{
    _char(ch);
}

void vt_stream::write(const std::wstring_view s)
{
    iso2022{s}.write(*this);
}

void vt_stream::write(const wchar_t ch)
{
    iso2022{ch}.write(*this);
}

void vt_stream::write_spaces(const int count)
{
    for (auto i = 0; i < count; i++)
        _char(' ');
}

void vt_stream::flush()
{
    if (_buffer_index) {
        _stream.write(&_buffer[0], _buffer_index);
        _stream.flush();
        _buffer_index = 0;
    }
}

void vt_stream::_csi()
{
    _string("\033[");
}

void vt_stream::_final(const std::string_view chars)
{
    _string(chars);
}

void vt_stream::_parm(const vt_parm value)
{
    if (value) _number(value);
}

void vt_stream::_parms(const vt_parms parms, const bool compact)
{
    auto last_good = -1;
    auto i = 0;
    for (const auto parm : parms) {
        if (parm) last_good = i;
        i++;
    }
    i = 0;
    for (const auto parm : parms) {
        if (i > last_good && compact) break;
        if (i > 0) _char(';');
        if (parm) _number(parm);
        i++;
    }
}

void vt_stream::_number(const int n)
{
    if (n >= 10) _number(n / 10);
    _char('0' + (n % 10));
}

void vt_stream::_string(const std::string_view s)
{
    for (auto ch : s)
        _char(ch);
}

void vt_stream::_char(const char ch)
{
    _buffer[_buffer_index++] = ch;
    if (_buffer_index == _buffer.size())
        flush();
}
