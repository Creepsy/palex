#include <functional>
#include <cstddef>
#include <stdexcept>

#include "bootstrap/BootstrapLexer.h"

#include "input/PalexRuleParser.h"

#include "parser_generator/validation.h"
#include "parser_generator/production_definition.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"
#include "parser_generator/shift_reduce_parsers/state_lookahead.h"
#include "parser_generator/shift_reduce_parsers/parser_state_comparators.h"

#include "../test_utils.h"

int main() {
    constexpr size_t LOOKAHEAD = 1;
    const char* input =
        "$S = s;\n"
        "s = A e C;\n"
        "s = A f D;\n"
        "s = B f C;\n"
        "s = B e D;\n"
        "e = E;\n"
        "f = E;\n"
    ;
    bootstrap::BootstrapLexer lexer(input);
    input::PalexRuleParser parser(
        std::bind(&bootstrap::BootstrapLexer::next_unignored_token, &lexer),
        std::bind(&bootstrap::BootstrapLexer::get_token, &lexer)
    ); 
    const std::vector<parser_generator::Production> productions = parser.parse_all_rules().productions;
    parser_generator::validate_productions(productions);
    const std::set<parser_generator::Production> productions_set(productions.begin(), productions.end());
    parser_generator::shift_reduce_parsers::FirstSet_t first_set = parser_generator::shift_reduce_parsers::generate_first_set(
        productions_set,
        LOOKAHEAD
    );
    TEST_EXCEPT(
        parser_generator::shift_reduce_parsers::ParserTable::generate(
            productions_set,
            parser_generator::shift_reduce_parsers::lalr_state_compare,
            LOOKAHEAD
        ),
        std::runtime_error
    );
    return 0;
}