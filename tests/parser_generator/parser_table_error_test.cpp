#include <sstream>
#include <cstddef>
#include <stdexcept>

#include "input/PalexRuleLexer.h"
#include "input/PalexRuleParser.h"

#include "parser_generator/validation.h"
#include "parser_generator/production_definition.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"
#include "parser_generator/shift_reduce_parsers/state_lookahead.h"
#include "parser_generator/shift_reduce_parsers/parser_state_comparators.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    using parser_generator::shift_reduce_parsers::DebugParseTree;

    constexpr size_t LOOKAHEAD = 1;
    std::stringstream input {
        "$S = addition;\n"
        "addition = addition ADD multiplication;\n"
        "addition = multiplication;\n"
        "multiplication = multiplication MUL number;\n"
        "multiplication = number;\n"
        "number = INT;\n"
    };
    input::PalexRuleLexer lexer(input);
    input::PalexRuleParser parser(lexer);
    const std::vector<parser_generator::Production> productions = parser.parse_all_rules().productions;
    parser_generator::validate_productions(productions);
    const std::set<parser_generator::Production> productions_set(productions.begin(), productions.end());
    parser_generator::shift_reduce_parsers::FirstSet_t first_set = parser_generator::shift_reduce_parsers::generate_first_set(
        productions_set,
        LOOKAHEAD
    );
    parser_generator::shift_reduce_parsers::ParserTable parser_table = parser_generator::shift_reduce_parsers::ParserTable::generate(
            productions_set,
            parser_generator::shift_reduce_parsers::lalr_state_compare,
            LOOKAHEAD
    );

    TEST_EXCEPT(parser_table.debug_parse({"INT", "ADD"}), palex_except::ParserError)    
    TEST_EXCEPT(parser_table.debug_parse({"ADD"}), palex_except::ParserError)    
    TEST_EXCEPT(parser_table.debug_parse({}), palex_except::ParserError)    
    TEST_EXCEPT(parser_table.debug_parse({"INT", "ADD", "ADD", "INT"}), palex_except::ParserError)    
    TEST_EXCEPT(parser_table.debug_parse({"INT", "UNKNOWN", "INT"}), palex_except::ParserError)    
    return 0;
}