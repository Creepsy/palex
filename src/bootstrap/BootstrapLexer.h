#pragma once

#include <string_view>

#include "TokenInfo.h"

namespace bootstrap {
    class BootstrapLexer {
        public:
            BootstrapLexer(const std::string_view input);
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

            const std::string_view input; // TODO make changeable?
            const char* position;
            FilePosition file_position;
            TokenInfo current_token;
    };
}