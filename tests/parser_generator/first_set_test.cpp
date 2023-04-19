#include <cassert>
#include <sstream>
#include <vector>
#include <set>

#include "parser_generator/lang/ParserProductionLexer.h"
#include "parser_generator/lang/ParserProductionParser.h"
#include "parser_generator/lang/validation.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"
#include "parser_generator/shift_reduce_parsers/parser_state.h"

#include "../test_utils.h"

#define TERMINAL(s) parser_generator::Symbol{parser_generator::Symbol::SymbolType::TERMINAL, s}

int main() {
    using parser_generator::shift_reduce_parsers::Lookahead_t;

    const parser_generator::shift_reduce_parsers::FirstSet_t FIRST_SET_3 = {
        {"number", std::set<Lookahead_t>{Lookahead_t{TERMINAL("INT")}}},
        {
            "multiplication", 
            std::set<Lookahead_t>{
                Lookahead_t{TERMINAL("INT")},
                Lookahead_t{TERMINAL("INT"), TERMINAL("MUL"), TERMINAL("INT")}
            }
        },
        {
            "addition", 
            std::set<Lookahead_t>{
                Lookahead_t{TERMINAL("INT")},
                Lookahead_t{TERMINAL("INT"), TERMINAL("MUL"), TERMINAL("INT")},
                Lookahead_t{TERMINAL("INT"), TERMINAL("ADD"), TERMINAL("INT")}
            }
        },
        {
            "$S", 
            std::set<Lookahead_t>{
                Lookahead_t{TERMINAL("INT")},
                Lookahead_t{TERMINAL("INT"), TERMINAL("MUL"), TERMINAL("INT")},
                Lookahead_t{TERMINAL("INT"), TERMINAL("ADD"), TERMINAL("INT")}
            }
        }
    };
    std::stringstream input(
        "number = <INT>;"
        "multiplication = multiplication <MUL> number;"
        "multiplication = number;"
        "addition = addition <ADD> multiplication;"
        "addition = multiplication;"
        "$S = addition;"
    );
    parser_generator::ParserProductionLexer lexer(input);
    parser_generator::ParserProductionParser parser(lexer); 
    std::vector<parser_generator::Production> productions = parser.parse_all_productions();
    parser_generator::validate_productions(productions);
    std::set<parser_generator::Production> productions_set(productions.begin(), productions.end());

    const parser_generator::shift_reduce_parsers::FirstSet_t first_set = parser_generator::shift_reduce_parsers::generate_first_set(productions_set, 3);
    TEST_TRUE(first_set == FIRST_SET_3);
    return 0;
}