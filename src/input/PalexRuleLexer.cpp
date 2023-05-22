#include "PalexRuleLexer.h"

#include <array>

#include "../util/utf8.h"

constexpr size_t ERROR_STATE = (size_t)-1;

void input::CharacterPosition::advance(const char32_t consumed) {
    if (consumed == (char32_t)'\n') {
        this->line++;
        this->column = 1;
    } else {
        this->column++;
    }
}


const std::array<std::string, 11> TOKEN_TYPE_TO_STRING {
    "UNDEFINED",
    "END_OF_FILE",
    "IGNORE",
    "REGEX",
    "PRIORITY_TAG",
    "ENTRY_PRODUCTION",
    "PRODUCTION",
    "TOKEN",
    "EQ",
    "EOL",
    "WSPACE"
};

bool input::Token::is_ignored() const {
    return this->type > TokenType::EOL;
}

input::PalexRuleLexer::PalexRuleLexer(std::istream& input) : input{input}, curr_position{} {
}

input::Token input::PalexRuleLexer::next_token() {
    this->fallback = std::nullopt;
    const CharacterPosition token_start = this->curr_position;
    size_t state = 0;
    std::u32string identifier = U"";
    char32_t curr = this->get_char();

    if (this->end()) {
        return Token{Token::TokenType::END_OF_FILE, U"", token_start};
    }

    while (true) {
        this->cache.push(curr);

        switch(state) {
            case 0:
                switch(curr) {
                    case 33:
                        state = 1;
                        break;
                    case 34:
                        state = 2;
                        break;
                    case 60:
                        state = 7;
                        break;
                    case 36:
                        state = 11;
                        break;
                    case 97 ... 122:
                        state = 13;
                        break;
                    case 65 ... 90:
                        state = 15;
                        break;
                    case 61:
                        state = 17;
                        break;
                    case 59:
                        state = 18;
                        break;
                    case 9 ... 13:
                    case 32:
                    case 133:
                    case 160:
                    case 5760:
                    case 8192 ... 8202:
                    case 8232 ... 8233:
                    case 8239:
                    case 8297:
                    case 12288:
                        state = 19;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 1:
                return Token{Token::TokenType::IGNORE, identifier, token_start};
            case 2:
                switch(curr) {
                    case 0 ... 9:
                    case 11 ... 33:
                    case 35 ... 91:
                    case 93 ... 1114111:
                        state = 3;
                        break;
                    case 92:
                        state = 4;
                        break;
                    case 34:
                        state = 6;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 3:
                switch(curr) {
                    case 0 ... 9:
                    case 11 ... 33:
                    case 35 ... 91:
                    case 93 ... 1114111:
                        state = 3;
                        break;
                    case 92:
                        state = 4;
                        break;
                    case 34:
                        state = 6;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 4:
                switch(curr) {
                    case 0 ... 9:
                    case 11 ... 12:
                    case 14 ... 1114111:
                        state = 5;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 5:
                switch(curr) {
                    case 0 ... 9:
                    case 11 ... 33:
                    case 35 ... 91:
                    case 93 ... 1114111:
                        state = 3;
                        break;
                    case 92:
                        state = 4;
                        break;
                    case 34:
                        state = 6;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 6:
                return Token{Token::TokenType::REGEX, identifier, token_start};
            case 7:
                switch(curr) {
                    case 48 ... 57:
                        state = 8;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 8:
                switch(curr) {
                    case 48 ... 57:
                        state = 9;
                        break;
                    case 62:
                        state = 10;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 9:
                switch(curr) {
                    case 48 ... 57:
                        state = 9;
                        break;
                    case 62:
                        state = 10;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 10:
                return Token{Token::TokenType::PRIORITY_TAG, identifier, token_start};
            case 11:
                switch(curr) {
                    case 83:
                        state = 12;
                        break;
                    default:
                        state = ERROR_STATE;
                        break;
                }
                break;
            case 12:
                return Token{Token::TokenType::ENTRY_PRODUCTION, identifier, token_start};
            case 13:
                this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::PRODUCTION};
                switch(curr) {
                    case 48 ... 57:
                    case 95:
                    case 97 ... 122:
                        state = 14;
                        break;
                    default:
                        return Token{Token::TokenType::PRODUCTION, identifier, token_start};
                }
                break;
            case 14:
                this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::PRODUCTION};
                switch(curr) {
                    case 48 ... 57:
                    case 95:
                    case 97 ... 122:
                        state = 14;
                        break;
                    default:
                        return Token{Token::TokenType::PRODUCTION, identifier, token_start};
                }
                break;
            case 15:
                this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::TOKEN};
                switch(curr) {
                    case 48 ... 57:
                    case 65 ... 90:
                    case 95:
                        state = 16;
                        break;
                    default:
                        return Token{Token::TokenType::TOKEN, identifier, token_start};
                }
                break;
            case 16:
                this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::TOKEN};
                switch(curr) {
                    case 48 ... 57:
                    case 65 ... 90:
                    case 95:
                        state = 16;
                        break;
                    default:
                        return Token{Token::TokenType::TOKEN, identifier, token_start};
                }
                break;
            case 17:
                return Token{Token::TokenType::EQ, identifier, token_start};
            case 18:
                return Token{Token::TokenType::EOL, identifier, token_start};
            case 19:
                this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::WSPACE};
                switch(curr) {
                    case 9 ... 13:
                    case 32:
                    case 133:
                    case 160:
                    case 5760:
                    case 8192 ... 8202:
                    case 8232 ... 8233:
                    case 8239:
                    case 8297:
                    case 12288:
                        state = 20;
                        break;
                    default:
                        return Token{Token::TokenType::WSPACE, identifier, token_start};
                }
                break;
            case 20:
                this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::WSPACE};
                switch(curr) {
                    case 9 ... 13:
                    case 32:
                    case 133:
                    case 160:
                    case 5760:
                    case 8192 ... 8202:
                    case 8232 ... 8233:
                    case 8239:
                    case 8297:
                    case 12288:
                        state = 20;
                        break;
                    default:
                        return Token{Token::TokenType::WSPACE, identifier, token_start};
                }
                break;
            case ERROR_STATE:
                return this->try_restore_fallback(identifier, token_start);
        }

        identifier += this->cache.top();
        this->curr_position.advance(this->cache.top());
        this->cache.pop();
        curr = this->get_char();
    }
}

input::Token input::PalexRuleLexer::next_unignored_token() {
    Token unignored{};

    do {
        unignored = this->next_token();
    } while (unignored.is_ignored());

    return unignored;
}

bool input::PalexRuleLexer::end() const {
    return this->input.eof() && this->cache.empty();
}

char32_t input::PalexRuleLexer::get_char() {
    if (!this->cache.empty()) {
        char32_t cached = this->cache.top();
        this->cache.pop();

        return cached;
    }

    return utf8::get_unicode_char(this->input);
}

input::Token input::PalexRuleLexer::try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start) {
    if (!this->fallback.has_value()) return Token{Token::TokenType::UNDEFINED, token_identifier, token_start};

    while (token_identifier.size() > this->fallback.value().token_length) {
        this->cache.push(token_identifier.back());
        token_identifier.pop_back();
    }

    this->curr_position = this->fallback.value().next_token_position;

    return Token{this->fallback.value().type, token_identifier, token_start};
}

std::ostream& input::operator<<(std::ostream& output, const CharacterPosition& to_print) {
    return output << "[Ln " << to_print.line << ", Col " << to_print.column << "]";
}

std::ostream& input::operator<<(std::ostream& output, const Token& to_print) {
    return output << to_print.position << " " << to_print.type << " " << to_print.identifier;
}

std::ostream& input::operator<<(std::ostream& output, const Token::TokenType to_print) {
    return output << TOKEN_TYPE_TO_STRING.at((size_t)to_print);
}