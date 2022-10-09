#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <json.h>

#include "lexer_generator/lexer_automaton.h"

#include "code_gen_data.h"

namespace code_gen {
    namespace cpp {
        struct CppLexerConfig {
            std::string lexer_path = ".";
            std::string utf8_lib_path = ".";

            std::string lexer_name = "Lexer";
            std::string lexer_namespace = "palex";

            bool create_utf8_lib = true;
            bool token_fallback = true;
        };

        CppLexerConfig create_lexer_config_from_json(const nlohmann::json& json_config); 

        void generate_lexer_files(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config, const TokenInfos& tokens);
        void generate_lexer_header(const CppLexerConfig& config, const TokenInfos& tokens);
        void generate_lexer_source(const CppLexerConfig& config, const TokenInfos& tokens, const lexer_generator::LexerAutomaton_t& dfa);
        void generate_utf8_lib(const CppLexerConfig& config);

        void complete_lexer_header(std::ostream& output, const std::string_view tag, const CppLexerConfig& config, const TokenInfos& tokens);
        void complete_lexer_source(
            std::ostream& output, 
            const std::string_view tag, 
            const CppLexerConfig& config, 
            const TokenInfos& tokens, 
            const lexer_generator::LexerAutomaton_t& dfa
        );

        void write_state_machine(std::ostream& output, const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config);
        void write_states(std::ostream& output, const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config);
        void write_state(
            std::ostream& output, 
            const lexer_generator::LexerAutomaton_t& dfa, 
            const CppLexerConfig& config, 
            const lexer_generator::LexerAutomaton_t::StateID_t to_write
        );
    }
}