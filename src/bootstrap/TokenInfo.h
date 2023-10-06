#pragma once

#include <cstddef>
#include <string_view>
#include <ostream>

#include "util/utf8.h"

namespace bootstrap {
    struct FilePosition {
        size_t line = 1;
        size_t column = 1;

        void advance(const utf8::Codepoint_t consumed);
    };
    
    struct TokenInfo {
        enum class TokenType {
            UNDEFINED,
            END_OF_FILE,
            IGNORE,
            RESULTS_IN,
            STRING,
            PRIORITY_TAG,
            ENTRY_PRODUCTION,
            PRODUCTION,
            TOKEN,
            EQ,
            EOL,
            PRODUCTION_TAG,
            ERROR_KW,
            WSPACE
        };

        TokenType type;
        std::string_view identifier;
        FilePosition begin;
        FilePosition end;
    };

    std::ostream& operator<<(std::ostream& output, const FilePosition& to_print);
    std::ostream& operator<<(std::ostream& output, const TokenInfo& to_print);
    std::ostream& operator<<(std::ostream& output, const TokenInfo::TokenType to_print);
}