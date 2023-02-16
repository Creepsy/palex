#include <vector>
#include <string>

#include "regex/RegexParser.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::pair<std::u32string, size_t>> TEST_CASES = {
        {U"int", 6},
        {U"integer", 14},
        {U"[a-z]", 1},
        {U"[a-z]b", 3},
        {U"bc|def|[a-z]", 1},
        {U"ib*", 2},
        {U"(abc)+", 6},
        {U"(abc){2,10}", 12}
    };

    for (const std::pair<std::u32string, size_t>& test : TEST_CASES) {
        TEST_TRUE(regex::RegexParser(test.first).parse_regex()->get_priority() == test.second)
    }
    
    return 0;
}