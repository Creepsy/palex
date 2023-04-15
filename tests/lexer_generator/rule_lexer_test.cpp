#include <sstream>

#include "lexer_generator/lang/LexerRuleLexer.h"

#include "../test_utils.h"

int main() {
    std::stringstream input("$ IDENT _IDENT1 \"regex\" \"\\\"with quote\" = ; \"undefined\n\"\" ^<>6UST");

    lexer_generator::LexerRuleLexer lexer(input);

    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IGNORE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::EQUALS)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::END_OF_LINE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::UNDEFINED)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::UNDEFINED)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::ANGLE_PARENTHESIS_OPEN)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::ANGLE_PARENTHESIS_CLOSE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::INTEGER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::END_OF_FILE)

    return 0;
}