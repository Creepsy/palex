#include <sstream>

#include "bootstrap/BootstrapLexer.h"

#include "../test_utils.h"

int main() {
    const char* input =
        "!WSPACE = \"\\s+\";\n"
        "<7>ANOTHER_1 = \"\";\n"
        "$S = some_prod;\n"
        "some_prod = ANOTHER_1 some_prod;\n"
        "some_prod#production_tag = -> error \"invalid input\";\n"
        ;
    bootstrap::BootstrapLexer lexer(input);

    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::IGNORE)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::TOKEN)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::STRING)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EOL)

    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::PRIORITY_TAG)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::TOKEN)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::STRING)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EOL)

    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::ENTRY_PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EOL)

    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::TOKEN)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EOL)

    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::PRODUCTION)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::PRODUCTION_TAG)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EQ)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::RESULTS_IN)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::ERROR_KW)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::STRING)
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::EOL)
    
    TEST_TRUE(lexer.next_unignored_token() == bootstrap::TokenInfo::TokenType::END_OF_FILE)
    return 0;
}