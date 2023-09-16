#include <string_view>
#include <iostream>

#include <IntIdentTestLexer.h>

int main() {
    const std::string_view input = "1000 anIdent another identifier 2000 \n 1999 number 99";
    palex::IntIdentTestLexer lexer(input);
    do {
        lexer.next_unignored_token();
        std::cout << lexer.current_token() << std::endl;
    } while (lexer.current_token().type != palex::IntIdentTestToken::TokenType::END_OF_FILE);
    return 0;
}