#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstddef>
#include <utility>
#include <string>

#include <json.h>

#include "util/utf8.h"
#include "util/Automaton.h"
#include "util/palex_except.h"
#include "util/config_getters.h"

#include "TestReport.h"
#include "test_utils.h"

bool test_utf8_input();
bool test_utf8_output();

bool test_automaton_states();
bool test_automaton_connections();
bool test_automaton_dfa_conversion();

bool test_config_getters();

int main() {
    tests::TestReport report;

    report.add_test("utf8_input", test_utf8_input);
    report.add_test("utf8_output", test_utf8_output);
    
    report.add_test("automaton_states", test_automaton_states);
    report.add_test("automaton_connections", test_automaton_connections);
    report.add_test("automaton_dfa_conversion", test_automaton_dfa_conversion);

    report.add_test("config_getters", test_config_getters);

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

    return true;
}

bool test_automaton_dfa_conversion() {
    auto merge_states = [](const std::vector<int>& to_merge) -> int {
        return to_merge.front();
    };

    auto resolve_connection_collisions = [](
        const sm::Automaton<int, int>::Connection& to_add, 
        std::vector<std::pair<int, std::set<size_t>>>& dfa_connections
    ) -> void {
        for (size_t i = 0; i < dfa_connections.size(); i++) {
            if (dfa_connections.at(i).first == to_add.value) {
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

    return true;
}

bool test_config_getters() {
    const nlohmann::json INPUT = nlohmann::json::parse(R"(
        {
            "abool": true,
            "notabool": 24,
            "astring": "abc",
            "notastring": false
        }
    )");

    bool bool_storage = false;
    std::string string_storage;

    TEST_TRUE(config::optional_bool(INPUT, "abool", bool_storage))
    TEST_TRUE(bool_storage)
    TEST_FALSE(config::optional_bool(INPUT, "notabool", bool_storage))
    TEST_TRUE(bool_storage)

    TEST_EXCEPT(config::require_bool(INPUT, "notabool", bool_storage, "test_input"), palex_except::ParserError)

    TEST_TRUE(config::optional_string(INPUT, "astring", string_storage))
    TEST_TRUE(string_storage == "abc")
    TEST_FALSE(config::optional_string(INPUT, "notastring", string_storage))
    TEST_TRUE(string_storage == "abc")
    
    TEST_EXCEPT(config::require_string(INPUT, "notastring", string_storage, "test_input"), palex_except::ParserError)
    
    return true;
}