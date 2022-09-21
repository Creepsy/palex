#include <iostream>
#include <sstream>
#include <fstream>

#include "util/unicode.h"
#include "util/regex/RegexParser.h"

#include "lexer_generator/lexer_automaton.h"
#include "lexer_generator/LexerRuleParser.h"

int main() {  
    std::ifstream input{"../examples/Lexer.lrules"};
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);

    std::optional<lexer_generator::TokenRegexRule> token_rule_result;
    lexer_generator::LexerAutomaton_t output_nfa{};
    const lexer_generator::LexerAutomaton_t::StateID_t root_state = output_nfa.add_state(U"");
    
    do {
        token_rule_result = parser.parse_token_rule();
        if(token_rule_result.has_value()) {
            lexer_generator::insert_rule_in_nfa(output_nfa, root_state, token_rule_result.value());
            lexer_generator::TokenRegexRule& token_rule = token_rule_result.value();
            // std::cout << (token_rule.ignore_token ? "$" : "") << token_rule.token_name << " = " << token_rule.token_regex << std::endl;
        } 
    } while(token_rule_result.has_value());

    std::cout << output_nfa << std::endl;


    return 0;
}