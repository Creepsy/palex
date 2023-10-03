#include "parser_table_generation.h"

#include <iterator>
#include <algorithm>
#include <cassert>
#include <functional>
#include <stack>
#include <utility>
#include <variant>

#include "util/palex_except.h"
#include "util/Visitor.h"
#include "util/stream_format.h"

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
    bool matches_lookahead(const std::vector<std::string>& token_names, const Lookahead_t& lookahead, const size_t curr_position);
    size_t get_current_state(const std::stack<std::pair<size_t, DebugParseTree>>& parse_stack);
    DebugParseTree reduce_production(const Production& to_reduce, std::stack<std::pair<size_t, DebugParseTree>>& parse_stack);

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
                return ProductionState(production, Lookahead_t(lookahead, Symbol{Symbol::SymbolType::TERMINAL, "END_OF_FILE"})); 
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
        const std::set<Lookahead_t> follow_set = follow_terminals(to_expand.get_current_symbol().value(), target, first_set, lookahead);
        for (const Production& expansion : nonterminal_mappings.at(nonterminal_name)) {
            for (const Lookahead_t& expansion_lookahead : follow_set) {
                expand_production_state(ProductionState(expansion, expansion_lookahead), target, first_set, nonterminal_mappings, lookahead);  
            }
        }
    }

    bool matches_lookahead(const std::vector<std::string>& token_names, const Lookahead_t& lookahead, const size_t curr_position) {
        const size_t sequence_length = std::min(lookahead.size(), token_names.size() - curr_position);
        for (size_t i = 0; i < sequence_length; i++) {
            if (token_names[curr_position + i] != lookahead[i].identifier) {
                return false;
            }
        }
        for (size_t i = sequence_length; i < lookahead.size(); i++) { // all remaining tokens after input end have to be EOF
            if (lookahead[i].identifier != "END_OF_FILE") {
                return false;
            }
        }
        return true;
    }

    size_t get_current_state(const std::stack<std::pair<size_t, DebugParseTree>>& parse_stack) {
        return parse_stack.empty() ? 0 : parse_stack.top().first;
    }

    bool operator==(const DebugParseTree& first, const DebugParseTree& second) {
        return first.identifier == second.identifier && first.sub_nodes == second.sub_nodes;
    }

    std::ostream& operator<<(std::ostream& output, const DebugParseTree& to_print) {
        output << to_print.identifier << "\n";
        output << sfmt::Indentation{1};
        for (const DebugParseTree& sub_node : to_print.sub_nodes) {
            output << sub_node;
        }
        output << sfmt::Indentation{-1};
        return output;
    }

    DebugParseTree reduce_production(const Production& to_reduce, std::stack<std::pair<size_t, DebugParseTree>>& parse_stack) {
        std::vector<DebugParseTree> sub_nodes;
        assert(to_reduce.symbols.size() <= parse_stack.size() && "BUG: Parse stack doesn't have enough elements for rule reduction!"); 
        for (size_t i = 0; i < to_reduce.symbols.size(); i++) {
            sub_nodes.push_back(parse_stack.top().second);
            parse_stack.pop();
        }
        std::reverse(sub_nodes.begin(), sub_nodes.end()); // stack pop reverses order, so we have to reverse it again
        return DebugParseTree{to_reduce.name, sub_nodes};
    }

    ParserTable::ParserTable(const ParserStateComparator_t& state_comparator) : state_comparator(state_comparator) {
    }

    DebugParseTree ParserTable::debug_parse(const std::vector<std::string>& token_names) const {
        std::stack<std::pair<size_t, DebugParseTree>> parse_stack;
        size_t curr_position = 0;
        bool accepted = false;

        while (!accepted) {
            const Action next_action = this->debug_next_action(token_names, get_current_state(parse_stack), curr_position);
            accepted = std::visit(
                Visitor{
                    [](const Action::GotoParameters& goto_action) -> bool {
                        assert(false && "BUG: debug_next_action should never return a goto action!"); 
                        return false;
                    },
                    [&](const Action::ShiftParameters& shift_action) -> bool {
                        parse_stack.push(std::make_pair(shift_action.next_state, DebugParseTree{token_names[curr_position++]}));
                        return false;
                    },
                    [&](const Action::ReduceParameters& reduce_action) -> bool {
                        const DebugParseTree reduced = reduce_production(reduce_action.to_reduce, parse_stack);
                        const size_t next_state = reduce_action.to_reduce.is_entry() 
                            ? 0 // when we accept the input, the next state doesn't matter
                            : this->debug_goto(get_current_state(parse_stack), reduce_action.to_reduce.name);
                        parse_stack.push(std::make_pair(next_state, reduced));
                        return reduce_action.to_reduce.is_entry();
                    }
                },
                next_action.parameters
            );
        }
        
        if (parse_stack.empty()) {
            throw palex_except::ParserError("Empty parse stack after parse!");
        }
        if (parse_stack.size() != 1) {
            throw palex_except::ParserError("Parse stack contains more than one element after parse!");
        }
        return parse_stack.top().second;
    }

    const std::vector<ParserState>& ParserTable::get_states() const {
        return this->states;
    }

    ParserTable::~ParserTable() {
    }

    ParserTable ParserTable::generate(const std::set<Production>& productions, const ParserStateComparator_t& state_comparator, const size_t lookahead) {
        const FirstSet_t first_set = generate_first_set(productions, lookahead);
        const NonterminalMappings_t nonterminal_mappings = generate_nonterminal_mappings(productions);
        const ParserState entry_state = create_entry_state(productions, lookahead);
        ParserTable parser_table(state_comparator);
        parser_table.construct_state_from_core(entry_state, first_set, nonterminal_mappings, lookahead);
        return parser_table;
    }

    std::optional<ParserStateID_t> ParserTable::try_get_matching_state_id(const ParserState& to_find) {
        const auto state_ptr = std::find_if(
            this->states.begin(), 
            this->states.end(), 
            std::bind(this->state_comparator, to_find, std::placeholders::_1)
        );
        if (state_ptr == this->states.end()) {
            return std::nullopt;
        }
        const bool are_all_lookaheads_covered = std::includes(
            state_ptr->get_production_states().begin(),
            state_ptr->get_production_states().end(),
            to_find.get_production_states().begin(),
            to_find.get_production_states().end()
        );
        // if not all lookaheads are covered, we need to generate the actions for the production states to be added; therefore the states aren't matching
        if (!are_all_lookaheads_covered) {
            return std::nullopt;
        }
        return std::optional<ParserStateID_t>{state_ptr - this->states.begin()};
    }

    ParserStateID_t ParserTable::construct_state_from_core(
        const ParserState& state_core, 
        const FirstSet_t& first_set,
        const NonterminalMappings_t& nonterminal_mappings,
        const size_t lookahead
    ) {
        const ParserState expanded_state = expand_parser_state(state_core, first_set, nonterminal_mappings, lookahead);
        std::optional<ParserStateID_t> exact_state_id = this->try_get_matching_state_id(expanded_state);
        if (exact_state_id.has_value()) {
            return exact_state_id.value();
        }
        const ParserStateID_t state_id = this->insert_state(expanded_state);
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

    const Action& ParserTable::debug_next_action(const std::vector<std::string>& token_names, const size_t curr_state, const size_t curr_position) const {
        assert(curr_state < this->states.size() && "BUG: Tried to access non-existent state!");
        const Action* default_action = nullptr;
        for (const Action& action : this->states[curr_state].get_actions()) {
            bool lookahead_match = std::visit(
                Visitor{
                    [](const Action::GotoParameters& goto_action) -> bool {
                        return false;
                    },
                    [&](const Action::ShiftParameters& shift_action) -> bool {
                        return matches_lookahead(token_names, shift_action.lookahead, curr_position);
                    },
                    [&](const Action::ReduceParameters& reduce_action) -> bool {
                        return matches_lookahead(token_names, reduce_action.lookahead, curr_position);
                    }
                },
                action.parameters
            );
            if (!lookahead_match) {
                continue;
            }
            bool empty_lookahead = std::visit(
                Visitor{
                    [](const Action::GotoParameters& goto_action) -> bool {
                        return false;
                    },
                    [&](const Action::ShiftParameters& shift_action) -> bool {
                        return shift_action.lookahead.empty();
                    },
                    [&](const Action::ReduceParameters& reduce_action) -> bool {
                        return reduce_action.lookahead.empty();
                    }
                },
                action.parameters
            );
            if (!empty_lookahead) {
                return action;
            }
            default_action = &action;
        }
        if (!default_action) {
            throw palex_except::ParserError("Invalid lookahead! No matching shift-/reduce-action found!");
        }
        return *default_action; 
    }

    size_t ParserTable::debug_goto(const size_t curr_state, const std::string& reduced) const {
        assert(curr_state < this->states.size() && "BUG: Tried to access non-existent state!");
        const auto goto_action = std::find_if(
            this->states[curr_state].get_actions().begin(),
            this->states[curr_state].get_actions().end(),
            [&](const Action& action) -> bool {
                return std::visit(
                    Visitor{
                        [&](const Action::GotoParameters& goto_action) -> bool {
                            return goto_action.reduced_symbol.identifier == reduced;
                        },
                        [](const Action::ShiftParameters& shift_action) -> bool {
                            return false;
                        },
                        [](const Action::ReduceParameters& reduce_action) -> bool {
                            return false;
                        }
                    },
                    action.parameters
                );
            }
        );
        if (goto_action == this->states[curr_state].get_actions().end()) {
            throw palex_except::ParserError("Invalid parser state! No matching goto action found!");
        }
        return std::get<Action::GotoParameters>(goto_action->parameters).next_state;
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