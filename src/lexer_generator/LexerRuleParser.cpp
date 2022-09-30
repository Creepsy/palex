#include "LexerRuleParser.h"

#include <iostream>
#include <sstream>

#include "util/regex/RegexParser.h"

#include "util/palex_except.h"
#include "util/unicode.h"

lexer_generator::LexerRuleParser::LexerRuleParser(LexerRuleLexer& input) : input(input) {
    this->consume();
}

//public

std::optional<lexer_generator::TokenRegexRule> lexer_generator::LexerRuleParser::parse_token_rule() {
    if(this->accept(Token::TokenType::END_OF_FILE)) return std::nullopt;

    TokenRegexRule rule{};

    if(this->accept(Token::TokenType::IGNORE)) {
        this->consume();
        rule.ignore_token = true;
    }

    const bool user_defined_priority = this->accept(Token::TokenType::ANGLE_BRACKET_OPEN);
    if(user_defined_priority) {
        this->consume();
        
        this->expect(Token::TokenType::INTEGER);
        rule.priority = std::stoull(unicode::to_utf8(this->curr.identifier));
        this->consume();

        this->consume(Token::TokenType::ANGLE_BRACKET_CLOSE);
    } 

    this->expect(Token::TokenType::IDENTIFER);
    rule.token_name = this->consume(Token::TokenType::IDENTIFER).identifier;

    this->consume(Token::TokenType::EQUALS);

    this->expect(Token::TokenType::REGEX);
    const std::u32string regex_str = this->curr.identifier.substr(1, this->curr.identifier.length() - 2); // remove " enclosing regex
    rule.token_regex = std::move(regex::RegexParser(regex_str).parse_regex());
    this->consume();

    this->consume(Token::TokenType::END_OF_LINE);

    if(!user_defined_priority) {
        rule.priority = rule.token_regex->get_priority();
    }

    return rule;
}

std::vector<lexer_generator::TokenRegexRule> lexer_generator::LexerRuleParser::parse_all_rules() {
    std::vector<TokenRegexRule> all_rules;

    while(true) {
        std::optional<lexer_generator::TokenRegexRule> token_rule_result = this->parse_token_rule();

        if(!token_rule_result.has_value()) {
            break;
        }

        all_rules.push_back(std::move(token_rule_result.value()));
    }

    return std::move(all_rules);
}

//private

void lexer_generator::LexerRuleParser::expect(const Token::TokenType type) {
    if(!this->accept(type)) this->throw_parsing_err(type);
}

bool lexer_generator::LexerRuleParser::accept(const Token::TokenType type) {
    return this->curr.type == type;
}

lexer_generator::Token lexer_generator::LexerRuleParser::consume() {
    Token consumed = this->curr;
    this->curr = this->input.next_token();

    return consumed;
}

lexer_generator::Token lexer_generator::LexerRuleParser::consume(const Token::TokenType type) {
    this->expect(type);
    return this->consume();
}

void lexer_generator::LexerRuleParser::throw_parsing_err(const Token::TokenType expected) {
    std::stringstream err{};

    err << "Unexpected token while parsing! Expected '" << Token::TOKEN_TYPE_NAMES.at((size_t)expected)
        << "', found '" << Token::TOKEN_TYPE_NAMES.at((size_t)this->curr.type) 
        << "' at " << this->curr.start << " with the identifier \"" << this->curr.identifier << "\"!";

    throw palex_except::ParserError(err.str());
}