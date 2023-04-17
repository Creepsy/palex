#include "parser_table_generation.h"

#include <optional>
#include <iterator>
#include <cassert>
#include <algorithm>

#include "util/palex_except.h"

namespace parser_generator::shift_reduce_parsers {
    // helper functions
    FirstSet_t generate_incomplete_first_set(const std::set<Production>& productions, const size_t lookahead, const FirstSet_t& previous_iter = {});
    std::optional<std::set<Lookahead_t>> try_get_terminals(
        const ProductionState& production_state, 
        const size_t lookahead, 
        const FirstSet_t& previous_iter
    );
    std::optional<std::set<Lookahead_t>> try_get_symbol_terminals(const ProductionState& production_state, const FirstSet_t& previous_iter);
    bool is_first_set_valid(const FirstSet_t& to_validate, const std::set<Production>& used_productions);
    std::set<Lookahead_t> combinations(const std::set<Lookahead_t>& base, const std::set<Lookahead_t>& to_append);
    std::set<Lookahead_t> truncate(const std::set<Lookahead_t>& to_truncate, const size_t max_length);

    FirstSet_t generate_incomplete_first_set(const std::set<Production>& productions, const size_t lookahead, const FirstSet_t& previous_iter) {
        FirstSet_t first_set;
        for (const Production& production : productions) {
            std::optional<std::set<Lookahead_t>> production_first_terminals = try_get_terminals(ProductionState(production), lookahead, previous_iter);
            if (production_first_terminals.has_value()) {
                first_set[production.name].insert(production_first_terminals.value().begin(), production_first_terminals.value().end());
            }
        }
        return first_set;
    }

    std::optional<std::set<Lookahead_t>> try_get_terminals(
        const ProductionState& production_state, 
        const size_t lookahead, 
        const FirstSet_t& previous_iter
    ) {
        if (lookahead == 0 || production_state.is_completed()) {
            return std::set<Lookahead_t>{Lookahead_t{}};
        }
        std::optional<std::set<Lookahead_t>> symbol_first_terminals = try_get_symbol_terminals(production_state, previous_iter);
        if (!symbol_first_terminals.has_value()) {
            return std::nullopt;
        }
        size_t min_consumed = std::min_element(symbol_first_terminals.value().begin(), symbol_first_terminals.value().end())->size();
        std::optional<std::set<Lookahead_t>> sub_first_terminals = try_get_terminals(production_state.advance(), lookahead - min_consumed, previous_iter);  
        if (!sub_first_terminals.has_value()) {
            return std::nullopt;
        }
        return truncate(combinations(symbol_first_terminals.value(), sub_first_terminals.value()), lookahead);
    }

    std::optional<std::set<Lookahead_t>> try_get_symbol_terminals(const ProductionState& production_state, const FirstSet_t& previous_iter) {
        const std::optional<Symbol> curr_symbol = production_state.get_current_symbol();
        assert(curr_symbol.has_value() && "Bug: Tried to get lookahead of already completed production!");
        if (curr_symbol.value().type == Symbol::SymbolType::TERMINAL) {
            return std::set<Lookahead_t>{Lookahead_t{curr_symbol.value()}};
        }
        const std::string& non_terminal_name = curr_symbol.value().identifier;
        if (previous_iter.find(non_terminal_name) == previous_iter.end()) {
            return std::nullopt; // this first-set can't be currently completed as information is still missing in this iteration
        }
        return previous_iter.at(non_terminal_name);
    }

    bool is_first_set_valid(const FirstSet_t& to_validate, const std::set<Production>& used_productions) {
        for (const Production& production : used_productions) {
            if (to_validate.find(production.name) == to_validate.end()) {
                return false;
            }
        }
        return true;
    }

    std::set<Lookahead_t> combinations(const std::set<Lookahead_t>& base, const std::set<Lookahead_t>& to_append) {
        std::set<Lookahead_t> all_combinations;
        for (const Lookahead_t& base_part : base) {
            for (const Lookahead_t& append_part : to_append) {
                Lookahead_t combination = base_part;
                combination.insert(combination.end(), append_part.begin(), append_part.end());
                all_combinations.insert(combination);
            }
        }
        return all_combinations;
    }

    std::set<Lookahead_t> truncate(const std::set<Lookahead_t>& to_truncate, const size_t max_length) {
        std::set<Lookahead_t> truncated;
        std::transform(
            to_truncate.begin(), 
            to_truncate.end(), 
            std::inserter(truncated, truncated.begin()), 
            [&](const Lookahead_t& to_truncate) -> Lookahead_t {
                if (to_truncate.size() > max_length) {
                    return Lookahead_t(to_truncate.begin(), to_truncate.begin() + (ssize_t)max_length);
                }
                return to_truncate;
            }
        );
        return truncated;
    }

    FirstSet_t generate_first_set(const std::set<Production>& productions, const size_t lookahead) {
        FirstSet_t first_set = generate_incomplete_first_set(productions, lookahead);
        while (true) {
            FirstSet_t next_iter = generate_incomplete_first_set(productions, lookahead, first_set);
            if (next_iter == first_set) {
                break;
            }
            first_set = next_iter;
        }
        if (!is_first_set_valid(first_set, productions)) {
            throw palex_except::ValidationError("Unable to create first set of productions as some of them recurse infinitely!");
        }
        return first_set;
    }

    std::set<Lookahead_t> follow_terminals(const Symbol& to_check, const ParserState& current_state, const FirstSet_t& first_set, const size_t lookahead) {
        std::set<Lookahead_t> follow_terminals_set;
        for (const ProductionState& production_state : current_state.get_production_states()) {
            if (!production_state.is_completed() && production_state.get_current_symbol() == to_check) {
                std::optional<std::set<Lookahead_t>> production_follow_terminals = try_get_terminals(production_state.advance(), lookahead, first_set);
                assert(production_follow_terminals.has_value() && "BUG: 'follow_tokens' got used with invalid first set!");
                assert(
                    !production_state.get_lookaheads().empty() && 
                    "BUG: production state has no lookahead, which is needed for computing the follow terminals!"
                );
                std::set<Lookahead_t> expanded_follow_terminals = truncate(
                    combinations(production_follow_terminals.value(), production_state.get_lookaheads()), 
                    lookahead
                );
                follow_terminals_set.insert(expanded_follow_terminals.begin(), expanded_follow_terminals.end());
            }
        }
        assert(
            !follow_terminals_set.empty() && 
            "BUG: Supplied a symbol to 'follow_tokens' in combination with a parser state that has no production starting with that symbol!"
        );
        return follow_terminals_set;
    }
}