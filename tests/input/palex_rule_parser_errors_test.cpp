#include <vector>
#include <string>
#include <functional>

#include "bootstrap/BootstrapLexer.h"

#include "input/PalexRuleParser.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::string> faulty_grammars = {
        "production = ; TOKEN = \"a\";",    // token after production
        "production = A",                   // missing ; at end of production
        "AVv = \"f\";"                      // invalid token
    };

    for (const std::string& faulty_grammar : faulty_grammars) {
        bootstrap::BootstrapLexer lexer(faulty_grammar.c_str());
        input::PalexRuleParser parser(
            std::bind(&bootstrap::BootstrapLexer::next_unignored_token, &lexer),
            std::bind(&bootstrap::BootstrapLexer::get_token, &lexer)
        );  
        std::cout << faulty_grammar << std::endl;
        TEST_EXCEPT(parser.parse_all_rules(), palex_except::ParserError)
    }
    return 0;
}