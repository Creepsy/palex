template<class StateValue_T, class ConnectionValue_T>
sm::Automaton<StateValue_T, ConnectionValue_T>::Automaton() {
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::add_state(const StateValue_T& to_add) -> StateID_t {
    this->states.push_back(to_add);

    return this->states.size() - 1;
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::connect_states(const StateID_t source, const StateID_t target) -> ConnectionID_t {
    return this->add_connection(Connection{source, target, Connection::ConnectionType::EPSILON, std::nullopt});
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::connect_states(const StateID_t source, const StateID_t target, const ConnectionValue_T& value) -> ConnectionID_t {
    return this->add_connection(Connection{source, target, Connection::ConnectionType::VALUE, value});
}

template<class StateValue_T, class ConnectionValue_T>
bool sm::Automaton<StateValue_T, ConnectionValue_T>::has_connection(const StateID_t source, const StateID_t target) const {
    if(this->transition_table.find(source) == this->transition_table.end()) return false;
    if(this->transition_table.at(source).find(target) == this->transition_table.at(source).end()) return false;

    return true;
}

template<class StateValue_T, class ConnectionValue_T>
bool sm::Automaton<StateValue_T, ConnectionValue_T>::has_connection(const StateID_t source, const StateID_t target, const ConnectionValue_T& value) const {
    if(!this->has_connection(source, target)) return false;

    for(const Connection& c : this->get_connections(source, target)) {
        if(c.value == value) return true;
    }

    return false;
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_state(const StateID_t id) const -> const StateValue_T& {
    return this->states.at(id);
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_state(const StateID_t id) -> StateValue_T& {
    return this->states.at(id);
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_connection(const ConnectionID_t id) const -> const Connection& {
    return this->connections.at(id);
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_connection(const ConnectionID_t id) -> Connection& {
    return this->connections.at(id);
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_connection(const StateID_t source, const StateID_t target, const ConnectionValue_T& value) const -> const Connection& {
    for(const ConnectionID_t c_id : this->get_connection_ids(source, target)) {
        if(this->connections.at(c_id).value == value) return this->connections.at(c_id);
    }

    throw std::runtime_error("No matching connection was found!");
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_connection(const StateID_t source, const StateID_t target, const ConnectionValue_T& value) -> Connection& {
    for(const ConnectionID_t c_id : this->get_connection_ids(source, target)) {
        if(this->connections.at(c_id).value == value) return this->connections.at(c_id);
    }

    throw std::runtime_error("No matching connection was found!");
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_connection_ids(const StateID_t source, const StateID_t target) const -> const std::vector<ConnectionID_t>& {
    return this->get_connection(this->transition_table.at(source).at(target));
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_connection_ids(const StateID_t source, const StateID_t target) -> std::vector<ConnectionID_t>& {
    return this->get_connection(this->transition_table.at(source).at(target));
}

// private

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::add_connection(const Connection& to_add) -> ConnectionID_t {
    if(to_add.source >= this->states.size() || to_add.target >= this->states.size())
        throw std::out_of_range("At least one of the nodes to connect doesn't exist!");
    this->connections.push_back(to_add);
    this->transition_table[to_add.source][to_add.target].push_back(this->connections.size() - 1);

    return this->connections.size() - 1;    
}

// functions

template<class StateValue_T, class ConnectionValue_T>
std::ostream& sm::operator<<(std::ostream& output, const Automaton<StateValue_T, ConnectionValue_T>& to_print) {
    output << "digraph {\n";

    for(size_t state = 0; state < to_print.states.size(); state++) {
        output << "\t" << state << " [label=\"" << to_print.states[state] << "\"];\n";
    }

    for(const typename Automaton<StateValue_T, ConnectionValue_T>::Connection& c : to_print.connections) {
        output << "\t" << c.source << " -> " << c.target;
        if(c.type != Automaton<StateValue_T, ConnectionValue_T>::Connection::ConnectionType::EPSILON) {
            output << " [label=\"" << c.value.value()<< "\"]";
        } else {
            output << " [color=red" << "]";
        }
        output << ";\n";
    }

    return output << "}";
}
