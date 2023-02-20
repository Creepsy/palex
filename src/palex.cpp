#include <iostream>
#include <fstream>

#include "parser_generator/lang_recognition/ParserProductionLexer.h"
#include "parser_generator/lang_recognition/ParserProductionParser.h"
#include "parser_generator/lang_recognition/validation.h"

int main(int argc, char* argv[]) {  
    std::ifstream input("../examples/Parser.prules");

    parser_generator::ParserProductionLexer lexer(input);
    parser_generator::ParserProductionParser parser(lexer);

    std::vector<parser_generator::Production> productions = parser.parse_all_productions();
    parser_generator::validate_productions(productions);

    for (const parser_generator::Production& production : productions) {
        std::cout << production << ";" << std::endl;
    }

    return 0;
}