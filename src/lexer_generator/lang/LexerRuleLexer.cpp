#include "LexerRuleLexer.h"

#include <stdexcept>

#include "util/utf8.h"

// static variables

const std::vector<std::string> lexer_generator::Token::TOKEN_TYPE_NAMES = {
    "UNDEFINED",
    "EOF",
    "IGNORE",
    "IDENTIFIER",
    "EQ",
    "REGEX",
    "ANGLE_PARENTHESIS_OPEN",
    "ANGLE_PARENTHESIS_CLOSE",
    "INTEGER",
    "EOL"
};



lexer_generator::FilePosition& lexer_generator::FilePosition::advance(const char32_t to_process) {
    if (to_process == '\n') {
        this->line++;
        this->column = 1;
    } else {
        this->column++;
    }

    return *this;
}

lexer_generator::FilePosition& lexer_generator::FilePosition::advance(const std::u32string& text) {
    for (const char32_t to_process : text) {
        this->advance(to_process);
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
    if (!this->input.fail()) {
        start.column--; //remove buffered char from position
    } else {
        return Token{Token::TokenType::END_OF_FILE, {}, start, start};
    }

    char32_t next_char = this->next_char(); //char already buffered, so we can skip eof check
    identifier += next_char;

    if (next_char == '=') {
        type = Token::TokenType::EQUALS;
    } else if (next_char == ';') {
        type = Token::TokenType::END_OF_LINE;
    } else if (next_char == '$') {
        type = Token::TokenType::IGNORE;
    } else if (next_char == '<') {
        type = Token::TokenType::ANGLE_PARENTHESIS_OPEN;
    } else if (next_char == '>') {
        type = Token::TokenType::ANGLE_PARENTHESIS_CLOSE;
    } else if (next_char == '"') {
        if (this->get_regex(identifier)) {
            type = Token::TokenType::REGEX;
        }
    } else if ((std::isalpha((int)next_char) && std::isupper((int)next_char)) || next_char == '_') {
        type = Token::TokenType::IDENTIFER;
        this->get_identifier(identifier);
    } else if (std::isdigit((int)next_char)) {
        type = Token::TokenType::INTEGER;
        this->get_integer(identifier);
    }

    FilePosition end = FilePosition{start}.advance(identifier);

    return Token{type, identifier, start, end};
}

//private

char32_t lexer_generator::LexerRuleLexer::next_char() {
    if (this->has_buffered_char) {
        this->has_buffered_char = false;

        return this->buffer;
    }

    char32_t next = utf8::get_unicode_char(this->input);
    if (this->input.good()) {
        this->curr_pos.advance(next);
    }

    return next;
}

char32_t lexer_generator::LexerRuleLexer::consume_wspace() {
    char32_t next;

    do {
        next = this->next_char();
    } while (std::iswspace(next) && this->input.good());

    this->insert_in_buffer(next);

    return next;
}

bool lexer_generator::LexerRuleLexer::get_regex(std::u32string& output) {
    while (this->input.good()) {
        char32_t next = this->next_char();
        output += next;

        if (next == '"') {
            break;
        } 
        if (next == '\n') {
            return false;
        }

        if (next == '\\') {
            output += this->next_char();
        } 
    }

    if (this->input.eof()) {
        output.pop_back(); //remove eof char
        
        return false;
    }

    return true;
}

void lexer_generator::LexerRuleLexer::get_identifier(std::u32string& output) {
    output += this->get_matching_sequence([](const char32_t to_check) -> bool {
        return (std::isupper((int)to_check) && std::isalpha((int)to_check)) || std::isdigit((int)to_check) || to_check == '_';
    });
}

void lexer_generator::LexerRuleLexer::get_integer(std::u32string& output) {
    output += this->get_matching_sequence([](const char32_t to_check) -> bool {
        return std::isdigit((int)to_check);
    });

}

std::u32string lexer_generator::LexerRuleLexer::get_matching_sequence(bool(*predicate)(const char32_t)) {
    std::u32string output;

    while (this->input.good()) {
        char32_t next = this->next_char();

        if (predicate(next)) {
            output += next;
        } else {
            this->insert_in_buffer(next);
            break;
        }
    }

    return output;
}

void lexer_generator::LexerRuleLexer::insert_in_buffer(char32_t to_insert) {
    if (this->has_buffered_char) {
        throw std::runtime_error("Tried to insert char into buffer, but it is already occupied!");
    }
    
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