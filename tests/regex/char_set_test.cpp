#include <vector>
#include <string_view>

#include "regex/character_classes.h"
#include "regex/regex_ast.h"
#include "regex/RegexParser.h"

#include "../test_utils.h"

struct TestCase {
    std::string_view input;

    std::vector<regex::CharRange> result;
    bool negated;
};

int main() {
    const std::vector<TestCase> TEST_CASES = {
        {"\\d", regex::character_classes::DIGIT_CLASS, false},
        {"\\D", regex::character_classes::NON_DIGIT_CLASS, false},
        {"\\w", regex::character_classes::WORD_CLASS, false},
        {"\\W", regex::character_classes::NON_WORD_CLASS, false},
        {"\\s", regex::character_classes::WHITESPACE_CLASS, false},
        {"\\S", regex::character_classes::NON_WHITESPACE_CLASS, false},
        {".", regex::character_classes::DOT_CLASS, false},
        {"[]", {}, false},
        {"[^]", {regex::CharRange{0, utf8::LAST_UNICODE_CHAR}}, true},
        {"[abcd]", {regex::CharRange{'a', 'd'}}, false},
        {"[a-z]", {regex::CharRange{'a', 'z'}}, false},
        {"[^a-b\\w]", {
            regex::CharRange{0, '0' - 1},
            regex::CharRange{'9' + 1, 'A' - 1},
            regex::CharRange{'Z' + 1, '_' - 1},
            regex::CharRange{'_' + 1, 'a' - 1}, 
            regex::CharRange{'z' + 1, utf8::LAST_UNICODE_CHAR}
        }, true},
        {"[a-dc-fe-j]", {regex::CharRange{'a', 'j'}}, false},
        {"[A-Za-z]", {regex::CharRange{'A', 'Z'}, regex::CharRange{'a', 'z'}}, false},
        {"[\\W\\w]", {regex::CharRange{0, utf8::LAST_UNICODE_CHAR}}, false},
        {"[\\w\\W]", {regex::CharRange{0, utf8::LAST_UNICODE_CHAR}}, false},
        {"[.]", {regex::CharRange{'.'}}, false},
        {"[a-]", {regex::CharRange{'-'}, regex::CharRange{'a'}}, false},
        {"[---]", {regex::CharRange{'-'}}, false},
        {"[-a]", {regex::CharRange{'-'}, regex::CharRange{'a'}}, false},
        {"a", {regex::CharRange{'a'}}, false},
        {"\\n", {regex::CharRange{'\n'}}, false},
        {"\\u55ff", {regex::CharRange{0x55ff}}, false},
        {"\\u{13aEF}", {regex::CharRange{0x13aef}}, false},
        {"\\cB", {regex::CharRange{2}}, false}
    };

    for (const TestCase& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.input).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexCharSet*>(base_ast.get()))
        regex::RegexCharSet* char_set = dynamic_cast<regex::RegexCharSet*>(base_ast.get());
        TEST_TRUE(test.negated == char_set->is_negated())        
        TEST_TRUE(
            std::equal(
                test.result.begin(),
                test.result.end(),
                char_set->get_range_set().get_ranges().begin(),
                char_set->get_range_set().get_ranges().end()
            )
        )
    }

    return 0;
}