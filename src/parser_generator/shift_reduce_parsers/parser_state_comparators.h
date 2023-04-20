#pragma once

#include "parser_state.h"

namespace parser_generator::shift_reduce_parsers {
    bool lr_state_compare(const ParserState& first, const ParserState& second);
}