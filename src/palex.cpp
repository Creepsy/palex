#include <iostream>
#include <fstream>

#include "lexer_generator/LexerRuleLexer.h"

int main() {  
    std::ifstream input{"../examples/Lexer.lrules"};
    lexer_generator::LexerRuleLexer lexer(input);
  
    lexer_generator::Token token;

    do {
        token = lexer.next_token();
        std::cout << token << std::endl; 
    } while(token.type != lexer_generator::Token::TokenType::END_OF_FILE);

    return 0;
}