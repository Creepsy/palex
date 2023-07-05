#pragma once

#include <vector>
#include <string>
#include <functional>

#include "parser_generator/production_definition.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"

#include "input/cmd_arguments.h"

namespace parser_generator::shift_reduce_parsers::code_gen {
    using ParserCodeGenerator_t = std::function<bool(const std::string&, const std::vector<Production>&, const ParserTable&, const input::PalexConfig&)>;

    extern const std::vector<ParserCodeGenerator_t> PARSER_CODE_GENERATORS;
    inline const ParserCodeGenerator_t EMPTY_PARSER_GENERATOR = [](
        const std::string&, 
        const std::vector<Production>&, 
        const ParserTable&, 
        const input::PalexConfig&
    ) -> bool {
        return false;
    };

    bool generate_parser(const std::string& unit_name, const std::vector<Production>& productions, const input::PalexConfig& config);
}