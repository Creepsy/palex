#include <cassert>
#include <functional>
#include <vector>
#include <set>

#include "bootstrap/BootstrapLexer.h"

#include "input/PalexRuleParser.h"
#include "parser_generator/validation.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"
#include "parser_generator/shift_reduce_parsers/parser_state.h"
#include "parser_generator/shift_reduce_parsers/state_lookahead.h"

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
    const char* input =
        "number = INT;"
        "multiplication = multiplication MUL number;"
        "multiplication = number;"
        "addition = addition ADD multiplication;"
        "addition = multiplication;"
        "$S = addition;"
    ;
    bootstrap::BootstrapLexer lexer(input);
    input::PalexRuleParser parser(
        std::bind(&bootstrap::BootstrapLexer::next_unignored_token, &lexer),
        std::bind(&bootstrap::BootstrapLexer::get_token, &lexer)
    );  
    std::vector<parser_generator::Production> productions = parser.parse_all_productions();
    parser_generator::validate_productions(productions);
    std::set<parser_generator::Production> productions_set(productions.begin(), productions.end());

    const parser_generator::shift_reduce_parsers::FirstSet_t first_set = parser_generator::shift_reduce_parsers::generate_first_set(productions_set, 3);
    TEST_TRUE(first_set == FIRST_SET_3);
    return 0;
}