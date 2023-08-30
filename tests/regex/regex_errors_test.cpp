#include <vector>
#include <string_view>

#include "regex/RegexParser.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::string_view> TEST_CASES = { 
        ")", "(", "[",
        "\\ca", "\\uAfF", "\\u24Ga", "\\u{24fffag}", "\\u{ffffffffff}",
        "a{3,", "a{3,", "a{}", "a{", "a{3, 5}"
    };

    for (const std::string_view test : TEST_CASES) {
        TEST_EXCEPT(regex::RegexParser(test).parse_regex(), palex_except::ParserError)
    }

    return 0;
}