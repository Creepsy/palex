#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <cstdint>

namespace encoding {
    constexpr char32_t LAST_UNICODE_CHAR = 0x10ffff;
    char32_t get_utf8(std::istream& input);
}

namespace std {
    std::ostream& operator<<(std::ostream& output, const std::u32string& to_print);
    std::ostream& operator<<(std::ostream& output, const char32_t to_print);
}