#include <iostream>
#include <sstream>
#include <numeric>
#include <fstream>

#include "util/unicode.h"
#include "util/regex/RegexParser.h"

#include "lexer_generator/lexer_automaton.h"
#include "lexer_generator/LexerRuleParser.h"
#include "lexer_generator/validation.h"

int main() {  
    std::ifstream input{"../examples/Lexer.lrules"};
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);

    std::optional<lexer_generator::TokenRegexRule> token_rule_result;
    lexer_generator::LexerAutomaton_t lexer_nfa{};
    const lexer_generator::LexerAutomaton_t::StateID_t root_state = lexer_nfa.add_state(U"");
    
    std::vector<lexer_generator::TokenRegexRule> lexer_rules;

    do {
        token_rule_result = parser.parse_token_rule();
        if(token_rule_result.has_value()) {
            lexer_rules.push_back(token_rule_result.value());
            lexer_generator::TokenRegexRule& token_rule = token_rule_result.value();
        } 
    } while(token_rule_result.has_value());

    lexer_generator::validate_rules(lexer_rules);

    for(const lexer_generator::TokenRegexRule& rule : lexer_rules) {
        lexer_generator::insert_rule_in_nfa(lexer_nfa, root_state, rule);
    }

    const auto merge_states_debug = [](const std::vector<std::u32string>& to_merge) -> std::u32string {
        std::u32string output = U"";

        for(const std::u32string& state_value : to_merge) {
            if(!state_value.empty()) {
                if(!output.empty()) output += U", ";
                output += state_value;
            }
        }

        return output;
    };

    lexer_generator::LexerAutomaton_t lexer_dfa = lexer_nfa.convert_to_dfa<std::u32string>(
        root_state,
        merge_states_debug,
        lexer_generator::resolve_connection_collisions
    );

    std::cout << lexer_dfa << std::endl;


    return 0;
}