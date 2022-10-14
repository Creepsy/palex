#pragma once

#include <string>
#include <iostream>

namespace utf8 {
    constexpr char32_t LAST_ASCII_CHAR = 0x7f;
    constexpr char32_t LAST_UTF8_2_BIT_CHAR = 0x7ff;
    constexpr char32_t LAST_UTF8_3_BIT_CHAR = 0xffff;
    constexpr char32_t LAST_UNICODE_CHAR = 0x10ffff;

    char32_t get_unicode_char(std::istream& input);

    std::u32string utf8_to_unicode(const std::string& to_convert);

    std::string unicode_to_utf8(const char32_t to_convert);
    std::string unicode_to_utf8(const std::u32string& to_convert);
}

namespace std {
    std::istream& operator>>(std::istream& input, char32_t& to_read);
    std::istream& operator>>(std::istream& input, std::u32string& to_read);

    std::ostream& operator<<(std::ostream& output, const char32_t to_print);
    std::ostream& operator<<(std::ostream& output, const std::u32string& to_print);
}