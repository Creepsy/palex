#include <iostream>
#include <fstream>

#include "parser_generator/ParserRuleLexer.h"
#include "parser_generator/ParserRuleParser.h"

int main(int argc, char* argv[]) {  
    std::ifstream input("../examples/Parser.prules");

    parser_generator::ParserRuleLexer lexer(input);
    parser_generator::ParserRuleParser parser(lexer);

    std::vector<parser_generator::Production> productions = parser.parse_all_productions();

    for (const parser_generator::Production& production : productions) {
        std::cout << production.name << " =";
        for (const parser_generator::Symbol& sym : production.symbols) {
            if (sym.type == parser_generator::Symbol::SymbolType::TERMINAL) {
                std::cout << " <" << sym.identifier << ">";
            } else {
                std::cout << " " << sym.identifier;
            }
        }
        std::cout << ";" << std::endl;
    }

    return 0;
}