#include "../test_utils.h"

#include "parser_generator/production_definition.h"

#include "parser_generator/shift_reduce_parsers/parser_state.h"
#include "parser_generator/shift_reduce_parsers/state_lookahead.h"

#define TERMINAL(s) parser_generator::Symbol{parser_generator::Symbol::SymbolType::TERMINAL, s}
#define NONTERMINAL(s) parser_generator::Symbol{parser_generator::Symbol::SymbolType::NONTERMINAL, s}


int main() {
    using namespace parser_generator::shift_reduce_parsers;

    const parser_generator::Production prod1{
        "test_production", 
        std::vector<parser_generator::Symbol>{TERMINAL("INT"), NONTERMINAL("test_production"), TERMINAL("INT")}
    };
    const parser_generator::Production prod2{
        "test_production", 
        std::vector<parser_generator::Symbol>{TERMINAL("STRING"), NONTERMINAL("test_production")}
    };
    const parser_generator::Production prod3{
        "test_production", 
        std::vector<parser_generator::Symbol>{TERMINAL("ADD")}
    };
    const ProductionState prod1_state = ProductionState(prod1, Lookahead_t{TERMINAL("END_OF_FILE"), TERMINAL("END_OF_FILE")});
    const ProductionState prod3_state = ProductionState(prod3, Lookahead_t{TERMINAL("END_OF_FILE"), TERMINAL("END_OF_FILE")});
    const ParserState state1( 
        std::set<ProductionState>{
            prod1_state, ProductionState(prod2), ProductionState(prod3_state)
        }
    );
    const ParserState state2(
        std::set<ProductionState>{
            prod1_state.advance(), ProductionState(prod2), prod3_state
        }
    ); 
    FirstSet_t first_set = generate_first_set(
        std::set<parser_generator::Production>{prod1, prod2, prod3}, 
        2
    );

    TEST_TRUE((
        follow_terminals(TERMINAL("INT"), state1, first_set, 2) == 
        std::set<Lookahead_t>{
            Lookahead_t{TERMINAL("ADD"), TERMINAL("INT")},
            Lookahead_t{TERMINAL("STRING"), TERMINAL("ADD")},
            Lookahead_t{TERMINAL("STRING"), TERMINAL("INT")},
            Lookahead_t{TERMINAL("STRING"), TERMINAL("STRING")},
            Lookahead_t{TERMINAL("INT"), TERMINAL("ADD")},
            Lookahead_t{TERMINAL("INT"), TERMINAL("INT")},
            Lookahead_t{TERMINAL("INT"), TERMINAL("STRING")}
        }
    ));
    TEST_TRUE((
        follow_terminals(TERMINAL("ADD"), state1, first_set, 2) == 
        std::set<Lookahead_t>{
            Lookahead_t{TERMINAL("END_OF_FILE"), TERMINAL("END_OF_FILE")}
        }
    ));
    TEST_TRUE((
        follow_terminals(NONTERMINAL("test_production"), state2, first_set, 2) == 
        std::set<Lookahead_t>{
            Lookahead_t{TERMINAL("INT"), TERMINAL("END_OF_FILE")}
        }
    ));

    return 0;
}