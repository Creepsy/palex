#include <vector>
#include <string_view>
#include <memory>

#include "regex/RegexParser.h"

#include "../test_utils.h"

struct TestCase {
    std::string_view input;

    size_t min;
    size_t max;
};


int main() {
    const std::vector<TestCase> TEST_CASES = {
        {"a+", 1, regex::RegexQuantifier::INFINITE},
        {"a*", 0, regex::RegexQuantifier::INFINITE},
        {"a?", 0, 1},
        {"a{7}", 7, 7},
        {"a{3,}", 3, regex::RegexQuantifier::INFINITE},
        {"a{10,15}", 10, 15}
    };

    for (const TestCase& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.input).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexQuantifier*>(base_ast.get()))
        regex::RegexQuantifier* quantifier = dynamic_cast<regex::RegexQuantifier*>(base_ast.get());
        TEST_TRUE(quantifier->get_min() == test.min)
        TEST_TRUE(quantifier->get_max() == test.max)
    }

    return 0;
}