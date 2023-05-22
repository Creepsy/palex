#pragma once

#include <string>
#include <functional>
#include <vector>

#include "input/cmd_arguments.h"

#include "lexer_generator/lexer_automaton.h"
#include "lexer_generator/token_definition.h"

namespace code_gen {
    using LexerCodeGenerator_t = std::function<bool (
        const std::vector<lexer_generator::TokenDefinition>&, 
        const lexer_generator::LexerAutomaton_t&, 
        const std::string&,
        const input::PalexConfig&
    )>;

    extern const std::vector<LexerCodeGenerator_t> LANGUAGE_CODE_GENERATORS; 
    inline const LexerCodeGenerator_t EMPTY_LEXER_GENERATOR = [](
        const std::vector<lexer_generator::TokenDefinition>&, 
        const lexer_generator::LexerAutomaton_t&, 
        const std::string&,
        const input::PalexConfig&
    ) {
        return false;
    };

    bool generate_lexer(const std::string& lexer_name, const std::vector<lexer_generator::TokenDefinition>& token_definitions, const input::PalexConfig& config);
}