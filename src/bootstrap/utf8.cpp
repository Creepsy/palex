#include "utf8.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <sstream>

constexpr uint8_t TAILING_BYTE_CHECK_MASK = 0xc0;
constexpr uint8_t TAILING_BYTE_PADDING = 0x80;
constexpr uint8_t CODEPOINT_2_BYTE_PADDING = 0xc0;
constexpr uint8_t CODEPOINT_3_BYTE_PADDING = 0xe0;
constexpr uint8_t CODEPOINT_4_BYTE_PADDING = 0xf0;
constexpr uint8_t TAIL_DATA_MASK = 0x3f;

constexpr size_t BYTE_COUNTS_INDEX_SHIFT = 3;
constexpr size_t BYTE_SHIFT = 6;
constexpr size_t TWO_BYTES_SHIFT = 12;
constexpr size_t THREE_BYTES_SHIFT = 18;
constexpr size_t INVALID_BYTE_1_SHIFT = 31;
constexpr size_t INVALID_BYTE_2_SHIFT = 30;
constexpr size_t INVALID_BYTE_3_SHIFT = 29;
constexpr size_t INVALID_BYTE_4_SHIFT = 28;
constexpr size_t OVERLONG_REWIND_SHIFT = 27;

constexpr size_t MAX_BYTE_COUNT = 4;

constexpr utf8::Codepoint_t REPLACEMENT_CHAR = 0xfffd;

// index: first 5 bytes of head_byte
const std::array<std::size_t, 32> BYTE_COUNTS = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
};
// index: utf8 byte count
const std::array<unsigned char, 5> HEAD_MASKS = {
    0x00, 0xff, 0x1f, 0x0f, 0x07
};

namespace utf8 {
    const char* advance_codepoint(const char* current, Codepoint_t* advanced_codepoint) {
        const size_t byte_count = BYTE_COUNTS[*current >> BYTE_COUNTS_INDEX_SHIFT];
        if (advanced_codepoint) {
            std::array<char, 4> codepoint_data{};
            std::strncpy(codepoint_data.data(), current, byte_count);
            *advanced_codepoint = 0; // clear codepoint
            
            *advanced_codepoint |= (codepoint_data[0] & HEAD_MASKS[byte_count]) << THREE_BYTES_SHIFT;
            *advanced_codepoint |= (TAIL_DATA_MASK & codepoint_data[1]) << TWO_BYTES_SHIFT;
            *advanced_codepoint |= (TAIL_DATA_MASK & codepoint_data[2]) << BYTE_SHIFT;
            *advanced_codepoint |= (TAIL_DATA_MASK & codepoint_data[3]);
            *advanced_codepoint >>= (MAX_BYTE_COUNT - byte_count) * BYTE_SHIFT;

            *advanced_codepoint |= (byte_count == 0) << INVALID_BYTE_1_SHIFT;
            *advanced_codepoint |= (byte_count > 1 && (codepoint_data[1] & ~TAIL_DATA_MASK) == TAILING_BYTE_PADDING) << INVALID_BYTE_2_SHIFT;
            *advanced_codepoint |= (byte_count > 2 && (codepoint_data[2] & ~TAIL_DATA_MASK) == TAILING_BYTE_PADDING) << INVALID_BYTE_3_SHIFT;
            *advanced_codepoint |= (byte_count > 3 && (codepoint_data[3] & ~TAIL_DATA_MASK) == TAILING_BYTE_PADDING) << INVALID_BYTE_4_SHIFT;
        }
        return current + byte_count;
    }

    const char* rewind_codepoint(const char* current, Codepoint_t* rewound_codepoint) {
        size_t byte_count = 0;
        do {
            current--;
            byte_count++;
        } while ((*current & TAILING_BYTE_CHECK_MASK) == TAILING_BYTE_PADDING);
        if (rewound_codepoint) {
            *rewound_codepoint = get_next_codepoint(current);
            *rewound_codepoint |= (byte_count == BYTE_COUNTS[*current >> BYTE_COUNTS_INDEX_SHIFT]) << OVERLONG_REWIND_SHIFT;
        }
        return current;
    }

    Codepoint_t get_next_codepoint(const char* current) {
        Codepoint_t next = 0;
        advance_codepoint(current, &next);
        return next;
    }

    std::string codepoint_to_utf8(const Codepoint_t to_convert) {
        if (is_error(to_convert)) {
            return codepoint_to_utf8(REPLACEMENT_CHAR);
        }
        if (to_convert <= LAST_ASCII_CODEPOINT) {
            return {(char)to_convert};
        }
        if (to_convert <= LAST_2_BYTE_CODEPOINT) {
            return {
                (char)((to_convert >> BYTE_SHIFT) | CODEPOINT_2_BYTE_PADDING),
                (char)((to_convert & TAIL_DATA_MASK) | TAILING_BYTE_PADDING)
            };
        } 
        if (to_convert <= LAST_3_BYTE_CODEPOINT) {
            return {
                (char)((to_convert >> TWO_BYTES_SHIFT) | CODEPOINT_3_BYTE_PADDING),
                (char)(((to_convert >> BYTE_SHIFT) & TAIL_DATA_MASK) | TAILING_BYTE_PADDING),
                (char)((to_convert & TAIL_DATA_MASK) | TAILING_BYTE_PADDING)
            };
        }
        if (to_convert <= LAST_4_BYTE_CODEPOINT) {
            return {
                (char)((to_convert >> THREE_BYTES_SHIFT) | CODEPOINT_4_BYTE_PADDING),
                (char)(((to_convert >> TWO_BYTES_SHIFT) & TAIL_DATA_MASK) | TAILING_BYTE_PADDING),
                (char)(((to_convert >> BYTE_SHIFT) & TAIL_DATA_MASK) | TAILING_BYTE_PADDING),
                (char)((to_convert & TAIL_DATA_MASK) | TAILING_BYTE_PADDING)
            };
        }
        assert(false && "BUG: unreachable code");
        return "";
    }

    bool is_error(const Codepoint_t to_check) {
        return to_check > LAST_4_BYTE_CODEPOINT;
    }

    std::string get_error_kind(const Codepoint_t error) {
        std::stringstream err;
        if (error & INVALID_BYTE_1_SHIFT) {
            err << "The first byte of the codepoint is invalid!\n";
        }
        if (error & INVALID_BYTE_2_SHIFT) {
            err << "The second byte of the codepoint is invalid!\n";
        }
        if (error & INVALID_BYTE_3_SHIFT) {
            err << "The third byte of the codepoint is invalid!\n";
        }
        if (error & INVALID_BYTE_4_SHIFT) {
            err << "The fourth byte of the codepoint is invalid!\n";
        }
        if (error & OVERLONG_REWIND_SHIFT) {
            err << "The rewound codepoint is invalid!\n";
        }
        return err.str();
    }
}