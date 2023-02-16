#include "lexer_automaton.h"

#include <cassert>
#include <vector>
#include <algorithm>
#include <sstream>

#include "util/utf8.h"

#include "util/palex_except.h"

#include "regex/RegexParser.h"

// helper functions

void throw_ambiguous_priority_err(const std::vector<std::u32string>& ambiguous_tokens, const size_t priority);

void throw_ambiguous_priority_err(const std::vector<std::u32string>& ambiguous_tokens, const size_t priority) {
    std::stringstream err_msg{};
    err_msg << "Tokens with ambiguous priority in the same state! The tokens ";

    for (size_t i = 0; i < ambiguous_tokens.size(); i++) {
        if (i != 0) {
            err_msg << ", ";
        }
        err_msg << ambiguous_tokens[i];
    }

    err_msg << " have the same priority of " << priority;

    throw palex_except::ValidationError(err_msg.str());
}

void lexer_generator::resolve_connection_collisions(
    const LexerAutomaton_t::Connection& to_add, 
    std::vector<std::pair<regex::CharRangeSet, std::set<LexerAutomaton_t::StateID_t>>>& dfa_connections
) {
    assert((to_add.value.has_value() && "tried to insert epsilon connection into dfa!"));
    regex::CharRangeSet remaining_value = to_add.value.value();

    for (auto iter = dfa_connections.begin(); iter != dfa_connections.end(); iter++) {     
        regex::CharRangeSet intersection = remaining_value.get_intersection((*iter).first);
        if (intersection.empty()) {
            continue;
        }
        if (intersection != (*iter).first) {
            iter = dfa_connections.insert(iter, std::make_pair(intersection, (*iter).second));
        }
        (*iter).second.insert(to_add.target);

        remaining_value = remaining_value - intersection;
    }

    if (!remaining_value.empty()) {
        dfa_connections.push_back(std::make_pair(remaining_value, std::set<size_t>{to_add.target}));
    }
}

std::map<std::u32string, size_t> lexer_generator::get_token_priorities(const std::vector<TokenRegexRule>& rules) {
    std::map<std::u32string, size_t> token_prioritites;

    for (const TokenRegexRule& rule : rules) {
        token_prioritites.insert(std::make_pair(rule.name, rule.priority));
    }    

    return token_prioritites;
}

std::u32string lexer_generator::merge_states_by_priority(const std::map<std::u32string, size_t>& token_priorities, const std::vector<std::u32string>& to_merge) {
    size_t highest_priority = 0;
    std::vector<std::u32string> hp_tokens;


    for (const std::u32string& token : to_merge) {
        if (!token.empty()) {
            const size_t token_priority = token_priorities.at(token);
            
            if (hp_tokens.empty() || token_priority > highest_priority) {
                highest_priority = token_priority;
                hp_tokens = {token};
            } else if (token_priority == highest_priority) {
                hp_tokens.push_back(token);
            }
        }
    }

    if (hp_tokens.size() > 1) {
        throw_ambiguous_priority_err(hp_tokens, highest_priority);
    }

    return (hp_tokens.empty()) ? U"" : hp_tokens.front();
}

void lexer_generator::insert_rule_in_nfa(LexerAutomaton_t& nfa, const LexerAutomaton_t::StateID_t root_state, const TokenRegexRule& to_insert) {
    const LexerAutomaton_t::StateID_t leaf_state = insert_regex_ast_in_nfa(nfa, root_state, to_insert.regex_ast.get());
    nfa.get_state(leaf_state) = to_insert.name;
}

lexer_generator::LexerAutomaton_t::StateID_t lexer_generator::insert_regex_ast_in_nfa(
    LexerAutomaton_t& nfa, 
    const LexerAutomaton_t::StateID_t root_state, 
    const regex::RegexBase* const to_insert
) {
    if (dynamic_cast<const regex::RegexAlternation*>(to_insert)) {
        return insert_regex_branch_in_nfa(nfa, root_state, dynamic_cast<const regex::RegexAlternation*>(to_insert));
    } 
    if (dynamic_cast<const regex::RegexCharSet*>(to_insert)) {
        return insert_regex_char_set_in_nfa(nfa, root_state, dynamic_cast<const regex::RegexCharSet*>(to_insert));
    }
    if (dynamic_cast<const regex::RegexQuantifier*>(to_insert)) {
        return insert_regex_quantifier_in_nfa(nfa, root_state, dynamic_cast<const regex::RegexQuantifier*>(to_insert));
    }
    if (dynamic_cast<const regex::RegexSequence*>(to_insert)) {
        return insert_regex_sequence_in_nfa(nfa, root_state, dynamic_cast<const regex::RegexSequence*>(to_insert));
    }

    throw std::runtime_error("Tried to insert unknown regex type into NFA!");
}

lexer_generator::LexerAutomaton_t::StateID_t lexer_generator::insert_regex_branch_in_nfa(
    LexerAutomaton_t& nfa,
    const LexerAutomaton_t::StateID_t root_state,
    const regex::RegexAlternation* const to_insert
) {
    const LexerAutomaton_t::StateID_t collect_state = nfa.add_state(U"");

    for (const std::unique_ptr<regex::RegexBase>& branch : to_insert->get_branches()) {
        nfa.connect_states(insert_regex_ast_in_nfa(nfa, root_state, branch.get()), collect_state);
    }

    return collect_state;
}

lexer_generator::LexerAutomaton_t::StateID_t lexer_generator::insert_regex_char_set_in_nfa(
    LexerAutomaton_t& nfa,
     const LexerAutomaton_t::StateID_t root_state,
      const regex::RegexCharSet* const to_insert
) {
    const LexerAutomaton_t::StateID_t target_state = nfa.add_state(U"");

    nfa.connect_states(root_state, target_state, to_insert->get_range_set());

    return target_state;
}

lexer_generator::LexerAutomaton_t::StateID_t lexer_generator::insert_regex_quantifier_in_nfa(
    LexerAutomaton_t& nfa,
    const LexerAutomaton_t::StateID_t root_state,
    const regex::RegexQuantifier* const to_insert
) {
    LexerAutomaton_t::StateID_t curr_state = nfa.add_state(U"");
    nfa.connect_states(root_state, curr_state);

    for (size_t i = 0; i < to_insert->get_min(); i++) {
        curr_state = insert_regex_ast_in_nfa(nfa, curr_state, to_insert->get_operand().get());
    }

    if (to_insert->get_max() == regex::RegexQuantifier::INFINITE) {
        const LexerAutomaton_t::StateID_t repeat_state = insert_regex_ast_in_nfa(nfa, curr_state, to_insert->get_operand().get());
        nfa.connect_states(repeat_state, curr_state);

        return curr_state;
    }

    std::vector<LexerAutomaton_t::StateID_t> intermediate_states = {curr_state};

    for (size_t i = to_insert->get_min(); i < to_insert->get_max(); i++) {
        intermediate_states.push_back(insert_regex_ast_in_nfa(nfa, intermediate_states.back(), to_insert->get_operand().get()));
    }

    for (size_t i = 0; i < intermediate_states.size() - 1; i++) {
        nfa.connect_states(intermediate_states[i], intermediate_states.back());
    }

    return intermediate_states.back();
}

lexer_generator::LexerAutomaton_t::StateID_t lexer_generator::insert_regex_sequence_in_nfa(
    LexerAutomaton_t& nfa, 
    const LexerAutomaton_t::StateID_t root_state, 
    const regex::RegexSequence* const to_insert
) {
    LexerAutomaton_t::StateID_t curr_root = root_state;

    for (const std::unique_ptr<regex::RegexBase>& element : to_insert->get_elements()) {
        curr_root = insert_regex_ast_in_nfa(nfa, curr_root, element.get());
    }

    return curr_root;
}
