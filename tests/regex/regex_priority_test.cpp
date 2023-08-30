#include <vector>
#include <string_view>

#include "regex/RegexParser.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::pair<std::string_view, size_t>> TEST_CASES = {
        {"int", 6},
        {"integer", 14},
        {"[a-z]", 1},
        {"[a-z]b", 3},
        {"bc|def|[a-z]", 1},
        {"ib*", 2},
        {"(abc)+", 6},
        {"(abc){2,10}", 12}
    };

    for (const std::pair<std::string_view, size_t>& test : TEST_CASES) {
        TEST_TRUE(regex::RegexParser(test.first).parse_regex()->get_priority() == test.second)
    }
    
    return 0;
}