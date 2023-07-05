#pragma once

#include <functional>
#include <vector>

#include "parser_state.h"

namespace parser_generator::shift_reduce_parsers {
    using ParserStateComparator_t = std::function<bool(const ParserState&, const ParserState&)>;

    extern const std::vector<ParserStateComparator_t> PARSER_STATE_COMPARATORS;
    inline const ParserStateComparator_t EMPTY_PARSER_STATE_COMPARATOR = [](const ParserState&, const ParserState&) -> bool {
        return false;
    };

    bool lr_state_compare(const ParserState& first, const ParserState& second);
    bool lalr_state_compare(const ParserState& first, const ParserState& second);
}