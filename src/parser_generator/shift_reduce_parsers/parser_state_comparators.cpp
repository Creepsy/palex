#include "parser_state_comparators.h"

namespace parser_generator::shift_reduce_parsers {
    bool lalr_state_compare(const ParserState& first, const ParserState& second) {
        return first < second;
    }
}