template<class ParseObject_T>
std::vector<ParseObject_T> regex::RegexParser::parse_until(Predicate_t predicate, ParseObject_T(RegexParser::*parse_func)(), const size_t max_count) {
    std::vector<ParseObject_T> return_sequence;

    while (!this->end() && !predicate(this->get_curr()) && return_sequence.size() < max_count) {
        return_sequence.push_back(std::move((this->*parse_func)()));
    }

    return return_sequence;
}