#include "parser_state.h"

#include <iterator>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <sstream>

#include "util/Visitor.h"

namespace parser_generator::shift_reduce_parsers {
    bool Action::conflict(const Action& first, const Action& second) {
        return std::visit( 
            Visitor{
                [](const Action::ShiftParameters& f, const Action::ShiftParameters& s) -> bool { 
                    return f.lookahead == s.lookahead && f != s; 
                },
                [](const Action::ReduceParameters& f, const Action::ReduceParameters& s) -> bool { 
                    return f.lookahead == s.lookahead && f != s; 
                },
                [](const Action::ShiftParameters& f, const Action::ReduceParameters& s) -> bool { 
                    return f.lookahead == s.lookahead; 
                },
                [](const Action::ReduceParameters& f, const Action::ShiftParameters& s) -> bool { 
                    return f.lookahead == s.lookahead;
                },
                [](const Action::GotoParameters& f, const Action::GotoParameters& s) -> bool { 
                    return f.reduced_symbol == s.reduced_symbol && f != s; 
                },
                [](const auto& f, const auto& s) -> bool { 
                    return false; 
                }
            },
            first.parameters,
            second.parameters
        );
    }

    ProductionState::ProductionState(const Production& production) : ProductionState(production, 0, {}) {
    }

    ProductionState::ProductionState(const Production& production, const std::set<Lookahead_t>& lookaheads) : ProductionState(production, 0, lookaheads) {

    }

    ProductionState::ProductionState(const Production& production, const size_t position, const std::set<Lookahead_t>& lookaheads) 
    : production(production), position(std::min(position, production.symbols.size())), lookaheads(lookaheads) {

    }

    bool ProductionState::add_lookahead(const Lookahead_t& to_add) {
        if (this->lookaheads.find(to_add) != this->lookaheads.end()) {
            return false;
        }
        this->lookaheads.insert(to_add);
        return true;
    }

    bool ProductionState::is_completed() const {
        return this->position == this->production.symbols.size();
    }

    std::optional<Symbol> ProductionState::get_current_symbol() const {
        if (this->is_completed()) {
            return std::nullopt;
        }
        return this->production.symbols[this->position];
    }

    const std::set<Lookahead_t>& ProductionState::get_lookaheads() const {
        return this->lookaheads;
    }

    const Production& ProductionState::get_production() const {
        return this->production;
    }

    size_t ProductionState::get_position() const {
        return this->position;
    }


    ProductionState ProductionState::advance() const {
        if (this->is_completed()) {
            throw std::runtime_error("Tried to advance the position past the end of the rule!");
        }
        return ProductionState(this->production, this->position + 1, this->lookaheads);
    }
        
    ProductionState::~ProductionState() {
    }

    ParserState::ParserState() {
    }

    ParserState::ParserState(const std::set<ProductionState>& initial_production_states)
     : production_states(initial_production_states) {

    }

    void ParserState::merge(const ParserState& to_merge) {
        for (const ProductionState& production_state : to_merge.production_states) {
            this->add_production_state(production_state);
        }
        for (const Action& action : to_merge.action_table) {
            if (this->conflicts_with(action)) {
                std::stringstream err;
                err << "Conflict occured on state merge:\n" 
                    << "State A:\n" << *this
                    << "State B:\n" << to_merge;
                throw std::runtime_error(err.str());
            }
            this->action_table.insert(action);
        }
    }

    void ParserState::add_action(const Action& to_add) {
        if (this->conflicts_with(to_add)) {
            std::stringstream err;
            err << "The action conflicts with the state it is supposed to be in:\n" 
                << "Action: " << to_add
                << "State:\n" << *this;
            throw std::runtime_error(err.str());
        }
        this->action_table.insert(to_add);
    }

    bool ParserState::add_production_state(const ProductionState& to_add) {
        const auto production_ptr = this->production_states.find(to_add);
        if (production_ptr == this->production_states.end()) {
            this->production_states.insert(to_add);
            return true;
        }
        
        const std::set<Lookahead_t> existent_lookaheads = production_ptr->get_lookaheads();
        std::set<Lookahead_t> union_lookaheads = existent_lookaheads;
        union_lookaheads.insert(to_add.get_lookaheads().begin(), to_add.get_lookaheads().end());

        this->production_states.erase(to_add);
        this->production_states.insert(ProductionState(to_add.get_production(), to_add.get_position(), union_lookaheads));
        return existent_lookaheads != union_lookaheads;
    }

    ParserState ParserState::advance_by(const Symbol& to_advance_by) const {
        std::set<ProductionState> advanced_production_states;
        for (const ProductionState& production_state : this->production_states) {
            if (!production_state.is_completed() && production_state.get_current_symbol() == to_advance_by) {
                advanced_production_states.insert(production_state.advance());
            }
        }
        assert((!advanced_production_states.empty()) && "BUG: Tried to advance into an empty state!");
        return ParserState(advanced_production_states);
    }

    const std::set<ProductionState>& ParserState::get_production_states() const {
        return this->production_states;
    }

    ParserState::~ParserState() {
    }

    bool ParserState::conflicts_with(const Action& to_check) const {
        return std::any_of(
            this->action_table.begin(), 
            this->action_table.end(), 
            [&](const Action& candidate) -> bool { return Action::conflict(to_check, candidate); }
        );
    }

    bool operator==(const Action::GotoParameters& first, const Action::GotoParameters& second) {
        return first.next_state == second.next_state && first.reduced_symbol == second.reduced_symbol;
    }

    bool operator==(const Action::ShiftParameters& first, const Action::ShiftParameters& second) {
        return first.next_state == second.next_state && first.lookahead == second.lookahead;
    }

    bool operator==(const Action::ReduceParameters& first, const Action::ReduceParameters& second) {
        return first.lookahead == second.lookahead && first.to_reduce == second.to_reduce;
    }

    bool operator==(const Action& first, const Action& second) {
        return first.parameters == second.parameters;
    }

    bool operator==(const ProductionState& first, const ProductionState& second) {
        return first.production == second.production && first.position == second.position && first.lookaheads == second.lookaheads;
    }

    bool operator==(const ParserState& first, const ParserState& second) {
        // actions are defined by production states -> seperate func later
        return first.production_states == second.production_states; // && first.shift_reduce_table == second.shift_reduce_table && first.goto_table == second.goto_table;
    }

    bool operator!=(const Action::GotoParameters& first, const Action::GotoParameters& second) {
        return !(first == second);
    }

    bool operator!=(const Action::ShiftParameters& first, const Action::ShiftParameters& second) {
        return !(first == second);
    }

    bool operator!=(const Action::ReduceParameters& first, const Action::ReduceParameters& second) {
        return !(first == second);
    }

    bool operator<(const Action::GotoParameters& first, const Action::GotoParameters& second) {
        if (first.next_state < second.next_state) {
            return true;
        }
        return first.next_state == second.next_state && first.reduced_symbol < second.reduced_symbol;
    }

    bool operator<(const Action::ShiftParameters& first, const Action::ShiftParameters& second) {
        if (first.next_state < second.next_state) {
            return true;
        }
        return first.next_state == second.next_state && first.lookahead < second.lookahead;
    }

    bool operator<(const Action::ReduceParameters& first, const Action::ReduceParameters& second) {
        if (first.to_reduce < second.to_reduce) {
            return true;
        }
        return first.to_reduce == second.to_reduce && first.lookahead < second.lookahead;
    }
    
    bool operator<(const Action& first, const Action& second) {
        return first.parameters < second.parameters;
    }

    bool operator<(const ProductionState& first, const ProductionState& second) {
        if (first.production < second.production) {
            return true;
        }
        return first.production == second.production && first.position < second.position;
    }

    bool operator<(const ParserState& first, const ParserState& second) {
        if (first.production_states < second.production_states) {
            return true;
        }
        return first.production_states == second.production_states && first.action_table < second.action_table;
    }

    std::ostream& operator<<(std::ostream& output, const Action& to_print) {
        std::visit(
            Visitor{
                [&](const Action::GotoParameters& goto_action) { 
                    output << "goto " << goto_action.next_state << " on " << goto_action.reduced_symbol; 
                },
                [&](const Action::ShiftParameters& shift_action) {
                    output << "shift " << shift_action.next_state << " on [" << shift_action.lookahead << "]"; 
                },
                [&](const Action::ReduceParameters& reduce_action) { 
                    output << "reduce " << reduce_action.to_reduce << " on [" << reduce_action.lookahead << "]"; 
                }
            },
            to_print.parameters
        );
        return output;
    }

    std::ostream& operator<<(std::ostream& output, const ProductionState& to_print) {
        output << to_print.production.name << " =";
        for (size_t i = 0; i < to_print.production.symbols.size(); i++) {
            if (i == to_print.position) {
                output << " .";
            }
            output << " " << to_print.production.symbols[i];
        }
        if (to_print.is_completed()) {
            output << " .";
        }
        return output << " " << to_print.lookaheads;
    }

    std::ostream& operator<<(std::ostream& output, const ParserState& to_print) {
        for (const ProductionState& production_state : to_print.production_states) {
            output << "\t" << production_state << "\n";
        }
        for (const Action& action : to_print.action_table) {
            output << "\t" << action << "\n";
        }
        return output;
    }
}

std::ostream& std::operator<<(std::ostream& output, const parser_generator::shift_reduce_parsers::Lookahead_t& to_print) {
    for(auto lookahead_iter = to_print.begin(); lookahead_iter != to_print.end(); lookahead_iter++) {
        if (lookahead_iter != to_print.begin()) {
            output << " ";
        }
        output << *lookahead_iter;
    }
    return output;
}

std::ostream& std::operator<<(std::ostream& output, const std::set<parser_generator::shift_reduce_parsers::Lookahead_t>& to_print) {
    output << "[";
    for (auto lookaheads_iter = to_print.begin(); lookaheads_iter != to_print.end(); lookaheads_iter++) {
        if (lookaheads_iter != to_print.begin()) {
            output << ", ";
        }
        output << *lookaheads_iter;
    }
    return output << "]";
}