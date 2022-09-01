#include "regex_ast.h"

using namespace regex;


// static variables

const size_t regex::RegexQuantifier::INFINITE = (size_t) -1;



//helper functions

inline const std::string get_indentation(const size_t indentation_level);

inline const std::string get_indentation(const size_t indentation_level) {
    return std::string(indentation_level, '\t');
}



regex::CharRange::CharRange() : start(1), end(0) { //empty char range (end < start)

}

regex::CharRange::CharRange(const char32_t c) : start(c), end(c) {
}

regex::CharRange::CharRange(const char32_t start, const char32_t end) : start(start), end(end) {
}

bool regex::CharRange::is_single_char() const {
    return this->start == this->end;
}

bool regex::CharRange::is_empty() const {
    return this->end < this->start;
}

bool regex::CharRange::is_subset_of(const CharRange other) const {
    return (this->start >= other.start && this->end <= other.end) || this->is_empty();
}

CharRange regex::CharRange::common_subset(const CharRange first, const CharRange second) {
    if(first.end < second.start || second.end < first.start) return CharRange{};

    if(first.is_subset_of(second)) return first;
    if(second.is_subset_of(first)) return second;

    if(first.start <= second.end && first.start >= second.start) return CharRange(first.start, second.end);

    return CharRange(second.start, first.end); // first.start < second.start < first.end
}

bool regex::CharRange::operator==(const CharRange other) const {
    return this->start == other.start && this->end == other.end;
}



regex::RegexBranch::RegexBranch() : RegexBase() {
}   



// public

void regex::RegexBranch::add_possibility(std::unique_ptr<RegexBase> possibility) {
    this->possibilities.push_back(std::move(possibility));
}

void regex::RegexBranch::debug(std::ostream& output, const size_t indentation_level) const {
    output << get_indentation(indentation_level) << "Branch\n";

    for(const std::unique_ptr<RegexBase>& possibility : this->possibilities) {
        possibility->debug(output, indentation_level + 1);
    }
}



regex::RegexSequence::RegexSequence() : RegexBase() {
}   



// public

void regex::RegexSequence::append_element(std::unique_ptr<RegexBase> to_append) {
    this->sequence.push_back(std::move(to_append));
}

void regex::RegexSequence::debug(std::ostream& output, const size_t indentation_level) const {
    output << get_indentation(indentation_level) << "Sequence\n";

    for(const std::unique_ptr<RegexBase>& element : this->sequence) {
        element->debug(output, indentation_level + 1);
    }
}



regex::RegexQuantifier::RegexQuantifier(std::unique_ptr<RegexBase> operand, const size_t min_count, const size_t max_count)
 : RegexBase(), operand(std::move(operand)), min_count(min_count), max_count(max_count) {
}   



// public

void regex::RegexQuantifier::debug(std::ostream& output, const size_t indentation_level) const {
    output << get_indentation(indentation_level) << "Quantifier(" << this->min_count << "-" << this->max_count << ")\n";
    this->operand->debug(output, indentation_level + 1);
}



regex::RegexCharSet::RegexCharSet(const bool negated) : RegexBase(), negated(negated) {
} 



// public

RegexCharSet& regex::RegexCharSet::add_char_range(CharRange to_add) {
    if(!to_add.is_empty()) {
        auto iter = this->ranges.begin();

        while(iter != this->ranges.end()) {
            //subset checks
            if(to_add.is_subset_of(*iter)) break;
            if((*iter).is_subset_of(to_add)) {
                iter = this->ranges.erase(iter);
                continue;
            }

            if(to_add.end < (*iter).end) { // element has to be inserted here
                if(to_add.end + 1 >= (*iter).start) { //ranges can be merged
                    (*iter).start = to_add.start;
                } else {
                    this->ranges.insert(iter, to_add);
                }
                break;
            }
            
            if((*iter).end + 1 >= to_add.start) { // *iter can be appended to to_add
                to_add.start = (*iter).start;
                iter = this->ranges.erase(iter);
                continue;
            }

            iter++;
        }    

        if(iter == this->ranges.end()) {
            if(!this->ranges.empty() && this->ranges.back().end + 1 == to_add.start) { // can be appended to last element
                this->ranges.back().end = to_add.end;
            } else {
                this->ranges.push_back(to_add);
            }
        }
    }

    return *this;
}

void regex::RegexCharSet::debug(std::ostream& output, const size_t indentation_level) const {
    output << get_indentation(indentation_level) << "CharSet(";
    if(this->negated) output << "^";

    for(const CharRange& range : this->ranges) output << range;

    output << ")\n";
}



// namespace functions

std::ostream& regex::operator<<(std::ostream& output, const CharRange& to_print) {
    if(to_print.start == to_print.end) {
        return output << '[' << (size_t)to_print.start << ']';
    } else {
        return output << '[' << (size_t)to_print.start << "-" << (size_t)to_print.end << ']';
    }
}