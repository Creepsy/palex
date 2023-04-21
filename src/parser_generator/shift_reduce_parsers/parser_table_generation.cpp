#include "parser_table_generation.h"

#include <iterator>
#include <algorithm>
#include <cassert>
#include <functional>

#include "parser_state_comparators.h"
#include "state_lookahead.h"

namespace parser_generator::shift_reduce_parsers {

    // helper functions
    template<class T>
    std::set<T> filter_set(const std::set<T>& to_filter, bool(*predicate)(const T&));
    std::set<Production> filter_entry_productions(const std::set<Production>& productions);
    std::set<ProductionState> filter_reduce_production_states(const std::set<ProductionState>& production_states);
    std::set<Symbol> get_next_symbols(const std::set<ProductionState>& production_states);
    NonterminalMappings_t generate_nonterminal_mappings(const std::set<Production>& productions);
    ParserState create_entry_state(const std::set<Production>& productions, const size_t lookahead);
    ParserState expand_parser_state(
        const ParserState& to_expand, 
        const FirstSet_t& first_set, 
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    );
    void expand_production_state(
        const ProductionState& to_expand, 
        ParserState& target, 
        const FirstSet_t& first_set, 
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    );

    template<class T>
    std::set<T> filter_set(const std::set<T>& to_filter, bool(*predicate)(const T&)) {
        std::set<T> filtered;
        std::copy_if(
            to_filter.begin(), 
            to_filter.end(), 
            std::inserter(filtered, filtered.begin()), 
            predicate
        );
        return filtered;
    }

    std::set<Production> filter_entry_productions(const std::set<Production>& productions) {
        return filter_set<Production>(productions, [](const Production& candidate) -> bool { return candidate.is_entry(); });
    }

    std::set<ProductionState> filter_reduce_production_states(const std::set<ProductionState>& production_states) {
        return filter_set<ProductionState>(
            production_states, 
            [](const ProductionState& candidate) -> bool {
                return candidate.is_completed(); 
            }
        );
    }

    std::set<Symbol> get_next_symbols(const std::set<ProductionState>& production_states) {
        std::set<Symbol> symbols;
        for (const ProductionState& production_state : production_states) {
            if (!production_state.is_completed()) {
                symbols.insert(production_state.get_current_symbol().value());
            }
        }
        return symbols;
    }

    NonterminalMappings_t generate_nonterminal_mappings(const std::set<Production>& productions) {
        NonterminalMappings_t nonterminal_mapping;
        for (const Production& production : productions) {
            nonterminal_mapping[production.name].insert(production);
        }
        return nonterminal_mapping;
    }

    ParserState create_entry_state(const std::set<Production>& productions, const size_t lookahead) {
        const std::set<Production> entry_productions = filter_entry_productions(productions);
        std::set<ProductionState> entry_production_states;
        std::transform(
            entry_productions.begin(), 
            entry_productions.end(), 
            std::inserter(entry_production_states, entry_production_states.begin()),
            [=](const Production& production) -> ProductionState { 
                return ProductionState(production, Lookahead_t(lookahead, Symbol{Symbol::SymbolType::TERMINAL, "EOF"})); 
            }
        );
        return ParserState(entry_production_states);
    }

    ParserState expand_parser_state(
        const ParserState& to_expand, 
        const FirstSet_t& first_set, 
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    ) {
        ParserState target{};
        for (const ProductionState& production_state : to_expand.get_production_states()) {
            expand_production_state(production_state, target, first_set, nonterminal_mappings, lookahead);
        }
        return target;
    }

    void expand_production_state(
        const ProductionState& to_expand, 
        ParserState& target, 
        const FirstSet_t& first_set, 
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    ) {
        if (!target.add_production_state(to_expand)) {
            return;
        }
        if (to_expand.is_completed() || to_expand.get_current_symbol().value().type == Symbol::SymbolType::TERMINAL) {
            return;
        }
        const std::string nonterminal_name = to_expand.get_current_symbol().value().identifier;
        assert(nonterminal_mappings.find(nonterminal_name) != nonterminal_mappings.end() && "BUG: nonterminal mappings are incomplete!");
        for (const Production& expansion : nonterminal_mappings.at(nonterminal_name)) {
            for (const Lookahead_t& expansion_lookahead : follow_terminals(to_expand.get_current_symbol().value(), target, first_set, lookahead)) {
                expand_production_state(ProductionState(expansion, expansion_lookahead), target, first_set, nonterminal_mappings, lookahead);  
            }
        }
    }

    ParserTable::ParserTable(const ParserStateCompare_t state_comparator) : state_comparator(state_comparator) {
    }

    ParserTable::~ParserTable() {
    }

    ParserTable ParserTable::generate(const std::set<Production>& productions, const ParserTable::ParserStateCompare_t state_comparator, const size_t lookahead) {
        const FirstSet_t first_set = generate_first_set(productions, lookahead);
        const NonterminalMappings_t nonterminal_mappings = generate_nonterminal_mappings(productions);
        const ParserState entry_state = create_entry_state(productions, lookahead);
        ParserTable parser_table(state_comparator);
        parser_table.construct_state_from_core(entry_state, first_set, nonterminal_mappings, lookahead);
        return parser_table;
    }

    bool ParserTable::does_exact_state_exist(const ParserState& to_check) {
        return std::find(this->states.begin(), this->states.end(), to_check) != this->states.end();
    }

    ParserStateID_t ParserTable::construct_state_from_core(
        const ParserState& state_core, 
        const FirstSet_t& first_set,
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    ) {
        const ParserStateID_t state_id = this->insert_state(expand_parser_state(state_core, first_set, nonterminal_mappings, lookahead));
        this->generate_actions(state_id, first_set, nonterminal_mappings, lookahead);
        return state_id;
    }

    void ParserTable::generate_actions(
        const ParserStateID_t target_state_id,
        const FirstSet_t& first_set,
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    ) {
        this->generate_reduce_actions(target_state_id);
        this->generate_shift_actions(target_state_id, first_set, nonterminal_mappings, lookahead);
        this->generate_goto_actions(target_state_id, first_set, nonterminal_mappings, lookahead);
    }

    void ParserTable::generate_shift_actions(
        const ParserStateID_t target_state_id,
        const FirstSet_t& first_set,
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    ) {
        for (const Symbol& next_symbol : get_next_symbols(this->states[target_state_id].get_production_states())) {
            if (next_symbol.type != Symbol::SymbolType::TERMINAL) {
                continue;
            }
            const ParserState shift_state_core = this->states[target_state_id].advance_by(next_symbol);
            const ParserStateID_t shift_state_id = this->construct_state_from_core(shift_state_core, first_set, nonterminal_mappings, lookahead);
            for (Lookahead_t lookahead : follow_terminals(next_symbol, this->states[target_state_id], first_set, std::max((size_t)1, lookahead) - 1)) {
                lookahead.insert(lookahead.begin(), next_symbol);
                this->states[target_state_id].add_action(Action{Action::ShiftParameters{shift_state_id, lookahead}});
            }
        }
    }

    void ParserTable::generate_reduce_actions(
        const ParserStateID_t target_state_id
    ) {
        for (const ProductionState& to_reduce : filter_reduce_production_states(this->states[target_state_id].get_production_states())) {
            this->states[target_state_id].add_action(Action{Action::ReduceParameters{to_reduce.get_production(), to_reduce.get_lookahead()}});
        }
    }

    void ParserTable::generate_goto_actions(
        const ParserStateID_t target_state_id,
        const FirstSet_t& first_set,
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    ) {
        for (const Symbol& next_symbol : get_next_symbols(this->states[target_state_id].get_production_states())) {
            if (next_symbol.type != Symbol::SymbolType::NONTERMINAL) {
                continue;
            }
            const ParserState goto_state_core = this->states[target_state_id].advance_by(next_symbol);
            const ParserStateID_t goto_state_id = this->construct_state_from_core(goto_state_core, first_set, nonterminal_mappings, lookahead);
            this->states[target_state_id].add_action(Action{Action::GotoParameters{goto_state_id, next_symbol}});
        }
    }
    
    ParserStateID_t ParserTable::insert_state(const ParserState& to_insert) {
        auto mergeable_state_ptr = std::find_if(
            this->states.begin(), 
            this->states.end(), 
            std::bind(this->state_comparator, to_insert, std::placeholders::_1)
        );
        if (mergeable_state_ptr != this->states.end()) {
            mergeable_state_ptr->merge(to_insert);
            return mergeable_state_ptr - this->states.begin();
        }
        this->states.push_back(to_insert);
        return this->states.size() - 1;
    }

    std::ostream& operator<<(std::ostream& output, const ParserTable& to_print) {
        for (size_t id = 0; id < to_print.states.size(); id++) {
            output << id << ":\n";
            output << to_print.states[id];
            if (id != to_print.states.size() - 1) {
                output << "\n";
            }
        }
        return output;
    }
}