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
    
    class ParserTable {
        public:
            using ParserStateCompare_t = bool(*)(const ParserState&, const ParserState&);

            ParserTable(const ParserStateCompare_t state_comparator);
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

            friend std::ostream& operator<<(std::ostream& output, const ParserTable& to_print);
    };

    std::ostream& operator<<(std::ostream& output, const ParserTable& to_print);
}