#include "validation.h"

#include <cassert>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <optional>
#include <map>

#include "util/palex_except.h"
#include "util/Visitor.h"

struct TaggedProductionName {
    std::string production_name;
    std::string production_tag;

    std::string to_representation() const {
        return this->production_tag.empty() ? this->production_name : this->production_name + "_" + this->production_tag; 
    }

    bool operator<(const TaggedProductionName& other) const {
        return this->to_representation() < other.to_representation();
    }
};

using MethodName_t = std::string;
using ProductionName_t = std::string;

// helper functions
std::set<ProductionName_t> collect_production_names(const std::vector<parser_generator::Production>& productions);
// it is not guaranteed that all productions are present in this lookup map in case of conflicting method names.
std::map<MethodName_t, ProductionName_t> collect_astbuilder_method_names(const std::vector<parser_generator::Production>& productions);

std::set<ProductionName_t> collect_production_names(const std::vector<parser_generator::Production>& productions) {
    std::set<ProductionName_t> production_names;
    for (const parser_generator::Production& production : productions) {
        production_names.insert(production.name);
    }
    return production_names;
}

std::map<MethodName_t, ProductionName_t> collect_astbuilder_method_names(const std::vector<parser_generator::Production>& productions) {
    std::map<MethodName_t, ProductionName_t> method_names;
    for (const parser_generator::Production& production : productions) {
        if (!production.is_entry()) {
            const std::optional<std::string> method_name = production.get_astbuilder_method_name();
            if (!method_name.has_value()) {
                continue;
            }
            method_names.insert(std::make_pair(method_name.value(), production.name));
        }
    }
    return method_names;
}

void parser_generator::validate_productions(const std::vector<Production>& to_check) {
    check_for_missing_productions(to_check);
    check_for_duplicate_productions(to_check);
    check_for_entry(to_check);
    check_for_astbuilder_method_conflicts(to_check);
}

void parser_generator::check_for_missing_productions(const std::vector<Production>& to_check) {
    const std::set<ProductionName_t> production_names = collect_production_names(to_check);

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

void parser_generator::check_for_astbuilder_method_conflicts(const std::vector<Production>& to_check) {
    const std::map<MethodName_t, ProductionName_t> method_names = collect_astbuilder_method_names(to_check);
    for (const Production& production : to_check) {
        const std::optional<std::string> method_name = production.get_astbuilder_method_name();
        if (!method_name.has_value()) {
            continue;
        }
        const auto match = method_names.find(method_name.value());
        if (match != method_names.end() && match->second != production.name) {
            throw palex_except::ValidationError(
                "The productions '" + match->second + "' and '" + production.name + 
                "lead to a conflict because they both need the method " + method_name.value() + "!"
            );
        }
    }
}