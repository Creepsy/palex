#include "regex_ast.h"

#include <cassert>
#include <cmath>

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

bool regex::CharRange::empty() const {
    return this->end < this->start;
}

bool regex::CharRange::can_prepend_to(const CharRange target) const {
    return !this->empty() && !target.empty() && this->end + 1 >= target.start && this->start <= target.start;
}

bool regex::CharRange::can_append_to(const CharRange target) const {
    return !this->empty() && !target.empty() && target.can_prepend_to(*this);
}

bool regex::CharRange::is_subset_of(const CharRange other) const {
    return (this->start >= other.start && this->end <= other.end) || this->empty();
}

bool regex::CharRange::operator==(const CharRange other) const {
    return (this->empty() && other.empty()) || (this->start == other.start && this->end == other.end);
}

// static

regex::CharRange regex::CharRange::common_subset(const CharRange first, const CharRange second) {
    if(first.end < second.start || second.end < first.start) return CharRange{};

    if(first.is_subset_of(second)) return first;
    if(second.is_subset_of(first)) return second;

    if(first.start <= second.end && first.start >= second.start) return CharRange(first.start, second.end);

    return CharRange(second.start, first.end); // first.start < second.start < first.end
}



regex::CharRangeSet& regex::CharRangeSet::insert_char_range(CharRange to_add) {
    if(!to_add.empty()) {
        auto iter = this->ranges.begin();

        while(iter != this->ranges.end()) {
            //subset checks
            if(to_add.is_subset_of(*iter)) return *this;
            if((*iter).is_subset_of(to_add)) {
                iter = this->ranges.erase(iter);
                continue;
            }

            if(to_add.end < (*iter).end) { // element has to be inserted here
                if(to_add.can_prepend_to(*iter)) {
                    (*iter).start = to_add.start;
                } else {
                    this->ranges.insert(iter, to_add);
                }
                return *this;
            }
            
            if((*iter).can_prepend_to(to_add)) {
                to_add.start = (*iter).start;
                iter = this->ranges.erase(iter);
                continue;
            }

            iter++;
        }    

        if(!this->ranges.empty() && to_add.can_append_to(this->ranges.back())) {
            this->ranges.back().end = to_add.end;
        } else {
            this->ranges.push_back(to_add);
        }
    }

    return *this;
}

regex::CharRangeSet& regex::CharRangeSet::remove_char_range(const CharRange to_remove) {
    if(!to_remove.empty()) {
        auto iter = this->ranges.begin();

        while(iter != this->ranges.end()) {
            const CharRange intersection = CharRange::common_subset(to_remove, *iter);

            if(!intersection.empty()) {
                if(*iter == intersection) {
                    iter = this->ranges.erase(iter);
                    continue;
                }

                const CharRange first_half{(*iter).start, intersection.start - 1}; 
                const CharRange second_half{intersection.end + 1, (*iter).end};

                assert(("Range remove error! Please create an issue on github containing the used input!", !(first_half.empty() && second_half.empty())));
                if(first_half.empty() || intersection.start == 0) { // edge-case which leads to underflow
                    *iter = second_half;
                } else if(second_half.empty()) {
                    *iter = first_half;
                } else {
                    *iter = second_half;
                    iter = this->ranges.insert(iter, first_half);
                    iter++;
                }
            }

            iter++;
        }
    }

    return *this;
}

bool regex::CharRangeSet::empty() const {
    return this->ranges.empty();
}

regex::CharRangeSet regex::CharRangeSet::get_intersection(const CharRangeSet& other) const {
    CharRangeSet intersection;

    for(const CharRange& own_range : this->ranges) {
        for(const CharRange& other_range : other.ranges) {
            CharRange range_intersection = CharRange::common_subset(own_range, other_range);

            if(!range_intersection.empty()) {
                intersection.insert_char_range(range_intersection);
            }
        }
    }

    return intersection;
}

std::list<regex::CharRange>& regex::CharRangeSet::get_ranges() {
    return this->ranges;
}

const std::list<regex::CharRange>& regex::CharRangeSet::get_ranges() const {
    return this->ranges;
}

regex::CharRangeSet regex::CharRangeSet::operator-(const CharRangeSet& to_subtract) const {
    CharRangeSet subtracted = *this;

    for(const CharRange range : to_subtract.ranges) {
        subtracted.remove_char_range(range);
    }
    
    return subtracted;
}

regex::CharRangeSet regex::CharRangeSet::operator+(const CharRangeSet to_add) const {
    CharRangeSet combined = *this;

    for(const CharRange range : to_add.ranges) {
        combined.insert_char_range(range);
    }

    return combined;
}


bool regex::CharRangeSet::operator==(const CharRangeSet& other) const {
    return this->ranges == other.ranges;
}

bool regex::CharRangeSet::operator!=(const CharRangeSet& other) const {
    return this->ranges != other.ranges;
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

size_t regex::RegexBranch::get_priority() const {
    size_t shortest = (size_t)-1;

    for(const std::unique_ptr<RegexBase>& possibility : this->possibilities) {
        shortest = std::min(shortest, possibility->get_priority());
    }

    return shortest;
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

size_t regex::RegexSequence::get_priority() const {
    size_t priority_sum = 0;

    for(const std::unique_ptr<RegexBase>& element : this->sequence) {
        priority_sum += element->get_priority();
    }

    return priority_sum;
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

size_t regex::RegexQuantifier::get_priority() const {
    return this->min_count * this->operand->get_priority();
}

void regex::RegexQuantifier::debug(std::ostream& output, const size_t indentation_level) const {
    output << get_indentation(indentation_level) << "Quantifier(" << this->min_count << "-" << this->max_count << ")\n";
    this->operand->debug(output, indentation_level + 1);
}



regex::RegexCharSet::RegexCharSet(const bool negated) : RegexBase(), negated(negated) {
    if(this->negated) {
        this->range_set.insert_char_range(CharRange{0, unicode::LAST_UNICODE_CHAR});
    }
} 

// public

void regex::RegexCharSet::insert_char_range(CharRange to_insert) {
    if(this->negated) {
        this->range_set.remove_char_range(to_insert);
    } else {
        this->range_set.insert_char_range(to_insert);
    }
}

const regex::CharRangeSet& regex::RegexCharSet::get_range_set() const {
    return this->range_set;
}

bool regex::RegexCharSet::is_negated() const {
    return this->negated;
}

size_t regex::RegexCharSet::get_priority() const {
    if(this->range_set.empty()) return 0;
    if(this->range_set.get_ranges().size() == 1 && this->range_set.get_ranges().front().is_single_char()) return 2;

    return 1;
}

void regex::RegexCharSet::debug(std::ostream& output, const size_t indentation_level) const {
    output << get_indentation(indentation_level) << "CharSet(";
    if(this->negated) output << "^";

    output << this->range_set << ")\n";

}



// namespace functions

std::ostream& regex::operator<<(std::ostream& output, const CharRange& to_print) {
    if(to_print.is_single_char()) {
        return output << (char32_t)to_print.start;
    } else {
        return output << (char32_t)to_print.start << "-" << (char32_t)to_print.end;
    }
}

std::ostream& regex::operator<<(std::ostream& output, const regex::CharRangeSet& to_print) {
    for(const CharRange range : to_print.get_ranges()) {
        assert(("CharRangeSet contains empty range! Please create an issue on github containing the used input!", !range.empty()));
        output << range;
    }

    return output;
}