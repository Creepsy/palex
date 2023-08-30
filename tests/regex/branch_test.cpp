#include <vector>
#include <memory>
#include <string_view>

#include "regex/RegexParser.h"

#include "../test_utils.h"

int main() {
    const std::vector<std::pair<std::string_view, size_t>> TEST_CASES = {
        {"a|b", 2},
        {"a|b|c|d", 4},
        {"[abc]|ba|cddd|d", 4}
    };

    for (const auto& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.first).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexAlternation*>(base_ast.get()))
        TEST_TRUE(dynamic_cast<regex::RegexAlternation*>(base_ast.get())->get_branches().size() == test.second)
    }
    
    return 0;
}