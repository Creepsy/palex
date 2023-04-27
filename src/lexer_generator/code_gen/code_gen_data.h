#pragma once

#include <vector>
#include <string>

#include "lexer_generator/token_definition.h"

namespace code_gen {
    struct TokenInfos {
        std::vector<std::string> tokens;
        std::vector<std::string> ignored_tokens;
    };

    TokenInfos conv_rules_to_generation_info(const std::vector<lexer_generator::TokenDefinition>& to_convert);
}