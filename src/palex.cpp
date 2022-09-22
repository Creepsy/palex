#include <iostream>
#include <sstream>
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
    lexer_generator::LexerAutomaton_t output_nfa{};
    const lexer_generator::LexerAutomaton_t::StateID_t root_state = output_nfa.add_state(U"");
    
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
        lexer_generator::insert_rule_in_nfa(output_nfa, root_state, rule);
    }

    std::cout << output_nfa << std::endl;


    return 0;
}