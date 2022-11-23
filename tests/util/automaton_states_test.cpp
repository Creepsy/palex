#include <map>

#include "util/Automaton.h"

#include "../test_utils.h"

int main() {
    sm::Automaton<int, int> test_automaton;

    TEST_TRUE(test_automaton.get_states().empty())

    TEST_TRUE(test_automaton.add_state(10) == 0)
    TEST_TRUE(test_automaton.add_state(11) == 1)
    TEST_TRUE(test_automaton.get_state(0) == 10)
    TEST_TRUE(test_automaton.get_state(1) == 11)
    TEST_TRUE((test_automaton.get_states() == std::map<size_t, int>{{0, 10}, {1, 11}}))

    test_automaton.remove_state(0);
    TEST_TRUE((test_automaton.get_states() == std::map<size_t, int>{{1, 11}}))

    TEST_EXCEPT(test_automaton.get_state(0), std::out_of_range)

    TEST_TRUE(test_automaton.add_state(12) == 2)
    TEST_TRUE((test_automaton.get_states() == std::map<size_t, int>{{1, 11}, {2, 12}}))

    return 0;
}