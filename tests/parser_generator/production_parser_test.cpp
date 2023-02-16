#include <sstream>
#include <string>

#include "parser_generator/ParserProductionLexer.h"
#include "parser_generator/ParserProductionParser.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    std::stringstream input("test = a <B>; test2 = a; test3 = b; test_err = ");

    parser_generator::ParserProductionLexer lexer(input);
    parser_generator::ParserProductionParser parser(lexer);

    TEST_TRUE(parser.parse_production().has_value())
    TEST_TRUE(parser.parse_production().has_value())
    TEST_TRUE(parser.parse_production().has_value())

    TEST_EXCEPT(parser.parse_production(), palex_except::ParserError)

    TEST_FALSE(parser.parse_production().has_value())

    return 0;
}