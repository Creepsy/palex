#pragma once

#include <memory>
#include <list>
#include <ostream>

#include "util/Automaton.h"

#include "util/regex/regex_ast.h"

#include "LexerRuleParser.h"

namespace lexer_generator {
    struct CharRangeSetWrapper { // Wrapper to overwrite operator<< -> graphviz output
        std::list<regex::CharRange> ranges;
    };

    typedef sm::Automaton<std::u32string, CharRangeSetWrapper> LexerAutomaton_t;

    std::ostream& operator<<(std::ostream& output, const CharRangeSetWrapper& to_print);
    
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