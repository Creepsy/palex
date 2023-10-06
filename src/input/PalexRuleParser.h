#pragma once

#include <optional>
#include <functional>

#include "lexer_generator/token_definition.h"
#include "parser_generator/production_definition.h"

#include "bootstrap/TokenInfo.h"

namespace input {
    struct PalexRules {
        std::vector<lexer_generator::TokenDefinition> token_definitions;
        std::vector<parser_generator::Production> productions;
    };

    class PalexRuleParser {
        public:
            using NextTokenFunc_t = std::function<bootstrap::TokenInfo::TokenType()>;
            using CurrTokenFunc_t = std::function<bootstrap::TokenInfo()>;

            PalexRuleParser(NextTokenFunc_t next_token, CurrTokenFunc_t curr_token);
            PalexRules parse_all_rules();
            std::vector<lexer_generator::TokenDefinition> parse_all_token_definitions();
            std::vector<parser_generator::Production> parse_all_productions();
            std::optional<lexer_generator::TokenDefinition> try_parse_token_definition();
            std::optional<parser_generator::Production> try_parse_production();
            ~PalexRuleParser();
        private:
            NextTokenFunc_t next_token;
            CurrTokenFunc_t curr_token;

            std::vector<parser_generator::Symbol> parse_symbol_sequence();
            parser_generator::ProductionResult_t parse_production_result();
            
            void expect(const bootstrap::TokenInfo::TokenType to_expect) const;
            bool accept(const bootstrap::TokenInfo::TokenType to_check) const;
            bootstrap::TokenInfo consume();
            bootstrap::TokenInfo consume(const bootstrap::TokenInfo::TokenType to_expect);
            bool consume_if(const bootstrap::TokenInfo::TokenType to_check);
            void throw_error(const std::string& message) const;
    };    
}