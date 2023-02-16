#include <vector>
#include <memory>

#include "regex/RegexParser.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::pair<std::u32string, size_t>> TEST_CASES = {
        {U"abbb", 4},
        {U"a[abc][avb](abg)", 4},
        {U"(abb)", 3},
        {U"(abb[aa])([f])", 2},
        {U"(abb)([f])", 2},
        {U"(aab)?a", 2}
    };

    for (const auto& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.first).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexSequence*>(base_ast.get()))
        TEST_TRUE(dynamic_cast<regex::RegexSequence*>(base_ast.get())->get_elements().size() == test.second)
    }

    return 0;
}