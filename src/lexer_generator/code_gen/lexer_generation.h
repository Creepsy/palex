#pragma once

#include <string>
#include <functional>
#include <map>

#include <json.h>

#include "lexer_generator/lexer_automaton.h"
#include "lexer_generator/LexerRuleParser.h"

namespace code_gen {
    using LexerCodeGenerator_t = std::function<bool (
        const std::vector<lexer_generator::TokenRegexRule>&, 
        const lexer_generator::LexerAutomaton_t&, 
        const std::string&, 
        const nlohmann::json&
    )>;

    extern const std::map<std::string, LexerCodeGenerator_t> LANGUAGE_CODE_GENERATORS; 

    bool generate_lexer(const std::string& lexer_name, const nlohmann::json& json_config, const std::string& target_language);
}