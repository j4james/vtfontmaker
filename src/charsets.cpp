// VT Font Maker
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "charsets.h"

const std::array<charset, 31> charset::all = {{

    {L"Unregistered/94", " @", 94, L""},
    {L"Unregistered/96", " @", 96, L""},
    {L"ASCII", "B", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"},
    {L"Latin-1 (ISO)", "A", 96, L" ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"},
    {L"Latin-2 (ISO)", "B", 96, L" Ą˘Ł¤ĽŚ§¨ŠŞŤŹ­ŽŻ°ą˛ł´ľśˇ¸šşťź˝žżŔÁÂĂÄĹĆÇČÉĘËĚÍÎĎĐŃŇÓÔŐÖ×ŘŮÚŰÜÝŢßŕáâăäĺćçčéęëěíîďđńňóôőö÷řůúűüýţ˙"},
    {L"Greek (ISO)", "F", 96, L" ‘’£␦␦¦§¨©␦«¬­␦―°±²³΄΅Ά·ΈΉΊ»Ό½ΎΏΐΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡ␦ΣΤΥΦΧΨΩΪΫάέήίΰαβγδεζηθικλμνξοπρςστυφχψωϊϋόύώ␦"},
    {L"Hebrew (ISO)", "H", 96, L" ␦¢£¤¥¦§¨©×«¬­®¯°±²³´µ¶·¸¹÷»¼½¾␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦‗אבגדהוזחטיךכלםמןנסעףפץצקרשת␦␦‎‏␦"},
    {L"Latin-Cyrillic (ISO)", "L", 96, L" ЁЂЃЄЅІЇЈЉЊЋЌ­ЎЏАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя№ёђѓєѕіїјљњћќ§ўџ"},
    {L"Latin-5 (ISO)", "M", 96, L" ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõö÷øùúûüışÿ"},
    {L"Supplemental (DEC)", "%5", 94, L"¡¢£␦¥␦§¤©ª«␦␦␦␦°±²³␦µ¶·␦¹º»¼½␦¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ␦ÑÒÓÔÕÖŒØÙÚÛÜŸ␦ßàáâãäåæçèéêëìíîï␦ñòóôõöœøùúûüÿ␦"},
    {L"Greek (DEC)", "\"?", 94, L"¡¢£␦¥␦§¤©ª«␦␦␦␦°±²³␦µ¶·␦¹º»¼½␦¿ϊΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟ␦ΠΡΣΤΥΦΧΨΩάέήί␦όϋαβγδεζηθικλμνξο␦πρστυφχψωςύώ΄␦"},
    {L"Hebrew (DEC)", "\"4", 94, L"¡¢£␦¥␦§¤©ª«␦␦␦␦°±²³␦µ¶·␦¹º»¼½␦¿␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦אבגדהוזחטיךכלםמןנסעףפץצקרשת␦␦␦␦"},
    {L"Turkish (DEC)", "%0", 94, L"¡¢£␦¥␦§¤©ª«␦␦İ␦°±²³␦µ¶·␦¹º»¼½ı¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖŒØÙÚÛÜŸŞßàáâãäåæçèéêëìíîïğñòóôõöœøùúûüÿş"},
    {L"Cyrillic (DEC)", "&4", 94, L"␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦␦юабцдефгхийклмнопярстужвьызшэщчъЮАБЦДЕФГХИЙКЛМНОПЯРСТУЖВЬЫЗШЭЩЧ"},
    {L"Special Graphics (DEC)", "0", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^ ♦▒␉␌␍␊°±␤␋┘┐┌└┼⎺⎻─⎼⎽├┤┴┬│≤≥π≠£·"},
    {L"Technical (DEC)", ">", 94, L"⎷┌─⌠⌡│⎡⎣⎤⎦⎛⎝⎞⎠⎨⎬␦␦╲╱␦␦␦␦␦␦␦≤≠≥∫∴∝∞÷Δ∇ΦΓ∼≃Θ×Λ⇔⇒≡ΠΨ␦Σ␦␦√ΩΞΥ⊂⊃∩∪∧∨¬αβχδεφγηιθκλ␦ν∂πψρστ␦ƒωξυζ←↑→↓"},
    {L"U.K. (NRCS)", "A", 94, L"!\"£$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"},
    {L"French (NRCS)", "R", 94, L"!\"£$%&'()*+,-./0123456789:;<=>?àABCDEFGHIJKLMNOPQRSTUVWXYZ°ç§^_`abcdefghijklmnopqrstuvwxyzéùè¨"},
    {L"French Canadian (NRCS)", "9", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?àABCDEFGHIJKLMNOPQRSTUVWXYZâçêî_ôabcdefghijklmnopqrstuvwxyzéùèû"},
    {L"Norwegian/Danish (NRCS)", "`", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ^_`abcdefghijklmnopqrstuvwxyzæøå~"},
    {L"Finnish (NRCS)", "5", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÅÜ_éabcdefghijklmnopqrstuvwxyzäöåü"},
    {L"German (NRCS)", "K", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?§ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜ^_`abcdefghijklmnopqrstuvwxyzäöüß"},
    {L"Italian (NRCS)", "Y", 94, L"!\"£$%&'()*+,-./0123456789:;<=>?§ABCDEFGHIJKLMNOPQRSTUVWXYZ°çé^_ùabcdefghijklmnopqrstuvwxyzàòèì"},
    {L"Swiss (NRCS)", "=", 94, L"!\"ù$%&'()*+,-./0123456789:;<=>?àABCDEFGHIJKLMNOPQRSTUVWXYZéçêîèôabcdefghijklmnopqrstuvwxyzäöüû"},
    {L"Swedish (NRCS)", "7", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?ÉABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÅÜ_éabcdefghijklmnopqrstuvwxyzäöåü"},
    {L"Spanish (NRCS)", "Z", 94, L"!\"£$%&'()*+,-./0123456789:;<=>?§ABCDEFGHIJKLMNOPQRSTUVWXYZ¡Ñ¿^_`abcdefghijklmnopqrstuvwxyz°ñç~"},
    {L"Portuguese (NRCS)", "%6", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZÃÇÕ^_`abcdefghijklmnopqrstuvwxyzãçõ~"},
    {L"Greek (NRCS)", "\">", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?ϊΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟ␦ΠΡΣΤΥΦΧΨΩάέήί␦όϋαβγδεζηθικλμνξο␦πρστυφχψωςύώ΄␦"},
    {L"Hebrew (NRCS)", "%=", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_אבגדהוזחטיךכלםמןנסעףפץצקרשת{|}~"},
    {L"Turkish (NRCS)", "%2", 94, L"ı\"#$%ğ'()*+,-./0123456789:;<=>?İABCDEFGHIJKLMNOPQRSTUVWXYZŞÖÇÜ_Ğabcdefghijklmnopqrstuvwxyzşöçü"},
    {L"Russian (NRCS)", "&5", 94, L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ЮАБЦДЕФГХИЙКЛМНОПЯРСТУЖВЬЫЗШЭЩЧ"},

}};

charset::charset(const std::wstring_view name, const std::string_view id, const int size, const std::wstring_view glyphs)
    : _name{name}, _id{id}, _size{size}, _glyphs{glyphs}
{
}

const std::wstring& charset::name() const
{
    return _name;
}

const std::string& charset::id() const
{
    return _id;
}

const int charset::size() const
{
    return _size;
}

const std::wstring& charset::glyphs() const
{
    return _glyphs;
}

std::vector<std::wstring> charset::names()
{
    auto names = std::vector<std::wstring>{};
    for (auto& cs : all)
        names.emplace_back(cs.name());
    return names;
}

std::vector<std::wstring> charset::names_for_size(const int size)
{
    auto names = std::vector<std::wstring>{};
    for (auto& cs : all)
        if (cs.size() == size)
            names.emplace_back(cs.name());
    return names;
}

std::optional<int> charset::index_of(const std::string_view id, const int size)
{
    auto index = 0;
    for (auto& cs : all)
        if (cs.size() == size) {
            if (cs.id() == id) return index;
            index++;
        }
    return {};
}

const charset* charset::from_index(const int index, const std::optional<int> size)
{
    auto i = 0;
    for (auto& cs : all)
        if (!size.has_value() || cs.size() == size.value()) {
            if (i == index) return &cs;
            i++;
        }
    return {};
}
