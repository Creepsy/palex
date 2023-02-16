#include "validation.h"

#include <set>
#include <string>

#include "util/palex_except.h"
#include "util/utf8.h"

void lexer_generator::validate_rules(const std::vector<TokenRegexRule>& to_validate) {
    validate_names(to_validate);
}

void lexer_generator::validate_names(const std::vector<TokenRegexRule>& to_validate) {
    std::set<std::u32string> names;

    for (const TokenRegexRule& rule : to_validate) {
        if (rule.name == U"UNDEFINED" || rule.name == U"END_OF_FILE") {
            throw palex_except::ValidationError("The token name " + utf8::unicode_to_utf8(rule.name) + " is already reserved by the generator!");
        }
        if (names.find(rule.name) != names.end()) {
            throw palex_except::ValidationError("Multiple definitions for token name " + utf8::unicode_to_utf8(rule.name) + "!");
        }

        names.insert(rule.name);
    }
}

