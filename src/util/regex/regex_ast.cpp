#include "regex_ast.h"

#include <cassert>

#include "util/unicode.h"

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

bool regex::CharRange::can_prepend_to(const CharRange target) const {
    return !this->is_empty() && !target.is_empty() && this->end + 1 >= target.start && this->start <= target.start;
}

bool regex::CharRange::can_append_to(const CharRange target) const {
    return !this->is_empty() && !target.is_empty() && target.can_prepend_to(*this);
}

bool regex::CharRange::is_subset_of(const CharRange other) const {
    return (this->start >= other.start && this->end <= other.end) || this->is_empty();
}

void regex::CharRange::add_to_list(std::list<CharRange>& target) const {
    CharRange insert_copy = *this;

    if(!this->is_empty()) {
        auto iter = target.begin();

        while(iter != target.end()) {
            //subset checks
            if(insert_copy.is_subset_of(*iter)) break;
            if((*iter).is_subset_of(insert_copy)) {
                iter = target.erase(iter);
                continue;
            }

            if(insert_copy.end < (*iter).end) { // element has to be inserted here
                if(insert_copy.can_prepend_to(*iter)) {
                    (*iter).start = insert_copy.start;
                } else {
                    target.insert(iter, insert_copy);
                }
                break;
            }
            
            if((*iter).can_prepend_to(insert_copy)) {
                insert_copy.start = (*iter).start;
                iter = target.erase(iter);
                continue;
            }

            iter++;
        }    

        if(iter == target.end()) {
            if(!target.empty() && insert_copy.can_append_to(target.back())) {
                target.back().end = insert_copy.end;
            } else {
                target.push_back(insert_copy);
            }
        }
    }
}

void regex::CharRange::remove_from_list(std::list<CharRange>& target) const {
    if(!this->is_empty()) {
        auto iter = target.begin();

        while(iter != target.end()) {
            const CharRange intersection = CharRange::common_subset(*this, *iter);

            if(!intersection.is_empty()) {
                if(*iter == intersection) {
                    iter = target.erase(iter);
                    continue;
                }

                const CharRange first_half{(*iter).start, intersection.start - 1}; 
                const CharRange second_half{intersection.end + 1, (*iter).end};

                assert(("Range remove error! Please create an issue on github containing the used regex!", !(first_half.is_empty() && second_half.is_empty())));

                if(first_half.is_empty()) {
                    *iter = second_half;
                } else if(second_half.is_empty()) {
                    *iter = first_half;
                } else {
                    *iter = second_half;
                    iter = target.insert(iter, first_half);
                    iter++;
                }
            }

            iter++;
        }
    }
}

bool regex::CharRange::operator==(const CharRange other) const {
    return (this->is_empty() && other.is_empty()) || (this->start == other.start && this->end == other.end);
}

// static

regex::CharRange regex::CharRange::common_subset(const CharRange first, const CharRange second) {
    if(first.end < second.start || second.end < first.start) return CharRange{};

    if(first.is_subset_of(second)) return first;
    if(second.is_subset_of(first)) return second;

    if(first.start <= second.end && first.start >= second.start) return CharRange(first.start, second.end);

    return CharRange(second.start, first.end); // first.start < second.start < first.end
}



regex::RegexBranch::RegexBranch() : RegexBase() {
}   

// public

void regex::RegexBranch::add_possibility(std::unique_ptr<RegexBase> possibility) {
    this->possibilities.push_back(std::move(possibility));
}

const std::vector<std::unique_ptr<regex::RegexBase>>& regex::RegexBranch::get_possibilities() const {
    return this->possibilities;
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

const std::vector<std::unique_ptr<regex::RegexBase>>& regex::RegexSequence::get_elements() const {
    return this->sequence;
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

const std::unique_ptr<regex::RegexBase>& regex::RegexQuantifier::get_operand() const {
    return this->operand;
}

size_t regex::RegexQuantifier::get_min() const {
    return this->min_count;
}
size_t regex::RegexQuantifier::get_max() const {
    return this->max_count;
}

void regex::RegexQuantifier::debug(std::ostream& output, const size_t indentation_level) const {
    output << get_indentation(indentation_level) << "Quantifier(" << this->min_count << "-" << this->max_count << ")\n";
    this->operand->debug(output, indentation_level + 1);
}



regex::RegexCharSet::RegexCharSet(const bool negated) : RegexBase(), negated(negated) {
    if(this->negated) {
        CharRange{0, unicode::LAST_UNICODE_CHAR}.add_to_list(this->ranges);
    }
} 

// public

void regex::RegexCharSet::insert_char_range(CharRange to_add) {
    if(this->negated) {
        to_add.remove_from_list(this->ranges);
    } else {
        to_add.add_to_list(this->ranges);
    }
}

const std::list<regex::CharRange>& regex::RegexCharSet::get_characters() const {
    return this->ranges;
}

bool regex::RegexCharSet::is_negated() const {
    return this->negated;
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