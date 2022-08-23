#pragma once

#include <string>
#include <optional>

#include "LexerRuleLexer.h"

namespace lexer_generator {
    struct TokenRegexRule {
        bool ignore_token;

        std::u32string token_name;
        std::u32string token_regex;
    };

    class LexerRuleParser {
        private:
            LexerRuleLexer& input;
            Token curr;

            void expect(const Token::TokenType type);
            bool accept(const Token::TokenType type);
            void consume();
            void consume(const Token::TokenType type);

            void throw_parsing_err(const Token::TokenType expected);
        public:
            LexerRuleParser(LexerRuleLexer& input);
            std::optional<TokenRegexRule> parse_token_rule();
    };
}