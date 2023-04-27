#include "production_definition.h"

namespace parser_generator {
    bool Production::is_entry() const {
        return this->name == ENTRY_PRODUCTION_NAME;
    }

    bool operator<(const Symbol& first, const Symbol& second) {
        if (first.type != second.type) return first.type < second.type;
        return first.identifier < second.identifier;
    }

    bool operator<(const Production& first, const Production& second) {
        if (first.name != second.name) return first.name < second.name;
        return first.symbols < second.symbols;
    }

    bool operator==(const Symbol& first, const Symbol& second) {
        return first.type == second.type && first.identifier == second.identifier;
    }

    bool operator==(const Production& first, const Production& second) {
        return first.name == second.name && first.symbols == second.symbols;
    }

    bool operator!=(const Symbol& first, const Symbol& second) {
        return !(first == second);
    }

    bool operator!=(const Production& first, const Production& second) {
        return !(first == second);
    }

    std::ostream& operator<<(std::ostream& output, const Symbol& to_print) {
        if (to_print.type == Symbol::SymbolType::TERMINAL) {
            return output << "<" << to_print.identifier << ">";
        }
        return output << to_print.identifier;
    }

    std::ostream& operator<<(std::ostream& output, const Production& to_print) {
        output << to_print.name << " =";
        for (const Symbol& sym : to_print.symbols) {
            output << " " << sym;
        }
        return output;
    }
}