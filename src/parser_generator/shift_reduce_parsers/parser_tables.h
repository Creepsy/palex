#pragma once

#include <cstddef>
#include <vector>
#include <set>
#include <ostream>
#include <variant>

#include "parser_generator/lang/ParserProductionParser.h"

namespace parser_generator::shift_reduce_parsers {
    using ParserStateID_t = size_t;
    using Lookahead_t = std::vector<Symbol>;

    // TODO: currently only class dummies
    // how to model different parser types (LR, LALR, ...) ? -> inheritance; but only for parser table? (not for production state & parser state)
    // only differ in how states are merged; so it should be sufficient to overwrite ParserTable state_insert or pass function on construction

    // Currently only LR(n) implementation adapt later for further types

    struct Action {
        struct GotoParameters {
            ParserStateID_t next_state;
            Symbol reduced_symbol;
        };
        struct ShiftParameters { // symbol to shift is included in lookahead?
            ParserStateID_t next_state;
            Lookahead_t lookahead;
        };
        struct ReduceParameters {
            const Production& to_reduce;
            Lookahead_t lookahead;
        };

        std::variant<GotoParameters, ShiftParameters, ReduceParameters> parameters;

        static bool conflict(const Action& first, const Action& second);
    };

    class ProductionState {
        public:
            ProductionState(const Production& production);
            bool is_completed() const;
            ProductionState advance() const;
            ~ProductionState();
        private:
            ProductionState(const Production& production, const size_t position);
            // not quite sure whether making this const causes problems later on...
            const Production& production;
            const size_t position;
            std::set<Lookahead_t> lookaheads;

            friend std::ostream& operator<<(std::ostream& output, const ProductionState& to_print);
            friend bool operator<(const ProductionState& first, const ProductionState& second); 
            friend bool operator==(const ProductionState& first, const ProductionState& second);
    };

    class ParserState {
        public:
            ParserState(const ParserStateID_t id);
            ~ParserState();
        private:
            const ParserStateID_t id;
            std::set<ProductionState> production_states;
            // use assertions to validate
            std::set<Action> shift_reduce_table;
            std::set<Action> goto_table; 

            friend std::ostream& operator<<(std::ostream& output, const ParserState& to_print);
            friend bool operator<(const ParserState& first, const ParserState& second);
    };

    class ParserTable {
        public:
            ParserTable();
            ~ParserTable();
        private:
            std::set<ParserState> states;

            friend std::ostream& operator<<(std::ostream& output, const ParserTable& to_print);
    };    

    bool operator==(const Action::GotoParameters& first, const Action::GotoParameters& second);
    bool operator==(const Action::ShiftParameters& first, const Action::ShiftParameters& second);
    bool operator==(const Action::ReduceParameters& first, const Action::ReduceParameters& second);
    bool operator==(const Action& first, const Action& second);
    bool operator==(const ProductionState& first, const ProductionState& second);

    bool operator!=(const Action::GotoParameters& first, const Action::GotoParameters& second);
    bool operator!=(const Action::ShiftParameters& first, const Action::ShiftParameters& second);
    bool operator!=(const Action::ReduceParameters& first, const Action::ReduceParameters& second);

    bool operator<(const Action::GotoParameters& first, const Action::GotoParameters& second);
    bool operator<(const Action::ShiftParameters& first, const Action::ShiftParameters& second);
    bool operator<(const Action::ReduceParameters& first, const Action::ReduceParameters& second);
    bool operator<(const Action& first, const Action& second);
    bool operator<(const ProductionState& first, const ProductionState& second);
    bool operator<(const ParserState& first, const ParserState& second);

    std::ostream& operator<<(std::ostream& output, const Action& to_print);
    std::ostream& operator<<(std::ostream& output, const ProductionState& to_print);
    std::ostream& operator<<(std::ostream& output, const ParserState& to_print);
    std::ostream& operator<<(std::ostream& output, const ParserTable& to_print);
}

namespace std {
    std::ostream& operator<<(std::ostream& output, const parser_generator::shift_reduce_parsers::Lookahead_t& to_print);
    std::ostream& operator<<(std::ostream& output, const std::set<parser_generator::shift_reduce_parsers::Lookahead_t>& to_print);
}