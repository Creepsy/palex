#pragma once

#include <ostream>
#include <cstddef>
#include <memory>
#include <vector>
#include <list>

#include "util/utf8.h"

namespace regex {    
    struct CharRange {
        utf8::Codepoint_t start;
        utf8::Codepoint_t end;

        CharRange();
        CharRange(const utf8::Codepoint_t single_char);
        CharRange(const utf8::Codepoint_t start, const utf8::Codepoint_t end);

        bool is_single_char() const;
        bool empty() const;
        bool can_prepend_to(const CharRange target) const;
        bool can_append_to(const CharRange target) const;

        bool is_subset_of(const CharRange other) const;

        bool operator==(const CharRange other) const;

        static CharRange common_subset(const CharRange first, const CharRange second);
    };

    class CharRangeSet {
        public:
            CharRangeSet() = default;

            CharRangeSet& insert_char_range(CharRange to_add);
            CharRangeSet& remove_char_range(const CharRange to_remove);

            bool empty() const;

            CharRangeSet get_intersection(const CharRangeSet& other) const;
            std::list<CharRange>& get_ranges();
            const std::list<CharRange>& get_ranges() const;

            CharRangeSet operator-(const CharRangeSet& to_subtract) const;
            CharRangeSet operator+(const CharRangeSet& to_add) const;

            bool operator==(const CharRangeSet& other) const;
            bool operator!=(const CharRangeSet& other) const;
        private:
            std::list<CharRange> ranges;
        };

    std::ostream& operator<<(std::ostream& output, const CharRangeSet& to_print);

    class RegexBase {
        public:
            RegexBase() = default;
            virtual ~RegexBase() = default;
            virtual size_t get_priority() const = 0;
            virtual void debug(std::ostream& output, const size_t indentation_level = 0) const = 0;
    };

    class RegexAlternation : public RegexBase {
        public:
            RegexAlternation() = default;
            ~RegexAlternation() = default;
            void add_branch(std::unique_ptr<RegexBase> branch);
            const std::vector<std::unique_ptr<RegexBase>>& get_branches() const;
            size_t get_priority() const override;
            void debug(std::ostream& output, const size_t indentation_level = 0) const override;
        private:
            std::vector<std::unique_ptr<RegexBase>> branches;
    };

    class RegexSequence : public RegexBase {
        public:
            RegexSequence() = default;
            ~RegexSequence() = default;
            void append_element(std::unique_ptr<RegexBase> to_append);
            const std::vector<std::unique_ptr<RegexBase>>& get_elements() const;
            size_t get_priority() const override;
            void debug(std::ostream& output, const size_t indentation_level = 0) const override;
        private:
            std::vector<std::unique_ptr<RegexBase>> sequence;
    };

    class RegexQuantifier : public RegexBase {
        public:
            const static size_t INFINITE;

            RegexQuantifier(std::unique_ptr<RegexBase> operand, const size_t min_count, const size_t max_count);
            const std::unique_ptr<RegexBase>& get_operand() const;
            size_t get_min() const;
            size_t get_max() const;
            size_t get_priority() const override;
            void debug(std::ostream& output, const size_t indentation_level = 0) const override;
        private:
            size_t min_count;
            size_t max_count;

            std::unique_ptr<RegexBase> operand;
    };

    class RegexCharSet : public RegexBase {
        public:
            RegexCharSet(const bool negated);
            void insert_char_range(CharRange to_insert);
            const CharRangeSet& get_range_set() const;
            bool is_negated() const;
            size_t get_priority() const override;
            void debug(std::ostream& output, const size_t indentation_level = 0) const override;
        private:
            const bool negated;
            
            CharRangeSet range_set;
    };

    std::ostream& operator<<(std::ostream& output, const CharRange& to_print);
}