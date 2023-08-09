#pragma once
// TODO remove old utf8

#include <string>
#include <cstdint>

namespace utf8 {
    using Codepoint_t = uint32_t; 
    constexpr Codepoint_t FIRST_ASCII_CODEPOINT = 0x0;
    constexpr Codepoint_t FIRST_2_BYTE_CODEPOINT = 0x0080;
    constexpr Codepoint_t FIRST_3_BYTE_CODEPOINT = 0x0800;
    constexpr Codepoint_t FIRST_4_BYTE_CODEPOINT = 0x10000;
    constexpr Codepoint_t LAST_ASCII_CODEPOINT = 0x007f;
    constexpr Codepoint_t LAST_2_BYTE_CODEPOINT = 0x07ff;
    constexpr Codepoint_t LAST_3_BYTE_CODEPOINT = 0xffff;
    constexpr Codepoint_t LAST_4_BYTE_CODEPOINT = 0x10ffff;

    const char* advance_codepoint(const char* current, Codepoint_t* advanced_codepoint = nullptr);
    const char* rewind_codepoint(const char* current, Codepoint_t* rewound_codepoint = nullptr);
    Codepoint_t get_next_codepoint(const char* current);
    std::string codepoint_to_utf8(const Codepoint_t to_convert); 
    bool is_error(const Codepoint_t to_check);
    std::string get_error_kind(const Codepoint_t error);
}