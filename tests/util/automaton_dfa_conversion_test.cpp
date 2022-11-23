#include <vector>
#include <utility>

#include "util/Automaton.h"

#include "../test_utils.h"

int main() {
    auto merge_states = [](const std::vector<int>& to_merge) -> int {
        return to_merge.front();
    };

    auto resolve_connection_collisions = [](
        const sm::Automaton<int, int>::Connection& to_add, 
        std::vector<std::pair<int, std::set<size_t>>>& dfa_connections
    ) -> void {
        for(size_t i = 0; i < dfa_connections.size(); i++) {
            if(dfa_connections.at(i).first == to_add.value) {
                dfa_connections.at(i).second.insert(to_add.target);
                return;
            }
        }

        dfa_connections.push_back(std::make_pair(to_add.value.value(), std::set<size_t>{to_add.target}));
    };

    sm::Automaton<int, int> test_automaton;

    test_automaton.add_state(10);
    test_automaton.add_state(11);
    test_automaton.add_state(12);
    test_automaton.add_state(13);

    test_automaton.connect_states(0, 1);
    test_automaton.connect_states(1, 0);
    test_automaton.connect_states(0, 2, 20);
    test_automaton.connect_states(2, 3, 21);
    test_automaton.connect_states(1, 3);

    sm::Automaton<int, int> test_dfa = test_automaton.convert_to_dfa<int>(0, merge_states, resolve_connection_collisions);

    TEST_TRUE(test_dfa.get_states().size() == 3);
    
    TEST_TRUE(test_dfa.are_connected(0, 1))
    TEST_TRUE(test_dfa.are_connected(1, 2))
    TEST_FALSE(test_dfa.are_connected(0, 2))
    
    TEST_TRUE(test_dfa.get_connection(1).source == 0)
    TEST_TRUE(test_dfa.get_connection(1).target == 1)
    TEST_TRUE(test_dfa.get_connection(1).value.has_value() && test_dfa.get_connection(1).value.value() == 20)

    TEST_TRUE(test_dfa.get_connection(0).source == 1)
    TEST_TRUE(test_dfa.get_connection(0).target == 2)
    TEST_TRUE(test_dfa.get_connection(0).value.has_value() && test_dfa.get_connection(0).value.value() == 21)

    return 0;
}