#include <vector>
#include <memory>
#include <string_view>

#include "regex/RegexParser.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::pair<std::string_view, size_t>> TEST_CASES = {
        {"abbb", 4},
        {"a[abc][avb](abg)", 4},
        {"(abb)", 3},
        {"(abb[aa])([f])", 2},
        {"(abb)([f])", 2},
        {"(aab)?a", 2}
    };

    for (const auto& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.first).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexSequence*>(base_ast.get()))
        TEST_TRUE(dynamic_cast<regex::RegexSequence*>(base_ast.get())->get_elements().size() == test.second)
    }

    return 0;
}