#pragma once

#include <cstddef>
#include <string_view>

#include "utf8.h"

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
            REGEX,
            PRIORITY_TAG,
            ENTRY_PRODUCTION,
            PRODUCTION,
            TOKEN,
            EQ,
            EOL,
            WSPACE
            // TODO? -> outsource this into extra file later (after / with lexer rework)?
        };

        TokenType type;
        std::string_view identifier;
        FilePosition begin;
        FilePosition end;
    };
}