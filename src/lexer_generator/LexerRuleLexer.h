#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <cstddef>
#include <vector>

namespace lexer_generator {
    struct FilePosition {
        size_t line = 1;
        size_t column = 1;

        FilePosition& advance(const char32_t c);
        FilePosition& advance(const std::u32string& text);
    };
    
    struct Token {
        enum class TokenType {
            UNDEFINED,
            END_OF_FILE,
            IGNORE,
            IDENTIFER,
            EQUALS,
            REGEX,
            ANGLE_BRACKET_OPEN,
            ANGLE_BRACKET_CLOSE,
            INTEGER,
            END_OF_LINE
        };

        const static std::vector<std::string> TOKEN_TYPE_NAMES;

        TokenType type;
        std::u32string identifier;
        
        FilePosition start;
        FilePosition end;
    };
    
    class LexerRuleLexer {
        public:
            LexerRuleLexer(std::istream& input);
            Token next_token();
        private:
            std::istream& input;

            bool has_buffered_char;
            char32_t buffer;

            FilePosition curr_pos;

            char32_t next_char();
            char32_t consume_wspace();
            
            bool get_regex(std::u32string& output);
            void get_identifier(std::u32string& output);
            void get_integer(std::u32string& output);

            std::u32string get_matching_sequence(bool(*predicate)(const char32_t));
            void insert_in_buffer(char32_t to_insert);
    };

    std::ostream& operator<<(std::ostream& stream, const FilePosition& to_print);
    std::ostream& operator<<(std::ostream& stream, const Token& to_print);
}