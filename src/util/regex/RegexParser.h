#pragma once

#include <string>
#include <cstddef>
#include <vector>
#include <utility>

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
                CHARACTER,
                CHARACTER_CLASS
            };


            typedef bool(*Predicate_t)(const char32_t);
            typedef std::pair<std::vector<CharRange>, CharType> MultiRangeCharacter;

            const static std::vector<std::string> CHAR_TYPE_NAMES;

            const std::u32string input;
            size_t curr_pos;

            std::unique_ptr<RegexBase> parse_regex_branch();
            std::unique_ptr<RegexBase> parse_regex_sequence();
            std::unique_ptr<RegexBase> parse_regex_quantifier();
            std::unique_ptr<RegexBase> parse_regex_charset();
            std::unique_ptr<RegexBase> parse_regex_group();

            MultiRangeCharacter parse_char();
            MultiRangeCharacter parse_escaped_char();

            void process_charset_contents(const std::vector<MultiRangeCharacter>& set_contents, RegexCharSet& target);

            char32_t parse_unicode_value();

            CharType get_curr_type();
            char32_t get_curr();

            template<class ParseObject_T>
            std::vector<ParseObject_T> parse_until(Predicate_t predicate, ParseObject_T(RegexParser::*parse_func)(), const size_t max_count = (size_t) - 1);
            std::u32string parse_matching_string(Predicate_t predicate, const size_t max_count = (size_t) -1);

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

#include "RegexParser.ipp"