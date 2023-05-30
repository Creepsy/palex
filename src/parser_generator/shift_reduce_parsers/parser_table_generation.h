#pragma once

#include <set>
#include <cstddef>
#include <string>
#include <map>
#include <vector>
#include <optional>

#include "parser_state.h"
#include "state_lookahead.h"

namespace parser_generator::shift_reduce_parsers {    
    using NonterminalMappings_t = std::map<std::string, std::set<Production>>;

    struct DebugParseTree {
        std::string identifier;
        std::vector<DebugParseTree> sub_nodes;
    }; 
    bool operator==(const DebugParseTree& first, const DebugParseTree& second);
    std::ostream& operator<<(std::ostream& output, const DebugParseTree& to_print);

    class ParserTable {
        public:
            using ParserStateCompare_t = bool(*)(const ParserState&, const ParserState&);

            ParserTable(const ParserStateCompare_t state_comparator);
            DebugParseTree debug_parse(const std::vector<std::string>& token_names) const;
            ~ParserTable();

            static ParserTable generate(const std::set<Production>& productions, const ParserTable::ParserStateCompare_t state_comparator, const size_t lookahead);
        private:
            const ParserStateCompare_t state_comparator;
            std::vector<ParserState> states;

            std::optional<ParserStateID_t> try_get_exact_state_id(const ParserState& to_find);
            ParserStateID_t construct_state_from_core(
                const ParserState& state_core, 
                const FirstSet_t& first_set,
                const NonterminalMappings_t& nonterminal_mappings,
                const size_t lookahead
            );
            void generate_actions(
                const ParserStateID_t target_state_id,
                const FirstSet_t& first_set,
                const NonterminalMappings_t& nonterminal_mappings,
                const size_t lookahead
            );
            void generate_shift_actions(
                const ParserStateID_t target_state_id,
                const FirstSet_t& first_set,
                const NonterminalMappings_t& nonterminal_mappings,
                const size_t lookahead
            );
            void generate_reduce_actions(
                const ParserStateID_t target_state_id
            );
            void generate_goto_actions(
                const ParserStateID_t target_state_id,
                const FirstSet_t& first_set,
                const NonterminalMappings_t& nonterminal_mappings,
                const size_t lookahead
            );
            ParserStateID_t insert_state(const ParserState& to_insert);
            const Action& debug_next_action(const std::vector<std::string>& token_names, const size_t curr_state, const size_t curr_position) const;
            size_t debug_goto(const size_t curr_state, const std::string& reduced) const;

            friend std::ostream& operator<<(std::ostream& output, const ParserTable& to_print);
    };

    std::ostream& operator<<(std::ostream& output, const ParserTable& to_print);
}