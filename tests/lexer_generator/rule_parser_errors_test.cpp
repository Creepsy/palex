#include <sstream>

#include "lexer_generator/lang/LexerRuleLexer.h"
#include "lexer_generator/lang/LexerRuleParser.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    std::stringstream input("$WSPACE = IDENT = \"aaaaa\"\n T2 =");
    
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);

    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)
    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)
    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)

    TEST_FALSE(parser.parse_token_rule().has_value())
    
    return 0;
}