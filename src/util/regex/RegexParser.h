#pragma once

#include <string>
#include <cstddef>
#include <vector>

#include "regex_ast.h"

namespace regex {
    class RegexParser {
        private:
            enum class CharType {
                SQUARE_OPEN,
                SQUARE_CLOSE,
                NEGATION,
                ESCAPE,
                MINUS,
                DOT,
                BRACKET_OPEN,
                BRACKET_CLOSE,
                PLUS,
                STAR,
                CURLY_OPEN,
                CURLY_CLOSE,
                COMMA,
                ALTERNATION,
                OPTIONAL,
                CHARACTER
            };

            const static std::vector<std::string> CHAR_TYPE_NAMES;

            const std::u32string input;
            size_t curr_pos;

            std::unique_ptr<RegexBase> parse_regex_branch();
            std::unique_ptr<RegexBase> parse_regex_sequence();
            std::unique_ptr<RegexBase> parse_regex_quantifier();
            std::unique_ptr<RegexBase> parse_regex_charset();
            std::unique_ptr<RegexBase> parse_regex_group();

            std::vector<CharRange> parse_char(const bool inside_set = false);
            std::vector<CharRange> parse_escaped_char();
            std::vector<CharRange> parse_char_range();

            char32_t parse_unicode_value();

            CharType get_curr_type();
            char32_t get_curr();

            std::u32string parse_matching_chars(bool(*predicate)(const char32_t), const size_t max_count = (size_t) -1);

            void expect(const CharType type);
            bool accept(const CharType type);
            char32_t consume();
            char32_t consume(const CharType type);

            bool end();

            void throw_parsing_err(const CharType expected);
            void throw_parsing_err(const std::string& message);
        public:
            RegexParser(const std::u32string& input);
            std::unique_ptr<RegexBase> parse_regex();
    };
}