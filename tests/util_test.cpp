#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstddef>
#include <utility>

#include "util/utf8.h"
#include "util/Automaton.h"
#include "util/palex_except.h"

#include "TestReport.h"
#include "test_utils.h"

bool test_utf8_input();
bool test_utf8_output();

bool test_automaton_states();
bool test_automaton_connections();

int main() {
    tests::TestReport report;

    report.add_test("utf8_input", test_utf8_input);
    report.add_test("utf8_output", test_utf8_output);
    
    report.add_test("automaton_states", test_automaton_states);
    report.add_test("automaton_connections", test_automaton_connections);

    report.run();

    return report.report();
}

bool test_utf8_input() {
    std::stringstream input("aШᢶ𐅄");

    TEST_TRUE(utf8::get_unicode_char(input) == 97)       // 1 byte char
    TEST_TRUE(utf8::get_unicode_char(input) == 1064)     // 2 byte char
    TEST_TRUE(utf8::get_unicode_char(input) == 6326)     // 3 byte char
    TEST_TRUE(utf8::get_unicode_char(input) == 65860)    // 4 byte char


    return true;   
}

bool test_utf8_output() {
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

bool test_automaton_states() {
    sm::Automaton<int, int> test_automaton;

    TEST_TRUE(test_automaton.add_state(10) == 0)
    TEST_TRUE(test_automaton.add_state(11) == 1)
    TEST_TRUE(test_automaton.get_state(0) == 10)
    TEST_TRUE(test_automaton.get_state(1) == 11)

    test_automaton.remove_state(0);
    TEST_EXCEPT(test_automaton.get_state(0), std::out_of_range)
    TEST_TRUE(test_automaton.add_state(12) == 2)

    return true;
}

bool test_automaton_connections() {
    sm::Automaton<int, int> test_automaton;
    
    sm::Automaton<int, int>::StateID_t first = test_automaton.add_state(10);
    sm::Automaton<int, int>::StateID_t second = test_automaton.add_state(11);

    TEST_EXCEPT(test_automaton.connect_states(1, 2), std::out_of_range)
    TEST_EXCEPT(test_automaton.connect_states(2, 0, 10), std::out_of_range)

    TEST_TRUE(test_automaton.connect_states(0, 1) == 0)

    TEST_TRUE(test_automaton.are_connected(0, 1));
    TEST_FALSE(test_automaton.are_connected(1, 0));
    TEST_TRUE(test_automaton.has_outgoing_connections(0))
    TEST_TRUE(test_automaton.has_incoming_connections(1))
    TEST_FALSE(test_automaton.has_outgoing_connections(1))
    TEST_FALSE(test_automaton.has_incoming_connections(0))

    test_automaton.remove_connection(0);
    
    TEST_FALSE(test_automaton.are_connected(0, 1))
    TEST_FALSE(test_automaton.has_incoming_connections(1))
    TEST_FALSE(test_automaton.has_outgoing_connections(0))

    TEST_TRUE(test_automaton.connect_states(1, 0, 10) == 1)
    TEST_TRUE(test_automaton.connect_states(0, 1) == 2)

    TEST_TRUE(test_automaton.has_incoming_connections(0))
    TEST_TRUE(test_automaton.has_outgoing_connections(0))

    test_automaton.remove_state(1);
    
    TEST_FALSE(test_automaton.has_incoming_connections(0))
    TEST_FALSE(test_automaton.has_outgoing_connections(0))

    return true;
}