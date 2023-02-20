#pragma once

#include <vector>
#include <optional>
#include <string>
#include <ostream>

#include "util/palex_except.h"

#include "parser_generator/lang_recognition/ParserProductionLexer.h"

namespace parser_generator {
    struct Symbol {
        enum class SymbolType {
            TERMINAL,
            NONTERMINAL
        };

        SymbolType type;
        std::string identifier;
    };
    
    struct Production {
        std::string name;

        std::vector<Symbol> symbols;
    };

    class ParserProductionParser {
        public:
            ParserProductionParser(ParserProductionLexer& input);
            std::optional<Production> parse_production();
            std::vector<Production> parse_all_productions();            
        private:
            ParserProductionLexer& input;
            Token curr;

            void expect(const Token::TokenType to_expect) const;
            bool accept(const Token::TokenType to_check) const;

            Token consume();
            Token consume(const Token::TokenType to_expect);
    };    

    bool operator<(const Symbol& first, const Symbol& second);
    bool operator<(const Production& first, const Production& second);

    std::ostream& operator<<(std::ostream& output, const Symbol& to_print);
    std::ostream& operator<<(std::ostream& output, const Production& to_print);
}