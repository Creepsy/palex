#include <sstream>
#include <stdexcept>

#include "util/unicode.h"
#include "util/Automaton.h"
#include "util/palex_except.h"

#include "util/regex/RegexParser.h"

#include "TestReport.h"
#include "test_utils.h"

bool test_unicode_input();
bool test_unicode_output();

bool test_automaton_add_states();
bool test_automaton_add_connections();

bool test_regex_errors();

int main() {
    tests::TestReport report;

    report.add_test("unicode_input", test_unicode_input);
    report.add_test("unicode_output", test_unicode_output);
    
    report.add_test("automaton_add_states", test_automaton_add_states);
    report.add_test("automaton_add_connections", test_automaton_add_connections);
    
    report.add_test("regex_errors", test_regex_errors);

    report.run();

    return report.report();
}

bool test_unicode_input() {
    std::stringstream input("aШᢶ𐅄");

    return unicode::get_utf8(input) == 97 &&   // 1 byte char
           unicode::get_utf8(input) == 1064 && // 2 byte char
           unicode::get_utf8(input) == 6326 && // 3 byte char
           unicode::get_utf8(input) == 65860;   // 4 byte char
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

    return output.str() == "aШᢶ𐅄";
}

bool test_automaton_add_states() {
    sm::Automaton<int, int> test_automaton;

    TEST_TRUE(test_automaton.add_state(10) == 0);
    TEST_TRUE(test_automaton.add_state(11) == 1);

    return true;
}

bool test_automaton_add_connections() {
    sm::Automaton<int, int> test_automaton;
    
    sm::Automaton<int, int>::StateID_t first = test_automaton.add_state(10);
    sm::Automaton<int, int>::StateID_t second = test_automaton.add_state(11);

    TEST_EXCEPT(test_automaton.connect_states(1, 2), std::out_of_range);
    TEST_EXCEPT(test_automaton.connect_states(2, 0, 10), std::out_of_range);

    TEST_TRUE(test_automaton.connect_states(0, 1) == 0);
    TEST_TRUE(test_automaton.connect_states(1, 0, 10) == 1);

    return true;
}

bool test_regex_errors() {
    TEST_EXCEPT(regex::RegexParser(U")").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"()").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"[").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"[").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"(").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"[").parse_regex(), palex_except::ParserError);

    TEST_EXCEPT(regex::RegexParser(U"").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"a|").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"|").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"|a").parse_regex(), palex_except::ParserError);

    TEST_EXCEPT(regex::RegexParser(U"\\ca").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"\\uAfF").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"\\u24Ga").parse_regex(), palex_except::ParserError);

    TEST_EXCEPT(regex::RegexParser(U"a{3,").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"a{}").parse_regex(), palex_except::ParserError);
    TEST_EXCEPT(regex::RegexParser(U"a{").parse_regex(), palex_except::ParserError);

    return true;
}