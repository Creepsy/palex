#pragma once

#include <cstddef>
#include <vector>
#include <set>
#include <ostream>
#include <variant>
#include <optional>

#include "parser_generator/production_definition.h"

namespace parser_generator::shift_reduce_parsers {
    using ParserStateID_t = size_t;
    using Lookahead_t = std::vector<Symbol>;

    struct Action {
        struct GotoParameters {
            ParserStateID_t next_state;
            Symbol reduced_symbol;
        };
        struct ShiftParameters { // symbol to shift is included in lookahead
            ParserStateID_t next_state;
            Lookahead_t lookahead;
        };
        struct ReduceParameters {
            const Production to_reduce;
            Lookahead_t lookahead;
        };

        std::variant<GotoParameters, ShiftParameters, ReduceParameters> parameters;

        static bool conflict(const Action& first, const Action& second);
    };

    class ProductionState {
        public:
            explicit ProductionState(const Production& production);
            ProductionState(const Production& production, const Lookahead_t& lookahead);
            ProductionState(const Production& production, const size_t position, const Lookahead_t& lookahead);
            bool is_completed() const;
            std::optional<Symbol> get_current_symbol() const;
            const Lookahead_t& get_lookahead() const;
            const Production& get_production() const;
            size_t get_position() const;
            ProductionState advance() const;
            ~ProductionState();
        private:
            const Production production;
            const size_t position;
            Lookahead_t lookahead; 

            friend std::ostream& operator<<(std::ostream& output, const ProductionState& to_print);
            friend bool operator<(const ProductionState& first, const ProductionState& second);  
            friend bool operator==(const ProductionState& first, const ProductionState& second);
    };

    class ParserState {
        public:
            explicit ParserState();
            ParserState(const std::set<ProductionState>& initial_production_states);
            void merge(const ParserState& to_merge);
            void add_action(const Action& to_add);
            bool add_production_state(const ProductionState& to_add); // returns whether the state changed
            ParserState advance_by(const Symbol& to_advance_by) const;
            const std::set<ProductionState>& get_production_states() const;
            const std::set<Action>& get_actions() const;
            ~ParserState();
        private:
            std::set<ProductionState> production_states;
            std::set<Action> action_table;

            bool conflicts_with(const Action& to_check) const;

            friend std::ostream& operator<<(std::ostream& output, const ParserState& to_print);
            friend bool operator<(const ParserState& first, const ParserState& second);
            friend bool operator==(const ParserState& first, const ParserState& second);
    };

    bool operator==(const Action::GotoParameters& first, const Action::GotoParameters& second);
    bool operator==(const Action::ShiftParameters& first, const Action::ShiftParameters& second);
    bool operator==(const Action::ReduceParameters& first, const Action::ReduceParameters& second);
    bool operator==(const Action& first, const Action& second);
    bool operator==(const ProductionState& first, const ProductionState& second);
    bool operator==(const ParserState& first, const ParserState& second);

    bool operator!=(const Action::GotoParameters& first, const Action::GotoParameters& second);
    bool operator!=(const Action::ShiftParameters& first, const Action::ShiftParameters& second);
    bool operator!=(const Action::ReduceParameters& first, const Action::ReduceParameters& second);
    bool operator!=(const Action& first, const Action& second);
    bool operator!=(const ProductionState& first, const ProductionState& second);
    bool operator!=(const ParserState& first, const ParserState& second);

    bool operator<(const Action::GotoParameters& first, const Action::GotoParameters& second);
    bool operator<(const Action::ShiftParameters& first, const Action::ShiftParameters& second);
    bool operator<(const Action::ReduceParameters& first, const Action::ReduceParameters& second);
    bool operator<(const Action& first, const Action& second);
    bool operator<(const ProductionState& first, const ProductionState& second);
    bool operator<(const ParserState& first, const ParserState& second);

    std::ostream& operator<<(std::ostream& output, const Action& to_print);
    std::ostream& operator<<(std::ostream& output, const ProductionState& to_print);
    std::ostream& operator<<(std::ostream& output, const ParserState& to_print);
}

namespace std {
    std::ostream& operator<<(std::ostream& output, const parser_generator::shift_reduce_parsers::Lookahead_t& to_print);
}