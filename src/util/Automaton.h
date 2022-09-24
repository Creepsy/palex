#pragma once

#include <cstddef>
#include <map>
#include <vector>
#include <stdexcept>
#include <optional>
#include <ostream>
#include <algorithm>

namespace sm {
    template<class StateValue_T, class ConnectionValue_T>
    class Automaton {
        public:
            typedef size_t ConnectionID_t;
            typedef size_t StateID_t;

            struct Connection {
                enum class ConnectionType {
                    VALUE,
                    EPSILON
                };
                
                size_t source;
                size_t target;

                ConnectionType type;
                std::optional<ConnectionValue_T> value;
            };

            Automaton();

            StateID_t add_state(const StateValue_T& to_add);
            ConnectionID_t connect_states(const StateID_t source, const StateID_t target);
            ConnectionID_t connect_states(const StateID_t source, const StateID_t target, const ConnectionValue_T& value);

            bool are_connected(const StateID_t source, const StateID_t target) const;
            bool has_outgoing_connections(const StateID_t source) const;
            bool has_incoming_connections(const StateID_t target) const;

            const StateValue_T& get_state(const StateID_t id) const;
            StateValue_T& get_state(const StateID_t id);
            const Connection& get_connection(const ConnectionID_t id) const;
            std::vector<ConnectionID_t> get_all_connection_ids(const StateID_t source, const StateID_t target) const;
            std::vector<ConnectionID_t> get_outgoing_connection_ids(const StateID_t source) const;
            std::vector<ConnectionID_t> get_incoming_connection_ids(const StateID_t target) const;
        
            void remove_state(const StateID_t id);
            void remove_connection(const ConnectionID_t id);

        private:
            ConnectionID_t add_connection(const Connection& to_add);

            StateID_t next_state_id;
            ConnectionID_t next_connection_id;

            std::map<StateID_t, StateValue_T> states;
            std::map<ConnectionID_t, const Connection> connections;

            std::map<StateID_t, std::map<StateID_t, std::vector<ConnectionID_t>>> transition_table;

            template<class FuncStateValue_T, class FuncConnectionValue_T>
            friend std::ostream& operator<<(std::ostream& output, const Automaton<FuncStateValue_T, FuncConnectionValue_T>& to_print);
    };    

    template<class StateValue_T, class ConnectionValue_T>
    std::ostream& operator<<(std::ostream& output, const Automaton<StateValue_T, ConnectionValue_T>& to_print); // converts to graphviz description
}

#include "Automaton.ipp"