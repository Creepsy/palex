#include "ParserRuleParser.h"

#include <sstream>

#include "util/utf8.h"

parser_generator::ParserRuleParser::ParserRuleParser(ParserRuleLexer& input) : input(input) {
    this->consume();
}

std::optional<parser_generator::Production> parser_generator::ParserRuleParser::parse_production() {
    if (this->accept(Token::TokenType::END_OF_FILE)) {
        return std::nullopt;
    }

    Production parsed{};
    
    parsed.name = utf8::unicode_to_utf8(this->consume(Token::TokenType::PRODUCTION).identifier);
    this->consume(Token::TokenType::EQ);

    while (this->accept(Token::TokenType::PRODUCTION) || this->accept(Token::TokenType::TOKEN)) {
        if (this->accept(Token::TokenType::PRODUCTION)) {
            parsed.symbols.push_back(Symbol{Symbol::SymbolType::NONTERMINAL, utf8::unicode_to_utf8(this->curr.identifier)});
        } else {
            parsed.symbols.push_back(
                Symbol{
                    Symbol::SymbolType::TERMINAL,
                    utf8::unicode_to_utf8(this->curr.identifier.substr(1, this->curr.identifier.length() - 2)) // remove <>
                }
            );
        }

        this->consume();
    }

    this->consume(Token::TokenType::EOL);

    return parsed;
}

std::vector<parser_generator::Production> parser_generator::ParserRuleParser::parse_all_productions() {
    std::vector<Production> productions;

    std::optional<Production> production = this->parse_production();

    while (production.has_value()) {
        productions.push_back(production.value());

        production = this->parse_production();
    }

    return productions;
}            

// private

void parser_generator::ParserRuleParser::expect(const Token::TokenType to_expect) const {
    if (!this->accept(to_expect)) {
        std::stringstream err_msg;
        err_msg << this->curr.position << " Invalid token '" << this->curr.identifier 
                << "' of type " << this->curr.type << ". Expected token of type " << to_expect << "!" << std::endl;
        throw palex_except::ParserError(err_msg.str());
    }
}

bool parser_generator::ParserRuleParser::accept(const Token::TokenType to_check) const {
    return this->curr.type == to_check;
}

parser_generator::Token parser_generator::ParserRuleParser::consume() {
    Token consumed = this->curr;
    this->curr = this->input.next_unignored_token();

    return consumed;
}

parser_generator::Token parser_generator::ParserRuleParser::consume(const Token::TokenType to_expect) {
    this->expect(to_expect);
    
    return this->consume();
}