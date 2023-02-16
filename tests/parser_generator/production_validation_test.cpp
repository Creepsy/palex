#include <vector>
#include <sstream>

#include "parser_generator/ParserProductionLexer.h"
#include "parser_generator/ParserProductionParser.h"
#include "parser_generator/validation.h"

#include "util/palex_except.h"

#include "../test_utils.h"

struct TestCase {
    std::string input;
    bool should_fail;
};

int main() {
    const std::vector<TestCase> TEST_CASES = {
        {"addition = addition <ADD> number; number = <INT>;", false},
        {"addition = addition <ADD> unknown; number = <INT>;", true},
        {"addition = addition <ADD> number; number = <INT>; number = <INT>;", true},
    };

    for (const TestCase& test : TEST_CASES) {
        std::stringstream input(test.input);
        parser_generator::ParserProductionLexer lexer(input);
        parser_generator::ParserProductionParser parser(lexer);

        if (test.should_fail) {
            TEST_EXCEPT(parser_generator::validate_productions(parser.parse_all_productions()), palex_except::ValidationError);
        } else {
            parser_generator::validate_productions(parser.parse_all_productions());
        }
    }

    return 0;
}