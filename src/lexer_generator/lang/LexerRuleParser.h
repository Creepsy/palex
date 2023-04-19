#pragma once

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <cstddef>

#include "regex/regex_ast.h"

#include "LexerRuleLexer.h"

namespace lexer_generator {
    struct TokenRegexRule {
        bool ignore_token;
        size_t priority;

        std::u32string name;
        std::unique_ptr<regex::RegexBase> regex_ast;
    };

    class LexerRuleParser {
        public:
            LexerRuleParser(LexerRuleLexer& input);
            std::optional<TokenRegexRule> parse_token_rule();
            std::vector<TokenRegexRule> parse_all_rules();
        private:
            LexerRuleLexer& input;
            Token curr;

            void expect(const Token::TokenType type);
            bool accept(const Token::TokenType type) const;
            Token consume();
            Token consume(const Token::TokenType type);

            void throw_parsing_err(const Token::TokenType expected) const;

    };
}