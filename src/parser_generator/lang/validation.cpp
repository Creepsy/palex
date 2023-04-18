#include "validation.h"

#include <cassert>
#include <sstream>

#include "util/palex_except.h"

// helper functions
std::set<std::string> collect_production_names(const std::vector<parser_generator::Production>& productions);

std::set<std::string> collect_production_names(const std::vector<parser_generator::Production>& productions) {
    std::set<std::string> production_names;

    for (const parser_generator::Production& production : productions) {
        production_names.insert(production.name);
    }

    return production_names;
}



void parser_generator::validate_productions(const std::vector<Production>& to_check) {
    check_for_missing_productions(to_check);
    check_for_duplicate_productions(to_check);
    check_for_entry(to_check);
}

void parser_generator::check_for_missing_productions(const std::vector<Production>& to_check) {
    const std::set<std::string> production_names = collect_production_names(to_check);

    for (const Production& prod : to_check) {
        assert(production_names.find(prod.name) != production_names.end() && "Production name missing in collected names!");
        
        for (const Symbol& sym : prod.symbols) {
            if (sym.type == Symbol::SymbolType::NONTERMINAL && production_names.find(sym.identifier) == production_names.end()) {
                throw palex_except::ValidationError("The used production '" + sym.identifier + "' doesn't exist!");
            }
        }
    }
}

void parser_generator::check_for_duplicate_productions(const std::vector<Production>& to_check) {
    std::set<Production> encountered_productions;

    for (const Production& prod : to_check) {
        if (encountered_productions.find(prod) != encountered_productions.end()) {
            std::stringstream prod_representation;
            prod_representation << prod;
            throw palex_except::ValidationError("Multiple definitions for production '" + prod_representation.str() + "'!");
        }
        encountered_productions.insert(prod);
    }
}

void parser_generator::check_for_entry(const std::vector<Production>& to_check) {
    for (const Production& production : to_check) {
        if (production.is_entry()) {
            return;
        }
    }
    throw palex_except::ValidationError("No entry production found!");
}