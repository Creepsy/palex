#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include <json.h>

#include "lexer_generator/lexer_automaton.h"

#include "code_gen_data.h"

namespace code_gen {
    namespace cpp {
        struct CppLexerConfig {
            std::string output_path = ".";
            std::string lexer_name = "Lexer";
            std::string lexer_namespace = "palex";
        };

        CppLexerConfig create_lexer_config_from_json(const nlohmann::json& json_config); 

        void generate_lexer_files(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config, const TokenInfos& tokens);
        void generate_lexer_header(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config, const TokenInfos& tokens);
        void generate_lexer_source(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config);
        void generate_unicode_lib();

        void complete_lexer_header(std::ostream& output, const std::string_view tag, const CppLexerConfig& config, const TokenInfos& tokens);
        void complete_lexer_source(std::ostream& output, const std::string_view tag);
    }
}