#include <vector>
#include <functional>

#include "bootstrap/BootstrapLexer.h"

#include "input/PalexRuleParser.h"
#include "parser_generator/validation.h"

#include "util/palex_except.h"

#include "../test_utils.h"

struct TestCase {
    std::string input;
    bool should_fail;
};

int main() {
    const std::vector<TestCase> TEST_CASES = {
        {"$S = addition; addition = addition ADD number; number = INT;", false},
        {"$S = addition; addition#some_tag = addition ADD number; addition#another_tag = number; number = INT;", false},
        {"addition = addition ADD number; number = INT;", true},
        {"$S = addition; addition = addition ADD unknown; number = INT;", true},
        {"$S = addition; addition = addition ADD number; number = INT; number = INT;", true},
        {"$S = addition; add#ition = addition ADD number; addtion = number;", true}
    };

    for (const TestCase& test : TEST_CASES) {
        bootstrap::BootstrapLexer lexer(test.input.c_str());
        input::PalexRuleParser parser(
            std::bind(&bootstrap::BootstrapLexer::next_unignored_token, &lexer),
            std::bind(&bootstrap::BootstrapLexer::get_token, &lexer)
        ); 

        if (test.should_fail) {
            TEST_EXCEPT(parser_generator::validate_productions(parser.parse_all_productions()), palex_except::ValidationError);
        } else {
            parser_generator::validate_productions(parser.parse_all_productions());
        }
    }

    return 0;
}