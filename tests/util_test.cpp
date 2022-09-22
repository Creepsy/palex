#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstddef>
#include <utility>

#include "util/unicode.h"
#include "util/Automaton.h"
#include "util/palex_except.h"

#include "util/regex/RegexParser.h"
#include "util/regex/character_classes.h"

#include "TestReport.h"
#include "test_utils.h"

bool test_unicode_input();
bool test_unicode_output();

bool test_automaton_add_states();
bool test_automaton_add_connections();

bool test_regex_errors();
bool test_regex_char_set();
bool test_regex_branch();
bool test_regex_sequence();
bool test_regex_quantifier();

int main() {
    tests::TestReport report;

    report.add_test("unicode_input", test_unicode_input);
    report.add_test("unicode_output", test_unicode_output);
    
    report.add_test("automaton_add_states", test_automaton_add_states);
    report.add_test("automaton_add_connections", test_automaton_add_connections);
    
    report.add_test("regex_errors", test_regex_errors);
    report.add_test("regex_char_set", test_regex_char_set);
    report.add_test("regex_branch", test_regex_branch);
    report.add_test("regex_sequence", test_regex_sequence);
    report.add_test("regex_quantifier", test_regex_quantifier);

    report.run();

    return report.report();
}

bool test_unicode_input() {
    std::stringstream input("aШᢶ𐅄");

    TEST_TRUE(unicode::get_utf8(input) == 97)       // 1 byte char
    TEST_TRUE(unicode::get_utf8(input) == 1064)     // 2 byte char
    TEST_TRUE(unicode::get_utf8(input) == 6326)     // 3 byte char
    TEST_TRUE(unicode::get_utf8(input) == 65860)    // 4 byte char


    return true;   
}

bool test_unicode_output() {
    std::u32string to_print = {
        97,     // 1 byte char
        1064,   // 2 byte char
        6326,   // 3 byte char
        65860   // 4 byte char
    };

    std::stringstream output;
    output << to_print;

    TEST_TRUE(output.str() == "aШᢶ𐅄")

    return true;
}

bool test_automaton_add_states() {
    sm::Automaton<int, int> test_automaton;

    TEST_TRUE(test_automaton.add_state(10) == 0)
    TEST_TRUE(test_automaton.add_state(11) == 1)

    return true;
}

bool test_automaton_add_connections() {
    sm::Automaton<int, int> test_automaton;
    
    sm::Automaton<int, int>::StateID_t first = test_automaton.add_state(10);
    sm::Automaton<int, int>::StateID_t second = test_automaton.add_state(11);

    TEST_EXCEPT(test_automaton.connect_states(1, 2), std::out_of_range)
    TEST_EXCEPT(test_automaton.connect_states(2, 0, 10), std::out_of_range)

    TEST_TRUE(test_automaton.connect_states(0, 1) == 0)
    TEST_TRUE(test_automaton.connect_states(1, 0, 10) == 1)

    return true;
}

bool test_regex_errors() {
    const std::vector<std::u32string> TEST_CASES = {
        U")", U"()", U"(", U"[",
        U"", U"a|", U"|", U"|a",
        U"\\ca", U"\\uAfF", U"\\u24Ga", U"\\u{24fffag}", U"\\u{ffffffffff}",
        U"a{3,", U"a{3,", U"a{}", U"a{", U"a{3, 5}"
    };

    for(const std::u32string& test : TEST_CASES) {
        TEST_EXCEPT(regex::RegexParser(test).parse_regex(), palex_except::ParserError)
    }

    return true;
}

bool test_regex_char_set() {
    struct TestCase {
        std::u32string input;

        std::vector<regex::CharRange> result;
        bool negated;
    };

    const std::vector<TestCase> TEST_CASES = {
        {U"\\d", regex::character_classes::DIGIT_CLASS, false},
        {U"\\D", regex::character_classes::NON_DIGIT_CLASS, false},
        {U"\\w", regex::character_classes::WORD_CLASS, false},
        {U"\\W", regex::character_classes::NON_WORD_CLASS, false},
        {U"\\s", regex::character_classes::WHITESPACE_CLASS, false},
        {U"\\S", regex::character_classes::NON_WHITESPACE_CLASS, false},
        {U".", regex::character_classes::DOT_CLASS, false},
        {U"[]", {}, false},
        {U"[^]", {regex::CharRange{0, unicode::LAST_UNICODE_CHAR}}, true},
        {U"[abcd]", {regex::CharRange{'a', 'd'}}, false},
        {U"[a-z]", {regex::CharRange{'a', 'z'}}, false},
        {U"[^a-b\\w]", {
            regex::CharRange{0, '0' - 1},
            regex::CharRange{'9' + 1, 'A' - 1},
            regex::CharRange{'Z' + 1, '_' - 1},
            regex::CharRange{'_' + 1, 'a' - 1}, 
            regex::CharRange{'z' + 1, unicode::LAST_UNICODE_CHAR}
        }, true},
        {U"[a-dc-fe-j]", {regex::CharRange{'a', 'j'}}, false},
        {U"[A-Za-z]", {regex::CharRange{'A', 'Z'}, regex::CharRange{'a', 'z'}}, false},
        {U"[\\W\\w]", {regex::CharRange{0, unicode::LAST_UNICODE_CHAR}}, false},
        {U"[\\w\\W]", {regex::CharRange{0, unicode::LAST_UNICODE_CHAR}}, false},
        {U"[.]", {regex::CharRange{'.'}}, false},
        {U"[a-]", {regex::CharRange{'-'}, regex::CharRange{'a'}}, false},
        {U"[---]", {regex::CharRange{'-'}}, false},
        {U"[-a]", {regex::CharRange{'-'}, regex::CharRange{'a'}}, false},
        {U"a", {regex::CharRange{'a'}}, false},
        {U"\\n", {regex::CharRange{'\n'}}, false},
        {U"\\u55ff", {regex::CharRange{0x55ff}}, false},
        {U"\\u{13aEF}", {regex::CharRange{0x13aef}}, false},
        {U"\\cB", {regex::CharRange{2}}, false}
    };

    for(const TestCase& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.input).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexCharSet*>(base_ast.get()))
        regex::RegexCharSet* char_set = dynamic_cast<regex::RegexCharSet*>(base_ast.get());
        TEST_TRUE(test.negated == char_set->is_negated())        
        TEST_TRUE(
            std::equal(
                test.result.begin(),
                test.result.end(),
                char_set->get_characters().begin(),
                char_set->get_characters().end()
            )
        )
    }

    return true;
}

bool test_regex_branch() {
    const std::vector<std::pair<std::u32string, size_t>> TEST_CASES = {
        {U"a|b", 2},
        {U"a|b|c|d", 4},
        {U"[abc]|ba|cddd|d", 4}
    };

    for(const auto& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.first).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexBranch*>(base_ast.get()))
        TEST_TRUE(dynamic_cast<regex::RegexBranch*>(base_ast.get())->get_possibilities().size() == test.second)
    }
    
    return true;
}

bool test_regex_sequence() {
    const std::vector<std::pair<std::u32string, size_t>> TEST_CASES = {
        {U"abbb", 4},
        {U"a[abc][avb](abg)", 4},
        {U"(abb)", 3},
        {U"(abb[aa])([f])", 2},
        {U"(abb)([f])", 2},
        {U"(aab)?a", 2}
    };

    for(const auto& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.first).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexSequence*>(base_ast.get()))
        TEST_TRUE(dynamic_cast<regex::RegexSequence*>(base_ast.get())->get_elements().size() == test.second)
    }

    return true;
}

bool test_regex_quantifier() {
    struct TestCase {
        std::u32string input;

        size_t min;
        size_t max;
    };

    const std::vector<TestCase> TEST_CASES = {
        {U"a+", 1, regex::RegexQuantifier::INFINITE},
        {U"a*", 0, regex::RegexQuantifier::INFINITE},
        {U"a?", 0, 1},
        {U"a{7}", 7, 7},
        {U"a{3,}", 3, regex::RegexQuantifier::INFINITE},
        {U"a{10,15}", 10, 15}
    };

    for(const TestCase& test : TEST_CASES) {
        std::unique_ptr<regex::RegexBase> base_ast = regex::RegexParser(test.input).parse_regex();
        TEST_TRUE(dynamic_cast<regex::RegexQuantifier*>(base_ast.get()))
        regex::RegexQuantifier* quantifier = dynamic_cast<regex::RegexQuantifier*>(base_ast.get());
        TEST_TRUE(quantifier->get_min() == test.min)
        TEST_TRUE(quantifier->get_max() == test.max)
    }

    return true;
}