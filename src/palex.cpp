#include <iostream>
#include <sstream>
#include <fstream>

#include "util/unicode.h"
#include "util/regex/RegexParser.h"

#include "lexer_generator/LexerRuleParser.h"

int main() {  
    std::ifstream input{"../examples/Lexer.lrules"};
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);

    std::optional<lexer_generator::TokenRegexRule> token_rule_result;

    do {
        token_rule_result = parser.parse_token_rule();
        if(token_rule_result.has_value()) {
            lexer_generator::TokenRegexRule& token_rule = token_rule_result.value();
            std::cout << (token_rule.ignore_token ? "$" : "") << token_rule.token_name << " = " << token_rule.token_regex << std::endl;
        } 
    } while(token_rule_result.has_value());

    std::cout << std::endl;
    
    regex::RegexParser regex_parser{U"(#|0x)?([\\da-fA-F]{2}){1,3}"};
    regex_parser.parse_regex()->debug(std::cout);

    return 0;
}