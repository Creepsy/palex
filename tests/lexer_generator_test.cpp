#include <sstream>

#include "lexer_generator/LexerRuleLexer.h"

#include "TestReport.h"
#include "test_utils.h"

bool test_lexer();

int main() {
    tests::TestReport report;

    report.add_test("lexer", test_lexer);

    report.run();

    return report.report();
}

bool test_lexer() {
    std::stringstream input("$ IDENT iDEnT \"regex\" \"\\\"with quote\" = ; \"undefined\n\"");

    lexer_generator::LexerRuleLexer lexer(input);

    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IGNORE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::EQUALS)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::END_OF_LINE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::UNDEFINED)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::UNDEFINED)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::END_OF_FILE);

    return true;
}