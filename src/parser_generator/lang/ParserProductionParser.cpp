#include "ParserProductionParser.h"

#include <sstream>

#include "util/utf8.h"

bool parser_generator::Production::is_entry() const {
    return this->name == ENTRY_PRODUCTION_NAME;
}

parser_generator::ParserProductionParser::ParserProductionParser(ParserProductionLexer& input) : input(input) {
    this->consume();
}

std::optional<parser_generator::Production> parser_generator::ParserProductionParser::parse_production() {
    if (this->accept(Token::TokenType::END_OF_FILE)) {
        return std::nullopt;
    }

    Production parsed{};
    if (this->accept(Token::TokenType::PRODUCTION) || this->accept(Token::TokenType::ENTRY_PRODUCTION)) {
        parsed.name = utf8::unicode_to_utf8(this->consume().identifier);
    } else {
        this->expect(Token::TokenType::PRODUCTION); // error because no production name was given
    }
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

std::vector<parser_generator::Production> parser_generator::ParserProductionParser::parse_all_productions() {
    std::vector<Production> productions;

    std::optional<Production> production = this->parse_production();

    while (production.has_value()) {
        productions.push_back(production.value());

        production = this->parse_production();
    }

    return productions;
}            

// private

void parser_generator::ParserProductionParser::expect(const Token::TokenType to_expect) const {
    if (!this->accept(to_expect)) {
        std::stringstream err_msg;
        err_msg << this->curr.position << " Invalid token '" << this->curr.identifier 
                << "' of type " << this->curr.type << ". Expected token of type " << to_expect << "!" << std::endl;
        throw palex_except::ParserError(err_msg.str());
    }
}

bool parser_generator::ParserProductionParser::accept(const Token::TokenType to_check) const {
    return this->curr.type == to_check;
}

parser_generator::Token parser_generator::ParserProductionParser::consume() {
    Token consumed = this->curr;
    this->curr = this->input.next_unignored_token();

    return consumed;
}

parser_generator::Token parser_generator::ParserProductionParser::consume(const Token::TokenType to_expect) {
    this->expect(to_expect);
    
    return this->consume();
}

bool parser_generator::operator<(const Symbol& first, const Symbol& second) {
    if (first.type < second.type) return true;
    return first.type == second.type && first.identifier < second.identifier;
}

bool parser_generator::operator<(const Production& first, const Production& second) {
    if (first.name < second.name) return true;
    return first.name == second.name && first.symbols < second.symbols;
}

bool parser_generator::operator==(const Symbol& first, const Symbol& second) {
    return first.type == second.type && first.identifier == second.identifier;
}

bool parser_generator::operator==(const Production& first, const Production& second) {
    return first.name == second.name && first.symbols == second.symbols;
}

std::ostream& parser_generator::operator<<(std::ostream& output, const Symbol& to_print) {
    if (to_print.type == Symbol::SymbolType::TERMINAL) {
        return output << "<" << to_print.identifier << ">";
    }
    return output << to_print.identifier;
}

std::ostream& parser_generator::operator<<(std::ostream& output, const Production& to_print) {
    output << to_print.name << " =";
    for (const Symbol& sym : to_print.symbols) {
        output << " " << sym;
    }
    return output;
}