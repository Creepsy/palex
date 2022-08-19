#include "LexerRuleLexer.h"

#include "util/encoding.h"

using namespace lexer_generator;

const std::map<Token::TokenType, std::string> lexer_generator::Token::TYPE_NAMES = {
    {Token::TokenType::UNDEFINED, "undefined"},
    {Token::TokenType::END_OF_FILE, "eof"},
    {Token::TokenType::IDENTIFER, "identifier"},
    {Token::TokenType::EQUALS, "eq"},
    {Token::TokenType::REGEX, "regex"},
    {Token::TokenType::END_OF_LINE, "eol"}
};

lexer_generator::LexerRuleLexer::LexerRuleLexer(std::istream& input) : input(input) {
}



//public

Token lexer_generator::LexerRuleLexer::next_token() {
    //TODO
}



//private

char32_t lexer_generator::LexerRuleLexer::next_char() {
    char32_t next = encoding::get_utf8(this->input);

    if(next == '\n') {
        this->curr_pos.line++;
        this->curr_pos.column = 0;
    } else {
        this->curr_pos.column++;
    }

    return next;
}