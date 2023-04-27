#include "PalexRuleParser.h"

#include <utility>
#include <sstream>
#include <string>
#include <cassert>

#include "util/palex_except.h"
#include "util/utf8.h"

#include "regex/RegexParser.h"

// helper functions
std::u32string strip_ends(const std::u32string& to_strip);

std::u32string strip_ends(const std::u32string& to_strip) {
    return to_strip.substr(1, to_strip.length() - 2);
}

namespace input {
    PalexRuleParser::PalexRuleParser(PalexRuleLexer& lexer) : lexer(lexer) {
        this->consume();
    }

    PalexRules PalexRuleParser::parse_all_rules() {
        std::vector<lexer_generator::TokenDefinition> token_definitions = this->parse_all_token_definitions(); // not const because of unique_ptr move
        const std::vector<parser_generator::Production> productions = this->parse_all_productions();
        assert(this->accept(Token::TokenType::END_OF_FILE) && "Bug: Parser didn't reach EOF after parsing finished!");
        return PalexRules{std::move(token_definitions), productions};
    }

    std::vector<lexer_generator::TokenDefinition> PalexRuleParser::parse_all_token_definitions() {
        std::vector<lexer_generator::TokenDefinition> token_definitions;
        std::optional<lexer_generator::TokenDefinition> next_token_definition = this->try_parse_token_definition();
        while (next_token_definition.has_value()) {
            token_definitions.push_back(std::move(next_token_definition.value()));
            next_token_definition = this->try_parse_token_definition();
        }
        return token_definitions;        
    }

    std::vector<parser_generator::Production> PalexRuleParser::parse_all_productions() {
        std::vector<parser_generator::Production> productions;
        std::optional<parser_generator::Production> next_production = this->try_parse_production();
        while (next_production.has_value()) {
            productions.push_back(next_production.value());
            next_production = this->try_parse_production();
        }
        return productions;    
    }

    std::optional<lexer_generator::TokenDefinition> PalexRuleParser::try_parse_token_definition() {
        if (
            this->accept(Token::TokenType::END_OF_FILE) || 
            this->accept(Token::TokenType::PRODUCTION) || 
            this->accept(Token::TokenType::ENTRY_PRODUCTION)
        ) {
            return std::nullopt;
        }
        lexer_generator::TokenDefinition parsed{};
        parsed.ignore_token = this->consume_if(Token::TokenType::IGNORE);
        const bool user_defined_priority = this->accept(Token::TokenType::PRIORITY_TAG);
        if (user_defined_priority) {
            const Token priority_tag = this->consume();
            const std::string priority_str = utf8::unicode_to_utf8(strip_ends(priority_tag.identifier)); // remove <> of tag
            parsed.priority = std::stoull(priority_str); 
        }
        this->expect(Token::TokenType::TOKEN);
        parsed.name = utf8::unicode_to_utf8(this->consume().identifier);
        this->consume(Token::TokenType::EQ);
        const std::u32string regex = strip_ends(this->consume(Token::TokenType::REGEX).identifier); // remove "" of regex
        parsed.token_regex = regex::RegexParser(regex).parse_regex();
        this->consume(Token::TokenType::EOL);
        if (!user_defined_priority) {
            parsed.priority = parsed.token_regex->get_priority();
        }
        return parsed;
    }

    std::optional<parser_generator::Production> PalexRuleParser::try_parse_production() {
        if (this->accept(Token::TokenType::END_OF_FILE)) {
            return std::nullopt;
        }    
        if (
            this->accept(Token::TokenType::TOKEN) ||
            this->accept(Token::TokenType::IGNORE) ||
            this->accept(Token::TokenType::PRIORITY_TAG)
        ) {
            this->throw_error(
                "Found the beginning of a token definition in the production block!."
                " All token definitions have to stand before the first production."
            );
        }
        parser_generator::Production parsed{};
        if (!this->accept(Token::TokenType::PRODUCTION) && !this->accept(Token::TokenType::ENTRY_PRODUCTION)) {
            this->expect(Token::TokenType::PRODUCTION); // error because no production was given
        }
        parsed.name = utf8::unicode_to_utf8(this->consume().identifier);
        this->consume(Token::TokenType::EQ);
        parsed.symbols = this->parse_symbol_sequence();
        this->consume(Token::TokenType::EOL);
        return parsed;
    }

    PalexRuleParser::~PalexRuleParser() {
    }

    std::vector<parser_generator::Symbol> PalexRuleParser::parse_symbol_sequence() {
        using parser_generator::Symbol;

        std::vector<Symbol> symbol_sequence;
        while (this->accept(Token::TokenType::PRODUCTION) || this->accept(Token::TokenType::TOKEN)) {
            symbol_sequence.push_back(
                parser_generator::Symbol{
                    (this->accept(Token::TokenType::TOKEN)) ? Symbol::SymbolType::TERMINAL : Symbol::SymbolType::NONTERMINAL,
                    utf8::unicode_to_utf8(this->curr.identifier)
                }
            );
            this->consume();
        }
        return symbol_sequence;
    }

    void PalexRuleParser::expect(const Token::TokenType to_expect) const {
        if (!this->accept(to_expect)) {
            std::stringstream error_message;
            error_message << "Invalid token '" << this->curr.identifier 
                    << "' of type " << this->curr.type << ". Expected token of type " << to_expect << "!";
            this->throw_error(error_message.str());
        }
    }

    bool PalexRuleParser::accept(const Token::TokenType to_check) const {
        return this->curr.type == to_check;
    }

    Token PalexRuleParser::consume() {
        Token consumed = this->curr;
        this->curr = this->lexer.next_unignored_token();
        return consumed;
    }

    Token PalexRuleParser::consume(const Token::TokenType to_expect) {
        this->expect(to_expect);
        return this->consume();
    }

    bool PalexRuleParser::consume_if(const Token::TokenType to_check) {
        if (this->accept(to_check)) {
            this->consume();
            return true;
        }
        return false;
    }

    void PalexRuleParser::throw_error(const std::string& message) const {
        std::stringstream error_message;
        error_message << this->curr.position << " " << message << std::endl;
        throw palex_except::ParserError(error_message.str());
    }
}
