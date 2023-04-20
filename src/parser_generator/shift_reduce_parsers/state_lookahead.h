#pragma once

#include <set>
#include <cstddef>
#include <string>
#include <map>

#include "parser_state.h"

namespace parser_generator::shift_reduce_parsers {
    using FirstSet_t = std::map<std::string, std::set<Lookahead_t>>;

    FirstSet_t generate_first_set(const std::set<Production>& productions, const size_t lookahead);
    std::set<Lookahead_t> follow_terminals(const Symbol& to_check, const ParserState& current_state, const FirstSet_t& first_set, const size_t lookahead);
}