#pragma once

#include <memory>
#include <list>
#include <ostream>
#include <vector>
#include <set>

#include "util/Automaton.h"

#include "util/regex/regex_ast.h"

#include "LexerRuleParser.h"

namespace lexer_generator {
    typedef sm::Automaton<std::u32string, regex::CharRangeSet> LexerAutomaton_t;

    void resolve_connection_collisions(
        const LexerAutomaton_t::Connection& to_add, 
        std::vector<std::pair<regex::CharRangeSet, std::set<LexerAutomaton_t::StateID_t>>>& dfa_connections
    );

    std::vector<std::u32string> merge_states_to_vector(const std::vector<std::u32string>& to_merge);
    
    void insert_rule_in_nfa(LexerAutomaton_t& nfa, const LexerAutomaton_t::StateID_t root_state, const TokenRegexRule& to_insert);

    LexerAutomaton_t::StateID_t insert_regex_ast_in_nfa(
        LexerAutomaton_t& nfa, 
        const LexerAutomaton_t::StateID_t root_state, 
        const regex::RegexBase* const to_insert
    );
    LexerAutomaton_t::StateID_t insert_regex_branch_in_nfa(
        LexerAutomaton_t& nfa, 
        const LexerAutomaton_t::StateID_t root_state, 
        const regex::RegexBranch* const to_insert
    );
    LexerAutomaton_t::StateID_t insert_regex_char_set_in_nfa(
        LexerAutomaton_t& nfa, 
        const LexerAutomaton_t::StateID_t root_state, 
        const regex::RegexCharSet* const to_insert
    );
    LexerAutomaton_t::StateID_t insert_regex_quantifier_in_nfa(
        LexerAutomaton_t& nfa, 
        const LexerAutomaton_t::StateID_t root_state, 
        const regex::RegexQuantifier* const to_insert
    );
    LexerAutomaton_t::StateID_t insert_regex_sequence_in_nfa(
        LexerAutomaton_t& nfa, 
        const LexerAutomaton_t::StateID_t root_state, 
        const regex::RegexSequence* const to_insert
    );
}