#pragma once

#include <vector>
#include <string>

#include "lexer_generator/LexerRuleParser.h"

namespace code_gen {
    struct TokenInfos {
        std::vector<std::u32string> tokens;
        std::vector<std::u32string> ignored_tokens;
    };

    TokenInfos conv_rules_to_generation_info(const std::vector<lexer_generator::TokenRegexRule>& to_convert);
}