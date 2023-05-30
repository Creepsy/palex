#include <sstream>
#include <cstddef>
#include <stdexcept>

#include "input/PalexRuleLexer.h"
#include "input/PalexRuleParser.h"

#include "parser_generator/validation.h"
#include "parser_generator/production_definition.h"

#include "parser_generator/shift_reduce_parsers/parser_table_generation.h"
#include "parser_generator/shift_reduce_parsers/state_lookahead.h"
#include "parser_generator/shift_reduce_parsers/parser_state_comparators.h"

#include "util/stream_format.h"

#include "../test_utils.h"

struct TestCase {
    std::vector<std::string> input_tokens;
    std::string formatted_output;
};

int main() {
    using parser_generator::shift_reduce_parsers::DebugParseTree;

    constexpr size_t LOOKAHEAD = 0;
    const std::vector<TestCase> TEST_CASES = {
        TestCase{
            std::vector<std::string>{"INT", "ADD", "INT"},
            "$S\n"
            "    addition\n"
            "        addition\n"
            "            multiplication\n"
            "                number\n"
            "                    INT\n"
            "        ADD\n"
            "        multiplication\n"
            "            number\n"
            "                INT\n"
        },
        TestCase{
            std::vector<std::string>{"INT", "ADD", "INT", "MUL", "INT"},
            "$S\n"
            "    addition\n"
            "        addition\n"
            "            multiplication\n"
            "                number\n"
            "                    INT\n"
            "        ADD\n"
            "        multiplication\n"
            "            multiplication\n"
            "                number\n"
            "                    INT\n"
            "            MUL\n"
            "            number\n"
            "                INT\n"
        },
        TestCase{
            std::vector<std::string>{"INT"},
            "$S\n"
            "    addition\n"
            "        multiplication\n"
            "            number\n"
            "                INT\n"
        }
    };
    std::stringstream input {
        "$S = addition;\n"
        "addition = addition ADD multiplication;\n"
        "addition = multiplication;\n"
        "multiplication = multiplication MUL number;\n"
        "multiplication = number;\n"
        "number = INT;\n"
    };
    input::PalexRuleLexer lexer(input);
    input::PalexRuleParser parser(lexer);
    const std::vector<parser_generator::Production> productions = parser.parse_all_rules().productions;
    parser_generator::validate_productions(productions);
    const std::set<parser_generator::Production> productions_set(productions.begin(), productions.end());
    parser_generator::shift_reduce_parsers::FirstSet_t first_set = parser_generator::shift_reduce_parsers::generate_first_set(
        productions_set,
        LOOKAHEAD
    );
    parser_generator::shift_reduce_parsers::ParserTable parser_table = parser_generator::shift_reduce_parsers::ParserTable::generate(
            productions_set,
            parser_generator::shift_reduce_parsers::lalr_state_compare,
            LOOKAHEAD
    );
    
    for (const TestCase& test_case : TEST_CASES) {
        std::stringstream output;
        sfmt::IndentationStreamBuffer indentation_stream_buffer(output);
        output << parser_table.debug_parse(test_case.input_tokens);
        std::cout << output.str();
        std::cout << test_case.formatted_output;
        TEST_TRUE(output.str() == test_case.formatted_output)
    }
    return 0;
}