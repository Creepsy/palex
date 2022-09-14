#include "LexerRuleLexer.h"

#include <stdexcept>

#include "util/unicode.h"

// static variables

const std::vector<std::string> lexer_generator::Token::TOKEN_TYPE_NAMES = {
    "UNDEFINED",
    "EOF",
    "IGNORE",
    "IDENTIFIER",
    "EQ",
    "REGEX",
    "EOL"
};



lexer_generator::FilePosition& lexer_generator::FilePosition::advance(const char32_t c) {
    if(c == '\n') {
        this->line++;
        this->column = 1;
    } else {
        this->column++;
    }

    return *this;
}

lexer_generator::FilePosition& lexer_generator::FilePosition::advance(const std::u32string& text) {
    for(const char32_t c : text) {
        this->advance(c);
    }

    return *this;
}



lexer_generator::LexerRuleLexer::LexerRuleLexer(std::istream& input) : input(input), curr_pos{}, has_buffered_char(false) {
}

//public

lexer_generator::Token lexer_generator::LexerRuleLexer::next_token() {
    Token::TokenType type = Token::TokenType::UNDEFINED;
    std::u32string identifier;

    this->consume_wspace();
    FilePosition start = this->curr_pos;
    if(!this->input.fail()) {
        start.column--; //remove buffered char from position
    } else {
        return Token{Token::TokenType::END_OF_FILE, {}, start, start};
    }

    char32_t next_char = this->next_char(); //char already buffered, so we can skip eof check
    identifier += next_char;

    if(next_char == '=') {
        type = Token::TokenType::EQUALS;
    } else if(next_char == ';') {
        type = Token::TokenType::END_OF_LINE;
    } else if(next_char == '$') {
        type = Token::TokenType::IGNORE;
    } else if(next_char == '"') {
        if(this->get_regex(identifier)) type = Token::TokenType::REGEX;
        
    } else if(std::isalpha(next_char) | next_char == '_') {
        if(this->get_identifier(identifier)) type = Token::TokenType::IDENTIFER; 
    }

    FilePosition end = FilePosition{start}.advance(identifier);

    return Token{type, identifier, start, end};
}

//private

char32_t lexer_generator::LexerRuleLexer::next_char() {
    if(this->has_buffered_char) {
        this->has_buffered_char = false;

        return this->buffer;
    }

    char32_t next = unicode::get_utf8(this->input);
    if(this->input.good()) this->curr_pos.advance(next);

    return next;
}

char32_t lexer_generator::LexerRuleLexer::consume_wspace() {
    char32_t next;

    do {
        next = this->next_char();
    } while(std::iswspace(next) && this->input.good());

    this->insert_in_buffer(next);

    return next;
}

bool lexer_generator::LexerRuleLexer::get_regex(std::u32string& output) {
    while(this->input.good()) {
        char32_t next = this->next_char();
        output += next;

        if(next == '"') {
            break;
        } else if(next == '\\') {
            output += this->next_char();
        } else if(next == '\n') {
            return false;
        }
    }

    if(this->input.eof()) {
        output.pop_back(); //remove eof char
        
        return false;
    }

    return true;
}

bool lexer_generator::LexerRuleLexer::get_identifier(std::u32string& output) {
    while(this->input.good()) {
        char32_t next = this->next_char();

        if(std::isalnum(next) || next == '_') {
            output += next;
        } else {
            this->insert_in_buffer(next); //char is part of the next token
            break;
        }
    }

    return true;
}

void lexer_generator::LexerRuleLexer::insert_in_buffer(char32_t to_insert) {
    if(this->has_buffered_char) throw std::runtime_error("Tried to insert char into buffer, but it is already occupied!");

    this->has_buffered_char = true;
    this->buffer = to_insert;
}



//namespace functions

std::ostream& lexer_generator::operator<<(std::ostream& stream, const FilePosition& to_print) {
    return stream << "[Ln " << to_print.line << ", Col " << to_print.column << "]";
}

std::ostream& lexer_generator::operator<<(std::ostream& stream, const Token& to_print) {
    return stream << to_print.start << "-" << to_print.end << " (" << Token::TOKEN_TYPE_NAMES.at((size_t)to_print.type) << ") " << to_print.identifier;
}