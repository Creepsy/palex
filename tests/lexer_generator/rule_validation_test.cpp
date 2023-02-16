#include <sstream>
#include <string>
#include <vector>

#include "lexer_generator/LexerRuleLexer.h"
#include "lexer_generator/LexerRuleParser.h"
#include "lexer_generator/validation.h"

#include "util/palex_except.h"

#include "../test_utils.h"

struct TestCase {
    std::string input;

    bool should_fail;
};


int main() {
    const std::vector<TestCase> TEST_CASES = {
        {"IDENTIFIER = \"a\"; INTEGER = \"a\"; _INT = \"a\"; _123WEIRD_IDENT = \"a\";", false},
        {"UNDEFINED = \"a\";", true},
        {"END_OF_FILE = \"a\";", true},
        {"DUP_IDENT = \"a\"; DUP_IDENT = \"a\";", true}
    };

    for (const TestCase& test : TEST_CASES) {
        std::stringstream input(test.input);
        lexer_generator::LexerRuleLexer lexer(input);
        lexer_generator::LexerRuleParser parser(lexer);
        
        std::vector<lexer_generator::TokenRegexRule> rules = parser.parse_all_rules();

        if (test.should_fail) {
            TEST_EXCEPT(lexer_generator::validate_rules(rules), palex_except::ValidationError);
        } else {
            lexer_generator::validate_rules(rules);
        }
    }


    return 0;
}