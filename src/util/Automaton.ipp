template<class StateValue_T, class ConnectionValue_T>
sm::Automaton<StateValue_T, ConnectionValue_T>::Automaton() : next_state_id{0}, next_connection_id{0} {
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::add_state(const StateValue_T& to_add) -> StateID_t {
    const StateID_t state_id = this->next_state_id++;

    this->states.insert(std::make_pair(state_id, to_add));

    return state_id;
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
bool sm::Automaton<StateValue_T, ConnectionValue_T>::are_connected(const StateID_t source, const StateID_t target) const {
    return !this->get_all_connection_ids(source, target).empty();
}

template<class StateValue_T, class ConnectionValue_T>
bool sm::Automaton<StateValue_T, ConnectionValue_T>::has_outgoing_connections(const StateID_t source) const {
    return !this->get_outgoing_connection_ids(source).empty();
}
template<class StateValue_T, class ConnectionValue_T>
bool sm::Automaton<StateValue_T, ConnectionValue_T>::has_incoming_connections(const StateID_t target) const {
    return !this->get_incoming_connection_ids(target).empty();
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
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_all_connection_ids(const StateID_t source, const StateID_t target) const -> std::vector<ConnectionID_t> {
    if(this->transition_table.find(source) == this->transition_table.end()) return {};
    if(this->transition_table.at(source).find(target) == this->transition_table.at(source).end()) return {};

    return this->transition_table.at(source).at(target);
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_outgoing_connection_ids(const StateID_t source) const -> std::vector<ConnectionID_t> {
    if(this->transition_table.find(source) == this->transition_table.end()) return {};

    std::vector<ConnectionID_t> outgoing;

    for(const std::pair<StateID_t, std::vector<ConnectionID_t>>& target : this->transition_table.at(source)) {
        outgoing.insert(outgoing.end(), target.second.begin(), target.second.end());
    }

    return outgoing;
}

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::get_incoming_connection_ids(const StateID_t target) const -> std::vector<ConnectionID_t> {
    std::vector<ConnectionID_t> incoming;
    
    for(const auto& source : this->transition_table) {
        if(source.second.find(target) != source.second.end()) {
            incoming.insert(incoming.end(), source.second.at(target).begin(), source.second.at(target).end());
        }
    }

    return incoming;
}

template<class StateValue_T, class ConnectionValue_T>
void sm::Automaton<StateValue_T, ConnectionValue_T>::remove_state(const StateID_t id) {
    for(const ConnectionID_t to_remove : this->get_incoming_connection_ids(id)) {
        this->remove_connection(to_remove);
    }

    for(const ConnectionID_t to_remove : this->get_outgoing_connection_ids(id)) {
        this->remove_connection(to_remove);
    }

    this->states.erase(id);
}

template<class StateValue_T, class ConnectionValue_T>
void sm::Automaton<StateValue_T, ConnectionValue_T>::remove_connection(const ConnectionID_t id) {
    const Connection& to_remove = this->get_connection(id);

    std::vector<ConnectionID_t>& transitions = this->transition_table.at(to_remove.source).at(to_remove.target);
    transitions.erase(std::find(transitions.begin(), transitions.end(), id));

    this->connections.erase(id);
}

// private

template<class StateValue_T, class ConnectionValue_T>
auto sm::Automaton<StateValue_T, ConnectionValue_T>::add_connection(const Connection& to_add) -> ConnectionID_t {
    if(to_add.source >= this->states.size() || to_add.target >= this->states.size())
        throw std::out_of_range("At least one of the nodes to connect doesn't exist!");
    const ConnectionID_t connection_id = this->next_connection_id++;

    this->connections.insert(std::make_pair(connection_id, to_add));
    this->transition_table[to_add.source][to_add.target].push_back(connection_id);

    return connection_id;    
}

// functions

template<class StateValue_T, class ConnectionValue_T>
std::ostream& sm::operator<<(std::ostream& output, const Automaton<StateValue_T, ConnectionValue_T>& to_print) {
    output << "digraph {\n";

    for(const auto& state : to_print.states) {
        output << "\t" << state.first << " [label=\"" << state.second << "\"];\n";
    }

    for(const auto& connection : to_print.connections) {
        output << "\t" << connection.second.source << " -> " << connection.second.target;
        if(connection.second.type != Automaton<StateValue_T, ConnectionValue_T>::Connection::ConnectionType::EPSILON) {
            output << " [label=\"" << connection.second.value.value()<< "\"]";
        } else {
            output << " [color=red" << "]";
        }
        output << ";\n";
    }

    return output << "}";
}
