#include <iostream>
#include <sstream>
#include <numeric>
#include <fstream>
#include <functional>

#include "util/unicode.h"
#include "regex/RegexParser.h"

#include "lexer_generator/lexer_automaton.h"
#include "lexer_generator/LexerRuleParser.h"
#include "lexer_generator/validation.h"

int main() {  
    using namespace std::placeholders;

    std::ifstream input{"../examples/Lexer.lrules"};
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);

    lexer_generator::LexerAutomaton_t lexer_nfa{};
    const lexer_generator::LexerAutomaton_t::StateID_t root_state = lexer_nfa.add_state(U"");
    
    std::vector<lexer_generator::TokenRegexRule> lexer_rules = parser.parse_all_rules();

    lexer_generator::validate_rules(lexer_rules);

    for(const lexer_generator::TokenRegexRule& rule : lexer_rules) {
        lexer_generator::insert_rule_in_nfa(lexer_nfa, root_state, rule);
    }

    std::map<std::u32string, size_t> token_priorities = lexer_generator::get_token_priorities(lexer_rules);
    auto merge_states = std::bind(lexer_generator::merge_states_by_priority, token_priorities, _1);

    lexer_generator::LexerAutomaton_t lexer_dfa = lexer_nfa.convert_to_dfa<std::u32string>(
        root_state,
        merge_states,
        lexer_generator::resolve_connection_collisions
    );

    std::cout << lexer_dfa << std::endl;

    return 0;
}