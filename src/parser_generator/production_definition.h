#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <variant>
#include <optional>

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

    using ReductionResult_t = std::monostate;
    using ErrorResult_t = std::optional<std::string>; // static error message. when none is supplied, the error message is queried at runtime
    using ProductionResult_t = std::variant<ReductionResult_t, ErrorResult_t>;

    struct Production {
        std::string name;
        std::vector<Symbol> symbols;
        std::string tag;
        ProductionResult_t result;

        bool is_entry() const;
        std::string get_representation() const;
        std::optional<std::string> get_astbuilder_method_name() const;
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