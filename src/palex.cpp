#include <iostream>
#include <fstream>

#include "parser_generator/ParserRuleLexer.h"
#include "parser_generator/ParserRuleParser.h"
#include "parser_generator/validation.h"

int main(int argc, char* argv[]) {  
    std::ifstream input("../examples/Parser.prules");

    parser_generator::ParserRuleLexer lexer(input);
    parser_generator::ParserRuleParser parser(lexer);

    std::vector<parser_generator::Production> productions = parser.parse_all_productions();
    parser_generator::validate_productions(productions);

    for (const parser_generator::Production& production : productions) {
        std::cout << production << ";" << std::endl;
    }

    return 0;
}