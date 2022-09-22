#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <cstdint>

namespace unicode {
    constexpr char32_t LAST_UNICODE_CHAR = 0x10ffff;
    char32_t get_utf8(std::istream& input);
    std::string to_utf8(const char32_t unicode);
    std::string to_utf8(const std::u32string& to_convert);
}

namespace std {
    std::ostream& operator<<(std::ostream& output, const std::u32string& to_print);
    std::ostream& operator<<(std::ostream& output, const char32_t to_print);
}