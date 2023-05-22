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
        // struct input::PalexConfig {
        //     std::string lexer_path = ".";
        //     std::string utf8_lib_path = ".";

        //     std::string lexer_name = "Lexer";
        //     std::string lexer_namespace = "palex";

        //     bool create_utf8_lib = true;
        //     bool token_fallback = true;
        // };

        bool generate_cpp_lexer_files(
            const std::vector<lexer_generator::TokenDefinition>& lexer_rules,
            const lexer_generator::LexerAutomaton_t& lexer_dfa, 
            const std::string& lexer_name, 
            const input::PalexConfig& config
        );

        // input::PalexConfig create_lexer_config_from_json(const std::string& lexer_name, const input::PalexConfig& config); 

        void generate_lexer_files(
            const lexer_generator::LexerAutomaton_t& dfa, 
            const std::string& lexer_name, 
            const input::PalexConfig& config, 
            const TokenInfos& tokens
        );
        void generate_lexer_header(const std::string& lexer_name, const input::PalexConfig& config, const TokenInfos& tokens);
        void generate_lexer_source(
            const std::string& lexer_name, 
            const input::PalexConfig& config, 
            const TokenInfos& tokens, 
            const lexer_generator::LexerAutomaton_t& dfa
        );
        void generate_utf8_lib(const input::PalexConfig& config);

        void complete_lexer_header(
            std::ostream& output, 
            const std::string_view tag, 
            const std::string& lexer_name, 
            const input::PalexConfig& config, 
            const TokenInfos& tokens
        );
        void complete_lexer_source(
            std::ostream& output, 
            const std::string_view tag, 
            const std::string& lexer_name,
            const input::PalexConfig& config, 
            const TokenInfos& tokens, 
            const lexer_generator::LexerAutomaton_t& dfa
        );

        void write_state_machine(std::ostream& output, const lexer_generator::LexerAutomaton_t& dfa, const input::PalexConfig& config);
        void write_state(
            std::ostream& output, 
            const lexer_generator::LexerAutomaton_t& dfa, 
            const input::PalexConfig& config, 
            const lexer_generator::LexerAutomaton_t::StateID_t to_write
        );
        void write_error_state(std::ostream& output, const input::PalexConfig& config);
        void write_state_transition_table(
            std::ostream& output, 
            const lexer_generator::LexerAutomaton_t& dfa, 
            const lexer_generator::LexerAutomaton_t::StateID_t to_write
        );
    }
}