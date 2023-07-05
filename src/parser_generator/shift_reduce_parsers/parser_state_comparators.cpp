#include "parser_state_comparators.h"

// helper functions
parser_generator::shift_reduce_parsers::ParserState reduce_to_lr_0_core(const parser_generator::shift_reduce_parsers::ParserState& to_reduce);

parser_generator::shift_reduce_parsers::ParserState reduce_to_lr_0_core(const parser_generator::shift_reduce_parsers::ParserState& to_reduce) {
    parser_generator::shift_reduce_parsers::ParserState lr_0_core;
    for (const parser_generator::shift_reduce_parsers::ProductionState& production_state : to_reduce.get_production_states()) {
        lr_0_core.add_production_state(
            parser_generator::shift_reduce_parsers::ProductionState(
                production_state.get_production(), 
                production_state.get_position(), 
                {} // No lookahead -> LR-0-State
            )
        );
    }
    return lr_0_core;
}

namespace parser_generator::shift_reduce_parsers {
    const std::vector<ParserStateComparator_t> PARSER_STATE_COMPARATORS = {
        EMPTY_PARSER_STATE_COMPARATOR,
        lalr_state_compare,
        lr_state_compare
    };

    bool lr_state_compare(const ParserState& first, const ParserState& second) {
        return first.get_production_states() == second.get_production_states(); // actions are the same when the production states are the same
    }

    bool lalr_state_compare(const ParserState& first, const ParserState& second) {
        return reduce_to_lr_0_core(first) == reduce_to_lr_0_core(second);
    }
}