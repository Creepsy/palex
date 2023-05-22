#pragma once

#include "token_definition.h"

namespace lexer_generator {
    void validate_rules(const std::vector<TokenDefinition>& to_validate);
    void validate_names(const std::vector<TokenDefinition>& to_validate);
}