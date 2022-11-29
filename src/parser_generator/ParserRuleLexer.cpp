#include "ParserRuleLexer.h"

#include <array>

#include "util/utf8.h"

constexpr size_t ERROR_STATE = (size_t)-1;

void parser_generator::CharacterPosition::advance(const char32_t consumed) {
    if(consumed == (char32_t)'\n') {
        this->line++;
        this->column = 1;
    } else {
        this->column++;
    }
}


std::array<std::string, 7> TOKEN_TYPE_TO_STRING {
    "UNDEFINED",
	"END_OF_FILE",
	"PRODUCTION",
	"TOKEN",
	"EQ",
	"EOL",
	"WSPACE"
};

bool parser_generator::Token::is_ignored() const {
    return this->type > TokenType::EOL;
}

parser_generator::ParserRuleLexer::ParserRuleLexer(std::istream& input) : input{input}, curr_position{} {
}

parser_generator::Token parser_generator::ParserRuleLexer::next_token() {
    this->fallback = std::nullopt;
    const CharacterPosition token_start = this->curr_position;
	size_t state = 0;
	std::u32string identifier = U"";
	char32_t curr = this->get_char();

	if(this->end()) {
		return Token{Token::TokenType::END_OF_FILE, U"", token_start};
	}

    while(true) {
		this->cache.push(curr);

		switch(state) {
			case 0:
				switch(curr) {
					case 97 ... 122:
						state = 1;
						break;
					case 60:
						state = 3;
						break;
					case 61:
						state = 7;
						break;
					case 59:
						state = 8;
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
						state = 9;
						break;
					default:
						state = ERROR_STATE;
						break;
				}
				break;
			case 1:
				this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::PRODUCTION};
				switch(curr) {
					case 48 ... 57:
					case 95:
					case 97 ... 122:
						state = 2;
						break;
					default:
						return Token{Token::TokenType::PRODUCTION, identifier, token_start};
				}
				break;
			case 2:
				this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::PRODUCTION};
				switch(curr) {
					case 48 ... 57:
					case 95:
					case 97 ... 122:
						state = 2;
						break;
					default:
						return Token{Token::TokenType::PRODUCTION, identifier, token_start};
				}
				break;
			case 3:
				switch(curr) {
					case 65 ... 90:
						state = 4;
						break;
					default:
						state = ERROR_STATE;
						break;
				}
				break;
			case 4:
				switch(curr) {
					case 48 ... 57:
					case 65 ... 90:
					case 95:
						state = 5;
						break;
					case 62:
						state = 6;
						break;
					default:
						state = ERROR_STATE;
						break;
				}
				break;
			case 5:
				switch(curr) {
					case 48 ... 57:
					case 65 ... 90:
					case 95:
						state = 5;
						break;
					case 62:
						state = 6;
						break;
					default:
						state = ERROR_STATE;
						break;
				}
				break;
			case 6:
				return Token{Token::TokenType::TOKEN, identifier, token_start};
			case 7:
				return Token{Token::TokenType::EQ, identifier, token_start};
			case 8:
				return Token{Token::TokenType::EOL, identifier, token_start};
			case 9:
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
						state = 10;
						break;
					default:
						return Token{Token::TokenType::WSPACE, identifier, token_start};
				}
				break;
			case 10:
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
						state = 10;
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

parser_generator::Token parser_generator::ParserRuleLexer::next_unignored_token() {
    Token unignored{};

    do {
        unignored = this->next_token();
    } while(unignored.is_ignored());

    return unignored;
}

bool parser_generator::ParserRuleLexer::end() const {
    return this->input.eof() && this->cache.empty();
}

char32_t parser_generator::ParserRuleLexer::get_char() {
    if(!this->cache.empty()) {
        char32_t cached = this->cache.top();
        this->cache.pop();

        return cached;
    }

    return utf8::get_unicode_char(this->input);
}

parser_generator::Token parser_generator::ParserRuleLexer::try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start) {
	if(!this->fallback.has_value()) return Token{Token::TokenType::UNDEFINED, token_identifier, token_start};

	while(token_identifier.size() > this->fallback.value().token_length) {
		this->cache.push(token_identifier.back());
		token_identifier.pop_back();
	}

	this->curr_position = this->fallback.value().next_token_position;

	return Token{this->fallback.value().type, token_identifier, token_start};
}

std::ostream& parser_generator::operator<<(std::ostream& output, const CharacterPosition& to_print) {
    return output << "[Ln " << to_print.line << ", Col " << to_print.column << "]";
}

std::ostream& parser_generator::operator<<(std::ostream& output, const Token& to_print) {
    return output << to_print.position << " " << to_print.type << " " << to_print.identifier;
}

std::ostream& parser_generator::operator<<(std::ostream& output, const Token::TokenType to_print) {
    return output << TOKEN_TYPE_TO_STRING.at((size_t)to_print);
}