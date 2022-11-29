#include <sstream>
#include <string>

#include "parser_generator/ParserRuleLexer.h"
#include "parser_generator/ParserRuleParser.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    std::stringstream input("test = a <B>; test2 = a; test3 = b; test_err = ");

    parser_generator::ParserRuleLexer lexer(input);
    parser_generator::ParserRuleParser parser(lexer);

    TEST_TRUE(parser.parse_production().has_value())
    TEST_TRUE(parser.parse_production().has_value())
    TEST_TRUE(parser.parse_production().has_value())

    TEST_EXCEPT(parser.parse_production(), palex_except::ParserError)

    TEST_FALSE(parser.parse_production().has_value())

    return 0;
}