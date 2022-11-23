#include <vector>

#include "regex/RegexParser.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::u32string> TEST_CASES = {
        U")", U"()", U"(", U"[",
        U"", U"a|", U"|", U"|a",
        U"\\ca", U"\\uAfF", U"\\u24Ga", U"\\u{24fffag}", U"\\u{ffffffffff}",
        U"a{3,", U"a{3,", U"a{}", U"a{", U"a{3, 5}"
    };

    for(const std::u32string& test : TEST_CASES) {
        TEST_EXCEPT(regex::RegexParser(test).parse_regex(), palex_except::ParserError)
    }

    return 0;
}