#include "encoding.h"

#include <cstddef>
#include <stdexcept>
#include <iostream>

char32_t encoding::get_utf8(std::istream& input) {
    //index: first 5 bytes of head_byte
    const static size_t char_lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };

    //index: utf8 byte length
    const static unsigned char head_masks[] = {
        0x00, 0xff, 0x1f, 0x0f, 0x07
    };

    char head_byte = input.get();
    if(input.eof()) return -1;
    
    size_t length = char_lengths[(unsigned char)head_byte >> 3];
    if(length == 0) throw std::runtime_error("Invalid utf8 character!");
    
    char utf8_bytes[3] = {};
    input.read(utf8_bytes, length - 1);
        
    char32_t unicode = 0;

    unicode |= (head_byte & head_masks[length]) << 18;
    unicode |= (0x3f & utf8_bytes[0]) << 12;
    unicode |= (0x3f & utf8_bytes[1]) << 6;
    unicode |= (0x3f & utf8_bytes[2]);
    unicode >>= (4 - length) * 6;

    return unicode;
}

std::ostream& std::operator<<(std::ostream& output, const std::u32string& to_print) {
    for(const char32_t c : to_print) output << c;

    return output;
}

std::ostream& std::operator<<(std::ostream& output, const char32_t to_print) {
    if(to_print <= 0x7f) {
        output << (char)to_print;
    } else if(to_print <= 0x7ff) {
        output << (char)((to_print >> 6) | 0xc0)
               << (char)((to_print & 0x3f) | 0x80);
    } else if(to_print <= 0xffff) {
        output << (char)((to_print >> 12) | 0xe0) 
               << (char)(((to_print >> 6) & 0x3f) | 0x80) 
               << (char)((to_print & 0x3f) | 0x80);
    } else if(to_print <= 0x10ffff) {
        output << (char)((to_print >> 18) | 0xf0) 
               << (char)(((to_print >> 12) & 0x3f) | 0x80) 
               << (char)(((to_print >> 6) & 0x3f) | 0x80) 
               << (char)((to_print & 0x3f) | 0x80);
    } else {
        throw std::runtime_error("Invalid utf8 character!");
    }

    return output;
}