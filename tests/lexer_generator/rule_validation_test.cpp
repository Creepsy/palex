#include <sstream>
#include <string>
#include <vector>

#include "input/PalexRuleLexer.h"
#include "input/PalexRuleParser.h"
#include "lexer_generator/validation.h"

#include "util/palex_except.h"

#include "../test_utils.h"

struct TestCase {
    std::string input;

    bool should_fail;
};


int main() {
    const std::vector<TestCase> TEST_CASES = {
        {"IDENTIFIER = \"a\"; INTEGER = \"a\"; INT = \"a\"; A123WEIRD_IDENT = \"a\";", false},
        {"UNDEFINED = \"a\";", true},
        {"END_OF_FILE = \"a\";", true},
        {"DUP_IDENT = \"a\"; DUP_IDENT = \"a\";", true}
    };

    for (const TestCase& test : TEST_CASES) {
        std::stringstream input(test.input);
        input::PalexRuleLexer lexer(input);
        input::PalexRuleParser parser(lexer);
        
        std::vector<lexer_generator::TokenDefinition> rules = parser.parse_all_token_definitions();

        if (test.should_fail) {
            TEST_EXCEPT(lexer_generator::validate_rules(rules), palex_except::ValidationError);
        } else {
            lexer_generator::validate_rules(rules);
        }
    }


    return 0;
}