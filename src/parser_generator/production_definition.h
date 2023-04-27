#pragma once

#include <vector>
#include <string>
#include <ostream>

namespace parser_generator {
    const std::string ENTRY_PRODUCTION_NAME = "$S";
    
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

        bool is_entry() const;
    };

    bool operator<(const Symbol& first, const Symbol& second);
    bool operator<(const Production& first, const Production& second);
    bool operator==(const Symbol& first, const Symbol& second);
    bool operator==(const Production& first, const Production& second);
    bool operator!=(const Symbol& first, const Symbol& second);
    bool operator!=(const Production& first, const Production& second);

    std::ostream& operator<<(std::ostream& output, const Symbol& to_print);
    std::ostream& operator<<(std::ostream& output, const Production& to_print);
}