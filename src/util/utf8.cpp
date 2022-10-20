#include "utf8.h"

#include <cwctype>
#include <cassert>
#include <cstddef>
#include <array>
#include <sstream>

constexpr char32_t MASK_6_BIT = 0x3f;

constexpr size_t BYTE_SHIFT = 6;
constexpr size_t TWO_BYTES_SHIFT = 12;
constexpr size_t THREE_BYTES_SHIFT = 18;

const char32_t PADDING_TRAILING_BYTE = 0x80;
const char32_t PADDING_2_BYTE_CHAR = 0xc0;
const char32_t PADDING_3_BYTE_CHAR = 0xe0;
const char32_t PADDING_4_BYTE_CHAR = 0xf0;


char32_t utf8::get_unicode_char(std::istream& input) {
    constexpr size_t BYTE_COUNT_INFO_SHIFT = 3;
    constexpr size_t MAX_BYTE_COUNT = 4;

    // index: first 5 bytes of head_byte
    const static std::array<size_t, 32> BYTE_COUNTS = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };

    // index: utf8 byte count
    const static std::array<unsigned char, 5> HEAD_MASKS = {
        0x00, 0xff, 0x1f, 0x0f, 0x07
    };

    char head_byte = input.get();
    
    size_t byte_count = BYTE_COUNTS[(unsigned char)head_byte >> BYTE_COUNT_INFO_SHIFT];
    bool err_flag = byte_count == 0;
    
    std::array<char, 3> utf8_bytes = {};
    input.read(utf8_bytes.data(), byte_count - 1);
    err_flag |= input.eof();

    char32_t unicode = 0;

    unicode |= (head_byte & HEAD_MASKS[byte_count]) << THREE_BYTES_SHIFT;
    unicode |= (MASK_6_BIT & utf8_bytes[0]) << TWO_BYTES_SHIFT;
    unicode |= (MASK_6_BIT & utf8_bytes[1]) << BYTE_SHIFT;
    unicode |= (MASK_6_BIT & utf8_bytes[2]);
    unicode >>= (MAX_BYTE_COUNT - byte_count) * BYTE_SHIFT;

    err_flag |= byte_count > 1 && (utf8_bytes[0] & ~MASK_6_BIT) == PADDING_TRAILING_BYTE;
    err_flag |= byte_count > 2 && (utf8_bytes[1] & ~MASK_6_BIT) == PADDING_TRAILING_BYTE;
    err_flag |= byte_count > 3 && (utf8_bytes[2] & ~MASK_6_BIT) == PADDING_TRAILING_BYTE;

    return (err_flag) ? (char32_t)-1 : unicode;
}

std::u32string utf8::utf8_to_unicode(const std::string& to_convert) {
    std::stringstream input(to_convert);

    std::u32string unicode_string;

    while(input.good()) {
        char32_t next = get_unicode_char(input);

        if(next == (char32_t)-1) {
            break;
        }

        unicode_string += next;
    }

    return unicode_string;
}

std::string utf8::unicode_to_utf8(const char32_t to_convert) {
    if(to_convert > LAST_UNICODE_CHAR) {
        return "";
    }
    
    if(to_convert <= LAST_ASCII_CHAR) {
        return {(char)to_convert};
    }
    if(to_convert <= LAST_UTF8_2_BIT_CHAR) {
        return {
            (char)((to_convert >> BYTE_SHIFT) | PADDING_2_BYTE_CHAR),
            (char)((to_convert & MASK_6_BIT) | PADDING_TRAILING_BYTE)
        };
    } 
    if(to_convert <= LAST_UTF8_3_BIT_CHAR) {
        return {
            (char)((to_convert >> TWO_BYTES_SHIFT) | PADDING_3_BYTE_CHAR),
            (char)(((to_convert >> BYTE_SHIFT) & MASK_6_BIT) | PADDING_TRAILING_BYTE),
            (char)((to_convert & MASK_6_BIT) | PADDING_TRAILING_BYTE)
        };
    }
    if(to_convert <= LAST_UNICODE_CHAR) {
        return {
            (char)((to_convert >> THREE_BYTES_SHIFT) | PADDING_4_BYTE_CHAR),
            (char)(((to_convert >> TWO_BYTES_SHIFT) & MASK_6_BIT) | PADDING_TRAILING_BYTE),
            (char)(((to_convert >> BYTE_SHIFT) & MASK_6_BIT) | PADDING_TRAILING_BYTE),
            (char)((to_convert & MASK_6_BIT) | PADDING_TRAILING_BYTE)
        };
    }

    assert(false && "Unreachable code reached!");

    return "";
}

std::string utf8::unicode_to_utf8(const std::u32string& to_convert) {
    std::string utf8_string;

    for(const char32_t c : to_convert) {
        utf8_string += unicode_to_utf8(c);
    }

    return utf8_string;
}

std::istream& std::operator>>(std::istream& input, char32_t& to_read) {
    to_read = utf8::get_unicode_char(input);

    return input;
}

std::istream& std::operator>>(std::istream& input, std::u32string& to_read) {
    while(!std::iswspace(input.peek())) {
        to_read += utf8::get_unicode_char(input);
    }

    return input;
}

std::ostream& std::operator<<(std::ostream& output, const char32_t to_print) {
    return output << utf8::unicode_to_utf8(to_print);
}

std::ostream& std::operator<<(std::ostream& output, const std::u32string& to_print) {
    return output << utf8::unicode_to_utf8(to_print);
}