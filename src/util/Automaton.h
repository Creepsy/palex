#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <optional>
#include <ostream>
#include <algorithm>
#include <functional>
#include <cassert>

namespace sm {
    template<class StateValue_T, class ConnectionValue_T>
    class Automaton {
        public:
            using ConnectionID_t = size_t;
            using StateID_t = size_t;

            struct Connection {
                enum class ConnectionType {
                    VALUE,
                    EPSILON
                };
                
                StateID_t source;
                StateID_t target;

                ConnectionType type;
                std::optional<ConnectionValue_T> value;
            };
            
            template<class StateValueOut_T>
            using merge_states_t = std::function<StateValueOut_T (const std::vector<StateValue_T>&)>;

            template<class StateValueOut_T>
            using resolve_connection_collisions_t = std::function<void (const Connection&, std::vector<std::pair<ConnectionValue_T, std::set<StateID_t>>>&)>;

            Automaton();

            StateID_t add_state(const StateValue_T& to_add);
            ConnectionID_t connect_states(const StateID_t source, const StateID_t target);
            ConnectionID_t connect_states(const StateID_t source, const StateID_t target, const ConnectionValue_T& value);

            bool are_connected(const StateID_t source, const StateID_t target) const;
            bool has_outgoing_connections(const StateID_t source) const;
            bool has_incoming_connections(const StateID_t target) const;

            const std::map<StateID_t, StateValue_T>& get_states() const;
            const StateValue_T& get_state(const StateID_t id) const;
            StateValue_T& get_state(const StateID_t id);
            const Connection& get_connection(const ConnectionID_t id) const;
            std::vector<ConnectionID_t> get_all_connection_ids(const StateID_t source, const StateID_t target) const;
            std::vector<ConnectionID_t> get_outgoing_connection_ids(const StateID_t source) const;
            std::vector<ConnectionID_t> get_incoming_connection_ids(const StateID_t target) const;
        
            void remove_state(const StateID_t id);
            void remove_connection(const ConnectionID_t id);

            template<class StateValueOut_T>
            Automaton<StateValueOut_T, ConnectionValue_T> convert_to_dfa(
                const StateID_t root_state,
                merge_states_t<StateValueOut_T> merge_states,
                resolve_connection_collisions_t<StateValueOut_T> resolve_connection_collisions
            ) const;
        private:
            ConnectionID_t add_connection(const Connection& to_add);
            void insert_mergeable_states(const StateID_t source, std::set<StateID_t>& mergeable_states) const;
            std::set<StateID_t> get_mergeable_states(const std::set<StateID_t> sources) const;
            
            template<class StateValueOut_T>
            StateID_t insert_nodes_as_node_in_dfa(
                Automaton<StateValueOut_T, ConnectionValue_T>& dfa,
                const std::set<StateID_t> to_insert,
                merge_states_t<StateValueOut_T> merge_states,
                resolve_connection_collisions_t<StateValueOut_T> resolve_connection_collisions,
                std::map<std::set<StateID_t>, StateID_t>& merged_states_mappings
            ) const;

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