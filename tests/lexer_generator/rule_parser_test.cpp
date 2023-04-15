#include <sstream>

#include "lexer_generator/lang/LexerRuleLexer.h"
#include "lexer_generator/lang/LexerRuleParser.h"

#include "../test_utils.h"

int main() {
    std::stringstream input("$WSPACE=\"\\s+\"; _IDE_NT2 =     \"\\w+\";  ");
    
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);
    
    const lexer_generator::TokenRegexRule rule_wspace = parser.parse_token_rule().value();
    TEST_TRUE(rule_wspace.ignore_token)
    TEST_TRUE(rule_wspace.name == U"WSPACE")
    TEST_TRUE(dynamic_cast<regex::RegexQuantifier*>(rule_wspace.regex_ast.get()));

    const lexer_generator::TokenRegexRule rule_ident = parser.parse_token_rule().value();
    TEST_FALSE(rule_ident.ignore_token)
    TEST_TRUE(rule_ident.name == U"_IDE_NT2")
    TEST_TRUE(dynamic_cast<regex::RegexQuantifier*>(rule_ident.regex_ast.get()));

    TEST_FALSE(parser.parse_token_rule().has_value())

    return 0;
}