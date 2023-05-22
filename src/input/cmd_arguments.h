#pragma once

#include <string>
#include <vector>

namespace input {
    enum class Language {
        NONE,
        CPP
    };

    enum class ParserType {
        NONE,
        LALR,
        LR
    };

    struct PalexConfig {
        std::vector<std::string> rule_files;
        std::string output_path = ".";
        Language language = Language::NONE;  
        ParserType parser_type = ParserType::NONE;
        size_t lookahead = 0;

        bool generate_lexer = false;
        bool generate_util = false;
        bool generate_parser = false;
        bool lexer_fallback = false;
    };

    PalexConfig parse_config_from_args(const int argc, char* argv[]);
}