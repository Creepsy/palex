#include <functional>
#include <variant>

#include "bootstrap/BootstrapLexer.h"

#include "input/PalexRuleParser.h"

#include "parser_generator/production_definition.h"

#include "../test_utils.h"

int main() {
    const char* input = 
        "!WSPACE = \"\\s+\";\n"
        "!<7>ANOTHER_1 = \"a\";\n"
        "$S = some_prod;\n"
        "some_prod = ANOTHER_1 some_prod -> error;\n"
        "some_prod#production_tag = -> error \"some error\";\n"
    ;
    bootstrap::BootstrapLexer lexer(input);
    input::PalexRuleParser parser(
        std::bind(&bootstrap::BootstrapLexer::next_unignored_token, &lexer),
        std::bind(&bootstrap::BootstrapLexer::get_token, &lexer)
    );
    const input::PalexRules palex_rules = parser.parse_all_rules();
    
    TEST_TRUE(palex_rules.token_definitions.size() == 2)
    TEST_TRUE(palex_rules.token_definitions[0].ignore_token)
    TEST_TRUE(palex_rules.token_definitions[1].ignore_token)
    TEST_TRUE(palex_rules.token_definitions[0].name == "WSPACE")
    TEST_TRUE(palex_rules.token_definitions[1].name == "ANOTHER_1")
    TEST_TRUE(palex_rules.token_definitions[0].priority == palex_rules.token_definitions[0].token_regex->get_priority())
    TEST_TRUE(palex_rules.token_definitions[1].priority == 7)

    TEST_TRUE(palex_rules.productions.size() == 3)
    TEST_TRUE(palex_rules.productions[0].is_entry())
    TEST_TRUE(palex_rules.productions[0].symbols.size() == 1)
    TEST_TRUE(std::holds_alternative<parser_generator::ReductionResult_t>(palex_rules.productions[0].result))
    TEST_TRUE(palex_rules.productions[1].name == "some_prod")
    TEST_TRUE(palex_rules.productions[1].symbols.size() == 2)
    TEST_TRUE(std::holds_alternative<parser_generator::ErrorResult_t>(palex_rules.productions[1].result))
    TEST_TRUE(std::get<parser_generator::ErrorResult_t>(palex_rules.productions[1].result).has_value() == false)
    TEST_TRUE(palex_rules.productions[2].name == "some_prod")
    TEST_TRUE(palex_rules.productions[2].tag == "production_tag")
    TEST_TRUE(palex_rules.productions[2].symbols.empty())
    TEST_TRUE(std::holds_alternative<parser_generator::ErrorResult_t>(palex_rules.productions[2].result))
    TEST_TRUE(std::get<parser_generator::ErrorResult_t>(palex_rules.productions[2].result).has_value() == true)
    TEST_TRUE(std::get<parser_generator::ErrorResult_t>(palex_rules.productions[2].result).value() == "some error")
    return 0;
}