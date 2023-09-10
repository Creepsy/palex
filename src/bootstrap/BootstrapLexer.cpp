#include "BootstrapLexer.h"

#include <cstring>
#include <cassert>
#include <iostream>

constexpr utf8::Codepoint_t LAST_UNSIGNED_CHAR = 0xff;

namespace bootstrap {
    void FilePosition::advance(const utf8::Codepoint_t consumed) {
        this->column++;
        if (consumed == (utf8::Codepoint_t)'\n') {
            this->column = 1;
            this->line++;
        }
    }

    BootstrapLexer::BootstrapLexer(const std::string_view input) : input{input}, position{input.begin()}, file_position{}, current_token{TokenInfo::TokenType::UNDEFINED, ""} {
    }

    TokenInfo::TokenType BootstrapLexer::next_token() {
        this->current_token.begin = this->file_position;
        this->current_token.type = TokenInfo::TokenType::UNDEFINED;
        const char* token_position = this->position;
        this->advance_token();
        this->current_token.identifier = std::string_view(token_position, this->position - token_position);
        this->current_token.end = this->file_position;
        return this->current_token.type; 
    }

    TokenInfo::TokenType BootstrapLexer::next_unignored_token() {
        do {
            this->next_token();
        } while (this->get_token().type == TokenInfo::TokenType::WSPACE);
        return this->current_token.type;
    }

    const TokenInfo& BootstrapLexer::get_token() const {
        return this->current_token;
    }

    bool BootstrapLexer::try_ascii_constant(const char* to_try, const TokenInfo::TokenType token_type) {
        const size_t identifier_size = std::strlen(to_try);
        if (!std::strncmp(this->position, to_try, identifier_size)) {
            this->current_token.type = token_type;
            this->advance_codepoints(identifier_size);
            return true;
        }
        return false;
    }

    void BootstrapLexer::advance_codepoints(const size_t count) {
        utf8::Codepoint_t curr;
        for (int i = 0; i < count; i++) {
            this->position = utf8::advance_codepoint(this->position, this->input.end(), &curr);
            this->file_position.advance(curr);
        }
    }

    void BootstrapLexer::advance_token() {
        if (*this->position == '\0') {
            this->current_token.type = TokenInfo::TokenType::END_OF_FILE;
            return;
        }

        if (this->try_ascii_constant("!", TokenInfo::TokenType::IGNORE)) {
            return;
        }
        if (this->try_ascii_constant("=", TokenInfo::TokenType::EQ)) {
            return;
        }
        if (this->try_ascii_constant(";", TokenInfo::TokenType::EOL)) {
            return;
        }
        if (this->try_ascii_constant("$S", TokenInfo::TokenType::ENTRY_PRODUCTION)) {
            return;
        }

        const utf8::Codepoint_t next = utf8::get_next_codepoint(this->position, this->input.end());
        if (std::isspace((int)next)) {
            this->advance_while(
                [](const utf8::Codepoint_t to_check) -> bool {
                    return to_check <= LAST_UNSIGNED_CHAR && std::isspace((int)to_check);
                }
            );
            this->current_token.type = TokenInfo::TokenType::WSPACE;
        } else if (std::isupper((int)next)) {
            this->advance_while(
                [](const utf8::Codepoint_t to_check) -> bool {
                    return to_check <= LAST_UNSIGNED_CHAR && (
                           std::isupper((int)to_check) || 
                           std::isdigit((int)to_check) || 
                           to_check == (utf8::Codepoint_t)'_');
                }
            );
            this->current_token.type = TokenInfo::TokenType::TOKEN;
        } else if (std::islower((int)next)) {
            this->advance_while(
                [](const utf8::Codepoint_t to_check) -> bool {
                    return to_check <= LAST_UNSIGNED_CHAR && (
                           std::islower((int)to_check) || 
                           std::isdigit((int)to_check) || 
                           to_check == (utf8::Codepoint_t)'_');
                }
            );
            this->current_token.type = TokenInfo::TokenType::PRODUCTION;
        } else if (next == (utf8::Codepoint_t)'"') {
            this->advance_regex_token();
        } else if (next == (utf8::Codepoint_t)'<') {
            this->advance_priority_token();
        } else {
            // Undefined token, advance by 1
            this->advance_codepoints(1);
        }
    }

    void BootstrapLexer::advance_priority_token() {
        assert(utf8::get_next_codepoint(this->position, this->input.end()) == (utf8::Codepoint_t)'<' && "BUG: Method should only get called when this condition succeeds!");
        this->advance_codepoints(1);
        bool consumed_number = this->advance_while(
            [](const utf8::Codepoint_t to_check) -> bool {
                return to_check <= LAST_UNSIGNED_CHAR && std::isdigit((int)to_check);
            }
        );
        if (!consumed_number) {
            return; // Undefined token, missing number
        }
        if (utf8::get_next_codepoint(this->position, this->input.end()) != (utf8::Codepoint_t)'>') {
            return; // Undefined token, missing closing tag
        }
        this->advance_codepoints(1);
        this->current_token.type = TokenInfo::TokenType::PRIORITY_TAG;
    }

    void BootstrapLexer::advance_regex_token() {
        assert(utf8::get_next_codepoint(this->position, this->input.end()) == (utf8::Codepoint_t)'"' && "BUG: Method should only get called when this condition succeeds!");
        this->advance_codepoints(1);
        while (utf8::get_next_codepoint(this->position, this->input.end()) != (utf8::Codepoint_t)'"' && *this->position != '\0') {
            if (utf8::get_next_codepoint(this->position, this->input.end()) == (utf8::Codepoint_t)'\\') {
                this->advance_codepoints(1); // skip escaped char
            }
            this->advance_codepoints(1);
        }
        if (utf8::get_next_codepoint(this->position, this->input.end()) != (utf8::Codepoint_t)'"') {
            return; // Undefined token, missing closing tag
        }
        this->advance_codepoints(1);
        this->current_token.type = TokenInfo::TokenType::REGEX;
    }

    bool BootstrapLexer::advance_while(bool (*predicate)(const utf8::Codepoint_t)) {
        bool consumed_codepoint = false;
        while (predicate(utf8::get_next_codepoint(this->position, this->input.end()))) {
            this->advance_codepoints(1);
            consumed_codepoint = true;
        }
        return consumed_codepoint;
    }
}