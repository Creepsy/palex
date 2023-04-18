#pragma once

#include <cstddef>
#include <string>
#include <istream>
#include <stack>
#include <optional>
#include <utility>

namespace parser_generator {
    struct CharacterPosition {
        size_t line = 1;
        size_t column = 1;

        void advance(const char32_t consumed);
    };

    struct Token {
        enum class TokenType {
            UNDEFINED,
            END_OF_FILE,
            PRODUCTION,
            TOKEN,
            EQ,
            EOL,
            WSPACE
        };

        TokenType type;
        std::u32string identifier;
        CharacterPosition position;

        bool is_ignored() const;
    };

    class ParserProductionLexer {
        public:
            ParserProductionLexer(std::istream& input);
            Token next_token();
            Token next_unignored_token();
            bool end() const;
        private:
            struct Fallback {
                size_t token_length;
                CharacterPosition next_token_position;
                Token::TokenType type;
            };

            Token try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start);
            char32_t get_char();

            std::istream& input;
            CharacterPosition curr_position;
            std::stack<char32_t> cache;
            std::optional<Fallback> fallback;
    };

    std::ostream& operator<<(std::ostream& output, const CharacterPosition& to_print);
    std::ostream& operator<<(std::ostream& output, const Token& to_print);
    std::ostream& operator<<(std::ostream& output, const Token::TokenType to_print);
}