#pragma once

#include <string>
#include <vector>

#include "input/cmd_arguments.h"

#include "parser_generator/production_definition.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"

namespace parser_generator::shift_reduce_parsers::code_gen::cpp {
    bool generate_parser_files(
        const std::string& unit_name, 
        const std::vector<Production>& productions, 
        const ParserTable& parser_table,
        const input::PalexConfig& config
    );
    void generate_parser_header(
        const std::string& unit_name,
        const std::vector<Production>& productions,
        const input::PalexConfig& config
    );
    void generate_parser_source(
        const std::string& unit_name,
        const std::vector<Production>& productions,
        const ParserTable& parser_table,
        const input::PalexConfig& config
    );
    void generate_ast_builder_header(
        const std::string& unit_name,
        const std::vector<Production>& productions,
        const input::PalexConfig& config
    );
}