#include "parser_state_comparators.h"

#include <algorithm>
namespace parser_generator::shift_reduce_parsers {
    // helper functions

    bool lr_state_compare(const ParserState& first, const ParserState& second) {
        return first.get_production_states() == second.get_production_states();
    }

    bool lalr_state_compare(const ParserState& first, const ParserState& second) {
        return std::equal(
            first.get_production_states().begin(),
            first.get_production_states().end(),
            second.get_production_states().begin(),
            second.get_production_states().end(),
            [](const ProductionState& first, const ProductionState& second) -> bool {
                return first.get_production() == second.get_production() && first.get_position() == second.get_position(); 
            }
        );
    }
}