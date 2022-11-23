#include "util/Automaton.h"

#include "../test_utils.h"

int main() {
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
    TEST_TRUE(test_automaton.connect_states(0, 1) == 3)

    TEST_TRUE(test_automaton.get_all_connection_ids(0, 1).size() == 2)
    TEST_TRUE(test_automaton.get_all_connection_ids(1, 0).size() == 1)

    TEST_TRUE(test_automaton.has_incoming_connections(0))
    TEST_TRUE(test_automaton.has_outgoing_connections(0))
    TEST_TRUE(test_automaton.has_incoming_connections(1))
    TEST_TRUE(test_automaton.has_outgoing_connections(1))

    TEST_TRUE(test_automaton.get_connection(1).value.has_value() && test_automaton.get_connection(1).value.value() == 10)
    TEST_FALSE(test_automaton.get_connection(2).value.has_value())

    TEST_EXCEPT(test_automaton.get_connection(10), std::out_of_range)

    test_automaton.remove_state(1);
    
    TEST_FALSE(test_automaton.has_incoming_connections(0))
    TEST_FALSE(test_automaton.has_outgoing_connections(0))

    return 0;
}