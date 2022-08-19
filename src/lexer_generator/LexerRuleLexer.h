#pragma once

#include <istream>
#include <string>
#include <cstddef>
#include <map>

namespace lexer_generator {
    struct FilePosition {
        size_t line;
        size_t column;
    };
    
    struct Token {
        enum TokenType {
            UNDEFINED,
            END_OF_FILE,
            IDENTIFER,
            EQUALS,
            REGEX,
            END_OF_LINE
        };

        const static std::map<TokenType, std::string> TYPE_NAMES;

        TokenType type;
        std::u32string identifier;
        
        FilePosition start;
        FilePosition end;
    };
    
    class LexerRuleLexer {
        private:
            std::istream& input;

            FilePosition curr_pos;

            char32_t next_char();
        public:
            LexerRuleLexer(std::istream& input);
            Token next_token();
    };
}