#pragma once

#include <string_view>
#include <string>
#include <cstddef>
#include <vector>
#include <utility>

#include "util/utf8.h"

#include "regex_ast.h"

namespace regex {
    class RegexParser {
        public:
            RegexParser(const std::string_view& input);
            std::unique_ptr<RegexBase> parse_regex();
        private:
            enum class CharType {
                BRACKET_OPEN,
                BRACKET_CLOSE,
                NEGATION,
                ESCAPE,
                MINUS,
                DOT,
                PARENTHESIS_OPEN,
                PARENTHESIS_CLOSE,
                PLUS,
                STAR,
                BRACE_OPEN,
                BRACE_CLOSE,
                COMMA,
                ALTERNATION,
                OPTIONAL,
                CHARACTER,
                CHARACTER_CLASS
            };

            typedef bool(*Predicate_t)(const utf8::Codepoint_t);
            typedef std::pair<std::vector<CharRange>, CharType> MultiRangeCharacter;

            const static std::vector<std::string> CHAR_TYPE_NAMES;

            const std::string_view input;
            size_t curr_pos;

            std::unique_ptr<RegexBase> parse_regex_alternation();
            std::unique_ptr<RegexBase> parse_regex_sequence();
            std::unique_ptr<RegexBase> parse_regex_quantifier();
            std::unique_ptr<RegexBase> parse_regex_charset();
            std::unique_ptr<RegexBase> parse_regex_group();

            MultiRangeCharacter parse_char();
            MultiRangeCharacter parse_escaped_char();

            utf8::Codepoint_t parse_unicode_value();

            CharType get_curr_type();
            utf8::Codepoint_t get_curr();

            template<class ParseObject_T>
            std::vector<ParseObject_T> parse_until(Predicate_t predicate, ParseObject_T(RegexParser::*parse_func)(), const size_t max_count = (size_t) - 1);
            std::string_view parse_matching_string(Predicate_t predicate, const size_t max_count = (size_t) -1);

            void expect(const CharType type);
            bool accept(const CharType type);
            utf8::Codepoint_t consume();
            utf8::Codepoint_t consume(const CharType type);

            bool end();

            void throw_parsing_err(const CharType expected);
            void throw_parsing_err(const std::string& message);
    
            static void process_charset_contents(const std::vector<MultiRangeCharacter>& set_contents, RegexCharSet& target);

    };
}

#include "RegexParser.ipp"