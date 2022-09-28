#include "lexer_automaton.h"

#include <cassert>
#include <vector>

#include "util/regex/RegexParser.h"

#include "util/unicode.h"

void lexer_generator::resolve_connection_collisions(
    const LexerAutomaton_t::Connection& to_add, 
    std::vector<std::pair<regex::CharRangeSet, std::set<LexerAutomaton_t::StateID_t>>>& dfa_connections
) {
    for(auto iter = dfa_connections.begin(); iter != dfa_connections.end(); iter++) {
        if(to_add.value.value() == (*iter).first) {
            (*iter).second.insert(to_add.target);
        } else {
            regex::CharRangeSet intersection = to_add.value.value().get_intersection((*iter).first);
            if(intersection.empty()) continue;

            if(intersection != (*iter).first) {
                iter = dfa_connections.insert(iter, std::make_pair(intersection, (*iter).second));
            }
            (*iter).second.insert(to_add.target);

            regex::CharRangeSet non_intersecting = to_add.value.value() - intersection;
            if(!non_intersecting.empty()) {
                iter = dfa_connections.insert(iter, std::make_pair(non_intersecting, std::set<LexerAutomaton_t::StateID_t>{to_add.target}));
            }
        }
        return;
    }

    dfa_connections.push_back(std::make_pair(to_add.value.value(), std::set<size_t>{to_add.target}));
}

std::vector<std::u32string> lexer_generator::merge_states_to_vector(const std::vector<std::u32string>& to_merge) {
    return to_merge;
}

void lexer_generator::insert_rule_in_nfa(LexerAutomaton_t& nfa, const LexerAutomaton_t::StateID_t root_state, const TokenRegexRule& to_insert) {
    std::unique_ptr<regex::RegexBase> regex_ast = regex::RegexParser(to_insert.token_regex).parse_regex();
    assert(("Regex is null! Please create an issue on github containing the used input!", regex_ast));

    const LexerAutomaton_t::StateID_t leaf_state = insert_regex_ast_in_nfa(nfa, root_state, regex_ast.get());
    nfa.get_state(leaf_state) = to_insert.token_name;
}

lexer_generator::LexerAutomaton_t::StateID_t lexer_generator::insert_regex_ast_in_nfa(
    LexerAutomaton_t& nfa, 
    const LexerAutomaton_t::StateID_t root_state, 
    const regex::RegexBase* const to_insert
) {
    if(dynamic_cast<const regex::RegexBranch*>(to_insert)) {
        return insert_regex_branch_in_nfa(nfa, root_state, dynamic_cast<const regex::RegexBranch*>(to_insert));
    } else if(dynamic_cast<const regex::RegexCharSet*>(to_insert)) {
        return insert_regex_char_set_in_nfa(nfa, root_state, dynamic_cast<const regex::RegexCharSet*>(to_insert));
    } else if(dynamic_cast<const regex::RegexQuantifier*>(to_insert)) {
        return insert_regex_quantifier_in_nfa(nfa, root_state, dynamic_cast<const regex::RegexQuantifier*>(to_insert));
    } else if(dynamic_cast<const regex::RegexSequence*>(to_insert)) {
        return insert_regex_sequence_in_nfa(nfa, root_state, dynamic_cast<const regex::RegexSequence*>(to_insert));
    } else {
        throw std::runtime_error("Tried to insert unknown regex type into NFA!");
    }
}

lexer_generator::LexerAutomaton_t::StateID_t lexer_generator::insert_regex_branch_in_nfa(
    LexerAutomaton_t& nfa,
    const LexerAutomaton_t::StateID_t root_state,
    const regex::RegexBranch* const to_insert
) {
    const LexerAutomaton_t::StateID_t collect_state = nfa.add_state(U"");

    for(const std::unique_ptr<regex::RegexBase>& possibility : to_insert->get_possibilities()) {
        nfa.connect_states(insert_regex_ast_in_nfa(nfa, root_state, possibility.get()), collect_state);
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

    for(size_t i = 0; i < to_insert->get_min(); i++) {
        curr_state = insert_regex_ast_in_nfa(nfa, curr_state, to_insert->get_operand().get());
    }

    if(to_insert->get_max() == regex::RegexQuantifier::INFINITE) {
        const LexerAutomaton_t::StateID_t repeat_state = insert_regex_ast_in_nfa(nfa, curr_state, to_insert->get_operand().get());
        nfa.connect_states(repeat_state, curr_state);

        return curr_state;
    } else {
        std::vector<LexerAutomaton_t::StateID_t> intermediate_states = {curr_state};

        for(size_t i = to_insert->get_min(); i < to_insert->get_max(); i++) {
            intermediate_states.push_back(insert_regex_ast_in_nfa(nfa, intermediate_states.back(), to_insert->get_operand().get()));
        }

        for(size_t i = 0; i < intermediate_states.size() - 1; i++) {
            nfa.connect_states(intermediate_states[i], intermediate_states.back());
        }

        return intermediate_states.back();
    }
}

lexer_generator::LexerAutomaton_t::StateID_t lexer_generator::insert_regex_sequence_in_nfa(
    LexerAutomaton_t& nfa, 
    const LexerAutomaton_t::StateID_t root_state, 
    const regex::RegexSequence* const to_insert
) {
    LexerAutomaton_t::StateID_t curr_root = root_state;

    for(const std::unique_ptr<regex::RegexBase>& element : to_insert->get_elements()) {
        curr_root = insert_regex_ast_in_nfa(nfa, curr_root, element.get());
    }

    return curr_root;
}
