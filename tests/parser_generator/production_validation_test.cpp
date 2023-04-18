#include <vector>
#include <sstream>

#include "parser_generator/lang/ParserProductionLexer.h"
#include "parser_generator/lang/ParserProductionParser.h"
#include "parser_generator/lang/validation.h"

#include "util/palex_except.h"

#include "../test_utils.h"

struct TestCase {
    std::string input;
    bool should_fail;
};

int main() {
    const std::vector<TestCase> TEST_CASES = {
        {"$S = addition; addition = addition <ADD> number; number = <INT>;", false},
        {"addition = addition <ADD> number; number = <INT>;", true},
        {"$S = addition; addition = addition <ADD> unknown; number = <INT>;", true},
        {"$S = addition; addition = addition <ADD> number; number = <INT>; number = <INT>;", true}
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