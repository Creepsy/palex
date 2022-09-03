template<class StateValue_T, class ConnectionValue_T>
sm::Automaton<StateValue_T, ConnectionValue_T>::Automaton() {
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::add_state(const StateValue_T& to_add) -> StateID_t {
    this->states.push_back(to_add);

    return this->states.size() - 1;
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::connect_states(const size_t source, const size_t target) -> ConnectionID_t {
    return this->add_connection(Connection{source, target, Connection::ConnectionType::EPSILON, std::nullopt});
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::connect_states(const size_t source, const size_t target, const ConnectionValue_T& value) -> ConnectionID_t {
    return this->add_connection(Connection{source, target, Connection::ConnectionType::VALUE, value});
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