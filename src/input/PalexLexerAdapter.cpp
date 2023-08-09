#include "PalexLexerAdapter.h"

#include <string_view>

#include "util/utf8.h"

#include "bootstrap/TokenInfo.h"

// helper functions
input::CharacterPosition advance_by(input::CharacterPosition to_advance, const std::u32string& consumed);

input::CharacterPosition advance_by(input::CharacterPosition to_advance, const std::u32string& consumed) {
    for (const char32_t c : consumed) {
        to_advance.advance(c);
    }
    return to_advance;
}

namespace input {
    PalexLexerAdapter::PalexLexerAdapter(PalexRuleLexer& to_adapt) : lexer(to_adapt), current_token{bootstrap::TokenInfo::TokenType::UNDEFINED, ""} {
    }

    bootstrap::TokenInfo::TokenType PalexLexerAdapter::next_token() {
        const Token lexer_token = this->lexer.next_unignored_token();
        const std::string utf8_identifier = utf8::unicode_to_utf8(lexer_token.identifier);
        const CharacterPosition end_position = advance_by(lexer_token.position, lexer_token.identifier);
        const size_t identifier_offset = this->identifier_buffer.size(); 
        this->identifier_buffer += utf8_identifier;
        this->current_token = bootstrap::TokenInfo{
            (bootstrap::TokenInfo::TokenType)lexer_token.type,
            std::string_view(this->identifier_buffer.c_str() + identifier_offset, utf8_identifier.size()),
            bootstrap::FilePosition{lexer_token.position.line, lexer_token.position.column},
            bootstrap::FilePosition{end_position.line, end_position.column}
        };
        return this->current_token.type;
    }

    const bootstrap::TokenInfo& PalexLexerAdapter::get_token() const {
        return this->current_token;
    }
}