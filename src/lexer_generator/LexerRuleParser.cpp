#include "LexerRuleParser.h"

#include <iostream>
#include <sstream>

#include "util/palex_except.h"
#include "util/encoding.h"

using namespace lexer_generator;

lexer_generator::LexerRuleParser::LexerRuleParser(LexerRuleLexer& input) : input(input) {
    this->consume();
}



//public

std::optional<TokenRegexRule> lexer_generator::LexerRuleParser::parse_token_rule() {
    if(this->accept(Token::TokenType::END_OF_FILE)) return std::nullopt;

    TokenRegexRule rule{};

    if(this->accept(Token::TokenType::IGNORE)) {
        this->consume();
        rule.ignore_token = true;
    }

    this->expect(Token::TokenType::IDENTIFER);
    rule.token_name = this->curr.identifier;
    this->consume();

    this->consume(Token::TokenType::EQUALS);

    this->expect(Token::TokenType::REGEX);
    rule.token_regex = this->curr.identifier.substr(1, this->curr.identifier.length() - 2); // remove " enclosing regex
    this->consume();

    this->consume(Token::TokenType::END_OF_LINE);

    return rule;
}



//private

void lexer_generator::LexerRuleParser::expect(const Token::TokenType type) {
    if(!this->accept(type)) this->throw_parsing_err(type);
}

bool lexer_generator::LexerRuleParser::accept(const Token::TokenType type) {
    return this->curr.type == type;
}

void lexer_generator::LexerRuleParser::consume() {
    this->curr = this->input.next_token();
}

void lexer_generator::LexerRuleParser::consume(const Token::TokenType type) {
    this->expect(type);
    this->consume();
}

void lexer_generator::LexerRuleParser::throw_parsing_err(const Token::TokenType expected) {
    std::stringstream err{};

    err << "Unexpected token while parsing! Expected '" << Token::TYPE_NAMES.at(expected)
        << "', found '" << Token::TYPE_NAMES.at(this->curr.type) 
        << "' at " << this->curr.start << " with the identifier \"" << this->curr.identifier << "\"!";

    throw ParserError(err.str().c_str());
}