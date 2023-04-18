#include <sstream>
#include <vector>
#include <set>

#include "util/palex_except.h"

#include "parser_generator/lang/ParserProductionLexer.h"
#include "parser_generator/lang/ParserProductionParser.h"
#include "parser_generator/lang/validation.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"
#include "parser_generator/shift_reduce_parsers/parser_tables.h"

#include "../test_utils.h"

int main() {
    std::stringstream input("$S = infinite_recursion; infinite_recursion = infinite_recursion <INT>;");
    parser_generator::ParserProductionLexer lexer(input);
    parser_generator::ParserProductionParser parser(lexer);
    std::vector<parser_generator::Production> productions = parser.parse_all_productions();
    parser_generator::validate_productions(productions);

    TEST_EXCEPT(parser_generator::shift_reduce_parsers::generate_first_set(std::set(productions.begin(), productions.end()), 1), palex_except::ValidationError);
    return 0;
}