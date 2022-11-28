#include <iostream>
#include <fstream>

#include "parser_generator/ParserRuleLexer.h"

int main(int argc, char* argv[]) {  
    std::ifstream input("../examples/Parser.prules");

    parser_generator::ParserRuleLexer lexer(input);

    parser_generator::Token token = lexer.next_unignored_token();
    while(!lexer.end()) {
        std::cout << token << std::endl;
        token = lexer.next_unignored_token();
    }

    return 0;
}