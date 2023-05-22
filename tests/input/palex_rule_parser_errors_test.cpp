#include <vector>
#include <string>
#include <sstream>

#include "input/PalexRuleParser.h"
#include "input/PalexRuleLexer.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::string> faulty_grammars = {
        "production = ; TOKEN = \"a\";",    // token after production
        "production = A",                   // missing ; at end of production
        "AVv = \"f\";"                      // invalid token
    };

    for (const std::string& faulty_grammar : faulty_grammars) {
        std::stringstream input(faulty_grammar);
        input::PalexRuleLexer lexer(input);
        input::PalexRuleParser parser(lexer);
        std::cout << faulty_grammar << std::endl;
        TEST_EXCEPT(parser.parse_all_rules(), palex_except::ParserError)
    }
    return 0;
}