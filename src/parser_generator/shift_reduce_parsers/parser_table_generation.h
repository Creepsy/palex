#pragma once

#include <set>
#include <cstddef>
#include <string>
#include <map>

#include "parser_state.h"

namespace parser_generator::shift_reduce_parsers {
    using FirstSet_t = std::map<std::string, std::set<Lookahead_t>>;
    
    template<bool(*state_compare_T)(const ParserState&, const ParserState&)>
    class ParserTable {
        public:
            ParserTable();
            ~ParserTable();
        private:
            std::set<ParserState> states;

            friend std::ostream& operator<<(std::ostream& output, const ParserTable<state_compare_T>& to_print) {
                for (auto state_iter = to_print.states.begin(); state_iter != to_print.states.end(); state_iter++) {
                    output << *state_iter;
                    if (std::distance(state_iter, to_print.states.end()) != 1) {
                        output << "\n";
                    }
                }
                return output;
            }
    };

    FirstSet_t generate_first_set(const std::set<Production>& productions, const size_t lookahead);
    std::set<Lookahead_t> follow_terminals(const Symbol& to_check, const ParserState& current_state, const FirstSet_t& first_set, const size_t lookahead);
}