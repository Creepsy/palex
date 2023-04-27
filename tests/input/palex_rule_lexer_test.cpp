#include <sstream>

#include "input/PalexRuleLexer.h"

#include "../test_utils.h"

int main() {
    std::stringstream input(
        "!WSPACE = \"\\s+\";\n"
        "<7>ANOTHER_1 = \"\";\n"
        "$S = some_prod;\n"
        "some_prod = ANOTHER_1 some_prod;\n"
        "some_prod = ;\n"
    );
    input::PalexRuleLexer lexer(input);

    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::IGNORE)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::TOKEN)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EOL)

    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::PRIORITY_TAG)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::TOKEN)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EOL)

    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::ENTRY_PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EOL)

    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::TOKEN)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EOL)

    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token().type == input::Token::TokenType::EOL)
    return 0;
}