#pragma once

#include <ostream>
#include <cstddef>
#include <memory>
#include <vector>
#include <string>
#include <list>

namespace regex {    
    struct CharRange {
        char32_t start;
        char32_t end;

        CharRange();
        CharRange(const char32_t c);
        CharRange(const char32_t start, const char32_t end);

        bool is_single_char() const;
        bool is_empty() const;

        bool is_subset_of(const CharRange other) const;
        static CharRange common_subset(const CharRange first, const CharRange second); //currently unused

        bool operator==(const CharRange other) const;
    };

    class RegexBase {
        private:
        public:
            RegexBase() = default;
            virtual ~RegexBase() = default;
            virtual void debug(std::ostream& output, const size_t indentation_level = 0) const = 0;
    };

    class RegexBranch : public RegexBase {
        private:
            std::vector<std::unique_ptr<RegexBase>> possibilities;
        public:
            RegexBranch();
            void add_possibility(std::unique_ptr<RegexBase> possibility);
            const std::vector<std::unique_ptr<RegexBase>>& get_possibilities() const;
            void debug(std::ostream& output, const size_t indentation_level = 0) const override;
    };

    class RegexSequence : public RegexBase {
        private:
            std::vector<std::unique_ptr<RegexBase>> sequence;
        public:
            RegexSequence();
            void append_element(std::unique_ptr<RegexBase> to_append);
            const std::vector<std::unique_ptr<RegexBase>>& get_elements() const;
            void debug(std::ostream& output, const size_t indentation_level = 0) const override;
    };

    class RegexQuantifier : public RegexBase {
        private:
            size_t min_count;
            size_t max_count;

            std::unique_ptr<RegexBase> operand;
        public:
            const static size_t INFINITE;

            RegexQuantifier(std::unique_ptr<RegexBase> operand, const size_t min_count, const size_t max_count);
            const std::unique_ptr<RegexBase>& get_operand() const;
            size_t get_min() const;
            size_t get_max() const;
            void debug(std::ostream& output, const size_t indentation_level = 0) const override;
    };

    class RegexCharSet : public RegexBase {
        private:
            const bool negated;
            
            std::list<CharRange> ranges;
        public:
            RegexCharSet(const bool negated);
            RegexCharSet& add_char_range(CharRange to_add);
            const std::list<CharRange>& get_characters() const;
            bool is_negated() const;
            void debug(std::ostream& output, const size_t indentation_level = 0) const override;
    };

    std::ostream& operator<<(std::ostream& output, const CharRange& to_print);
}