#pragma once

#include <string_view>
#include <cstddef>

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

    class BootstrapLexer {
        public:
            BootstrapLexer(const char* const input);
            TokenInfo::TokenType next_token();
            TokenInfo::TokenType next_unignored_token();
            const TokenInfo& get_token() const;
        private:
            bool try_ascii_constant(const char* to_try, const TokenInfo::TokenType token_type);
            void advance_codepoints(const size_t count);
            void advance_token();
            void advance_priority_token();
            void advance_regex_token();
            bool advance_while(bool (*predicate)(const utf8::Codepoint_t));

            const char* const input; // make changeable?
            const char* position;
            FilePosition file_position;
            TokenInfo current_token;
    };
}