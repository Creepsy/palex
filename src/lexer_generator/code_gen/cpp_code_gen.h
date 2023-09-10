#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include "input/cmd_arguments.h"

#include "lexer_generator/lexer_automaton.h"
#include "lexer_generator/token_definition.h"

#include "code_gen_data.h"

namespace code_gen {
    namespace cpp {
        bool generate_lexer_files(
            const std::vector<lexer_generator::TokenDefinition>& token_definitions,
            const lexer_generator::LexerAutomaton_t& lexer_dfa, 
            const std::string& unit_name, 
            const input::PalexConfig& config
        );
        void generate_lexer_header(const std::string& unit_name, const input::PalexConfig& config, const TokenInfos& tokens);
        void generate_lexer_source(
            const TokenInfos& tokens, 
            const lexer_generator::LexerAutomaton_t& lexer_dfa,
            const std::string& unit_name, 
            const input::PalexConfig& config
        );
        void generate_token_header(const std::string& unit_name, const input::PalexConfig& config, const TokenInfos& tokens);
        void generate_token_source(const std::string& unit_name, const input::PalexConfig& config, const TokenInfos& tokens);
        void generate_utf8_lib(const input::PalexConfig& config);
    }
}