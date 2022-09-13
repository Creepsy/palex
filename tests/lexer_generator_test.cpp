#include <sstream>

#include "lexer_generator/LexerRuleLexer.h"
#include "lexer_generator/LexerRuleParser.h"

#include "util/palex_except.h"

#include "TestReport.h"
#include "test_utils.h"

bool test_rule_lexer();
bool test_rule_parser();
bool test_rule_parser_errors();

int main() {
    tests::TestReport report;

    report.add_test("rule_lexer", test_rule_lexer);
    report.add_test("rule_parser", test_rule_parser);
    report.add_test("rule_parser_errors", test_rule_parser_errors);

    report.run();

    return report.report();
}

bool test_rule_lexer() {
    std::stringstream input("$ IDENT iDEnT _iden1 \"regex\" \"\\\"with quote\" = ; \"undefined\n\"\" 6ust");

    lexer_generator::LexerRuleLexer lexer(input);

    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IGNORE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::EQUALS)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::END_OF_LINE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::UNDEFINED)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::UNDEFINED)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::END_OF_FILE);

    return true;
}

bool test_rule_parser() {
    std::stringstream input("$WSPACE=\"\\s+\"; _IDE_NT2 =     \"\\w+\";  ");
    
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);
    
    const lexer_generator::TokenRegexRule rule_wspace = parser.parse_token_rule().value();
    TEST_TRUE(rule_wspace.ignore_token)
    TEST_TRUE(rule_wspace.token_name == U"WSPACE")
    TEST_TRUE(rule_wspace.token_regex == U"\\s+")

    const lexer_generator::TokenRegexRule rule_ident = parser.parse_token_rule().value();
    TEST_FALSE(rule_ident.ignore_token)
    TEST_TRUE(rule_ident.token_name == U"_IDE_NT2")
    TEST_TRUE(rule_ident.token_regex == U"\\w+")

    TEST_FALSE(parser.parse_token_rule().has_value())
    
    return true;
}

bool test_rule_parser_errors() {
    std::stringstream input("$WSPACE = IDENT = \"aaaaa\"\n T2 =");
    
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);

    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)
    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)
    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)

    TEST_FALSE(parser.parse_token_rule().has_value())
    
    return true;
}