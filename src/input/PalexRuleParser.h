#pragma once

#include <optional>

#include "lexer_generator/token_definition.h"
#include "parser_generator/production_definition.h"

#include "PalexRuleLexer.h"

namespace input {
    struct PalexRules {
        std::vector<lexer_generator::TokenDefinition> token_definitions;
        std::vector<parser_generator::Production> productions;
    };

    class PalexRuleParser {
        public:
            PalexRuleParser(PalexRuleLexer& lexer);
            PalexRules parse_all_rules();
            std::vector<lexer_generator::TokenDefinition> parse_all_token_definitions();
            std::vector<parser_generator::Production> parse_all_productions();
            std::optional<lexer_generator::TokenDefinition> try_parse_token_definition();
            std::optional<parser_generator::Production> try_parse_production();
            ~PalexRuleParser();
        private:
            PalexRuleLexer& lexer;
            Token curr;

            std::vector<parser_generator::Symbol> parse_symbol_sequence();
            
            void expect(const Token::TokenType to_expect) const;
            bool accept(const Token::TokenType to_check) const;
            Token consume();
            Token consume(const Token::TokenType to_expect);
            bool consume_if(const Token::TokenType to_check);
            void throw_error(const std::string& message) const;
    };    
}