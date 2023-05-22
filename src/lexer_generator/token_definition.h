#pragma once

#include <cstddef>
#include <string>
#include <memory>

#include "regex/regex_ast.h"

namespace lexer_generator {
    struct TokenDefinition {
        bool ignore_token;
        size_t priority;

        std::string name;
        std::unique_ptr<regex::RegexBase> token_regex;
    };
}