#pragma once

#include "LexerRuleParser.h"

namespace lexer_generator {
    void validate_rules(const std::vector<TokenRegexRule>& to_validate);
    void validate_token_names(const std::vector<TokenRegexRule>& to_validate);
}