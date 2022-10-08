#include "code_gen_data.h"

code_gen::TokenInfos code_gen::conv_rules_to_generation_info(const std::vector<lexer_generator::TokenRegexRule>& to_convert) {
    TokenInfos generator_info{};

    for(const lexer_generator::TokenRegexRule& rule : to_convert) {
        if(rule.ignore_token) {
            generator_info.ignored_tokens.push_back(rule.name);
        } else {
            generator_info.tokens.push_back(rule.name);
        }
    }

    return generator_info;
}
