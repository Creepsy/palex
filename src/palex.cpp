#include <iostream>
#include <fstream>

#include "input/PalexRuleLexer.h"
#include "input/PalexRuleParser.h"
#include "input/cmd_arguments.h"

#include "parser_generator/validation.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"
#include "parser_generator/shift_reduce_parsers/state_lookahead.h"
#include "parser_generator/shift_reduce_parsers/parser_state_comparators.h"

#include "parser_generator/shift_reduce_parsers/code_gen/parser_generation.h"

int main(int argc, char* argv[]) { 
    const input::PalexConfig config = input::parse_config_from_args(argc, (const char**)argv); 
    std::ifstream input("../examples/Example.palex");
    input::PalexRuleLexer lexer(input);
    input::PalexRuleParser parser(lexer);

    std::vector<parser_generator::Production> productions = parser.parse_all_rules().productions;
    parser_generator::validate_productions(productions);

    // parser_generator::shift_reduce_parsers::FirstSet_t first_set = parser_generator::shift_reduce_parsers::generate_first_set(
    //     std::set<parser_generator::Production>(productions.begin(), productions.end()),
    //     4
    // );
    // std::cout << parser_generator::shift_reduce_parsers::ParserTable::generate(
    //     std::set<parser_generator::Production>(productions.begin(), productions.end()),
    //     parser_generator::shift_reduce_parsers::lr_state_compare,
    //     2
    // );

    for (const parser_generator::Production& production : productions) {
        std::cout << production << ";" << std::endl;
    }
    parser_generator::shift_reduce_parsers::code_gen::generate_parser("Example", productions, config);

    return 0;
}