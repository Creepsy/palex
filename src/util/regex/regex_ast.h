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
        static CharRange common_subset(const CharRange first, const CharRange second);

        bool operator==(const CharRange other) const;
    };

    class RegexBase {
        private:
        public:
            RegexBase();
            virtual ~RegexBase() = default;
            virtual void debug(std::ostream& output, const size_t indentation_level = 0) const = 0;
    };

    class RegexBranch : public RegexBase {
        private:
            std::vector<std::unique_ptr<RegexBase>> possibilities;
        public:
            RegexBranch();
            void add_possibility(std::unique_ptr<RegexBase> possibility);
            void debug(std::ostream& output, const size_t indentation_level = 1) const override;
    };

    class RegexSequence : public RegexBase {
        private:
            std::vector<std::unique_ptr<RegexBase>> sequence;
        public:
            RegexSequence();
            void append_element(std::unique_ptr<RegexBase> to_append);
            void debug(std::ostream& output, const size_t indentation_level = 1) const override;
    };

    class RegexQuantifier : public RegexBase {
        private:
            size_t min_count;
            size_t max_count;

            std::unique_ptr<RegexBase> operand;
        public:
            const static size_t INFINITE;

            RegexQuantifier(std::unique_ptr<RegexBase> operand, const size_t min_count, const size_t max_count);
            void debug(std::ostream& output, const size_t indentation_level = 1) const override;
    };

    class RegexCharSet : public RegexBase {
        private:
            const bool negated;
            
            std::list<CharRange> ranges;
        public:
            RegexCharSet(const bool negated);
            RegexCharSet& add_char_range(CharRange to_add);
            void debug(std::ostream& output, const size_t indentation_level = 1) const override;
    };

    std::ostream& operator<<(std::ostream& output, const CharRange& to_print);
}