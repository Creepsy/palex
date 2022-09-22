#include "validation.h"

#include <set>
#include <string>

#include "util/palex_except.h"
#include "util/unicode.h"

void lexer_generator::validate_rules(const std::vector<TokenRegexRule>& to_validate) {
    validate_token_names(to_validate);
}

void lexer_generator::validate_token_names(const std::vector<TokenRegexRule>& to_validate) {
    std::set<std::u32string> token_names;

    for(const TokenRegexRule& rule : to_validate) {
        if(rule.token_name == U"UNDEFINED" || rule.token_name == U"END_OF_FILE") {
            throw palex_except::ValidationError("The token name " + unicode::to_utf8(rule.token_name) + " is already reserved by the generator!");
        } else if(token_names.find(rule.token_name) != token_names.end()) {
            throw palex_except::ValidationError("Multiple definitions for token name " + unicode::to_utf8(rule.token_name) + "!");
        }

        token_names.insert(rule.token_name);
    }
}

