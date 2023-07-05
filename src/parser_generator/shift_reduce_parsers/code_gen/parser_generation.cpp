#include "parser_generation.h"

#include <iostream>
#include <cassert>

#include "util/palex_except.h"

#include "parser_generator/validation.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"
#include "parser_generator/shift_reduce_parsers/parser_state_comparators.h"
#include "parser_generator/shift_reduce_parsers/parser_state.h"
#include "parser_generator/shift_reduce_parsers/state_lookahead.h"

#include "cpp_code_gen.h"

namespace parser_generator::shift_reduce_parsers::code_gen {
    const std::vector<ParserCodeGenerator_t> PARSER_CODE_GENERATORS = {
        EMPTY_PARSER_GENERATOR,
        cpp::generate_parser_files
    };

    bool generate_parser(const std::string& unit_name, const std::vector<Production>& productions, const input::PalexConfig& config) {
        if (productions.empty()) {
            std::cerr << "Skipped generation of parser as no productions are given!" << std::endl;
            return false;
        }
        if (config.parser_type == input::ParserType::NONE) {
            throw palex_except::ValidationError("No parser type supplied!");
        }
        validate_productions(productions);
        const FirstSet_t first_set = generate_first_set(
            std::set<Production>(productions.begin(), productions.end()),
            config.lookahead
        );
        const ParserTable parser_table = ParserTable::generate(
            std::set<Production>(productions.begin(), productions.end()),
            PARSER_STATE_COMPARATORS[(size_t)config.parser_type],
            config.lookahead
        );
        assert(PARSER_CODE_GENERATORS.size() > (size_t)config.language && "BUG: Supplied language has no parser generator associated with it!");
        return PARSER_CODE_GENERATORS[(size_t)config.language](unit_name, productions, parser_table, config);
    }
}