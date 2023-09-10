#include "PalexLexerAdapter.h"

#include <string_view>

#include "util/utf8.h"

#include "bootstrap/TokenInfo.h"

namespace input {
    PalexLexerAdapter::PalexLexerAdapter(PalexRuleLexer& to_adapt) : lexer(to_adapt), current_token{bootstrap::TokenInfo::TokenType::UNDEFINED, ""} {
    }

    bootstrap::TokenInfo::TokenType PalexLexerAdapter::next_token() {
        this->lexer.next_unignored_token();
        this->current_token = bootstrap::TokenInfo{
            (bootstrap::TokenInfo::TokenType)this->lexer.current_token().type,
            this->lexer.current_token().identifier,
            bootstrap::FilePosition{this->lexer.current_token().begin.line, this->lexer.current_token().begin.column},
            bootstrap::FilePosition{this->lexer.current_token().end.line, this->lexer.current_token().end.column}
        };
        return this->current_token.type;
    }

    const bootstrap::TokenInfo& PalexLexerAdapter::get_token() const {
        return this->current_token;
    }
}