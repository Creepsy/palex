#include "validation.h"

#include <cassert>
#include <sstream>
#include <string>
#include <utility>

#include "util/palex_except.h"

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

// helper functions
std::set<std::string> collect_production_names(const std::vector<parser_generator::Production>& productions);
std::set<TaggedProductionName> collect_tagged_productions(const std::vector<parser_generator::Production>& productions);

std::set<std::string> collect_production_names(const std::vector<parser_generator::Production>& productions) {
    std::set<std::string> production_names;
    for (const parser_generator::Production& production : productions) {
        production_names.insert(production.name);
    }
    return production_names;
}

std::set<TaggedProductionName> collect_tagged_productions(const std::vector<parser_generator::Production>& productions) {
    std::set<TaggedProductionName> tagged_production_names;
    for (const parser_generator::Production& production : productions) {
        tagged_production_names.insert(TaggedProductionName{production.name, production.tag});
    }
    return tagged_production_names;
}

void parser_generator::validate_productions(const std::vector<Production>& to_check) {
    check_for_missing_productions(to_check);
    check_for_duplicate_productions(to_check);
    check_for_entry(to_check);
    check_for_tag_conflicts(to_check);
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

void parser_generator::check_for_tag_conflicts(const std::vector<Production>& to_check) {
    const std::set<TaggedProductionName> tagged_production_names = collect_tagged_productions(to_check);
    for (const Production& production : to_check) {
        const auto match = tagged_production_names.find(TaggedProductionName{production.name, production.tag});
        if (match != tagged_production_names.end() && match->production_name != production.name) {
            throw palex_except::ValidationError(
                "The productions '" + match->production_name + "' and '" + production.name + "lead to a conflict because the have the same tagged name!"
            );
        }
    }
}