#include "validation.h"

#include <set>
#include <string>

#include "util/palex_except.h"

void lexer_generator::validate_rules(const std::vector<TokenDefinition>& to_validate) {
    validate_names(to_validate);
}

void lexer_generator::validate_names(const std::vector<TokenDefinition>& to_validate) {
    std::set<std::string> names;

    for (const TokenDefinition& rule : to_validate) {
        if (rule.name == "UNDEFINED" || rule.name == "END_OF_FILE") {
            throw palex_except::ValidationError("The token name " + rule.name + " is already reserved by the generator!");
        }
        if (names.find(rule.name) != names.end()) {
            throw palex_except::ValidationError("Multiple definitions for token name " + rule.name + "!");
        }

        names.insert(rule.name);
    }
}

