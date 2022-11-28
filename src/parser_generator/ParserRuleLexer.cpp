#include "ParserRuleLexer.h"

#include <array>

#include "util/utf8.h"

constexpr size_t ERROR_STATE = (size_t)-1;

void parser_generator::CharacterPosition::advance(const char32_t consumed) {
    if(consumed == (char32_t)'\n') {
        this->line++;
        this->column = 0;
    } else {
        this->column++;
    }
}


std::array<std::string, 3> TOKEN_TYPE_TO_STRING {
    "UNDEFINED",
	"END_OF_FILE",
	"test"
};

bool parser_generator::Token::is_ignored() const {
    return this->type > TokenType::test;
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
					case 116:
						state = 1;
						break;
					default:
						state = ERROR_STATE;
						break;
				}
				break;
			case 1:
				switch(curr) {
					case 101:
						state = 2;
						break;
					default:
						state = ERROR_STATE;
						break;
				}
				break;
			case 2:
				switch(curr) {
					case 115:
						state = 3;
						break;
					default:
						state = ERROR_STATE;
						break;
				}
				break;
			case 3:
				switch(curr) {
					case 116:
						state = 4;
						break;
					default:
						state = ERROR_STATE;
						break;
				}
				break;
			case 4:
				return Token{Token::TokenType::test, identifier, token_start};
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
    return output << to_print.position << " " << TOKEN_TYPE_TO_STRING.at((size_t)to_print.type) << " " << to_print.identifier;
}