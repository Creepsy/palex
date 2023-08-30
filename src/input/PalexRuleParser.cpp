#include "PalexRuleParser.h"

#include <utility>
#include <sstream>
#include <string>
#include <string_view>
#include <cassert>

#include "util/palex_except.h"
#include "util/utf8.h"

#include "regex/RegexParser.h"

// helper functions
std::string_view strip_ends(const std::string_view& to_strip);

std::string_view strip_ends(const std::string_view& to_strip) {
    return to_strip.substr(1, to_strip.length() - 2);
}

namespace input {
    PalexRuleParser::PalexRuleParser(NextTokenFunc_t next_token, CurrTokenFunc_t curr_token)
     : next_token(std::move(next_token)), curr_token(std::move(curr_token)) 
    {
        this->consume();
    }

    PalexRules PalexRuleParser::parse_all_rules() {
        std::vector<lexer_generator::TokenDefinition> token_definitions = this->parse_all_token_definitions(); // not const because of unique_ptr move
        const std::vector<parser_generator::Production> productions = this->parse_all_productions();
        assert(this->accept(bootstrap::TokenInfo::TokenType::END_OF_FILE) && "Bug: Parser didn't reach EOF after parsing finished!");
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
            this->accept(bootstrap::TokenInfo::TokenType::END_OF_FILE) || 
            this->accept(bootstrap::TokenInfo::TokenType::PRODUCTION) || 
            this->accept(bootstrap::TokenInfo::TokenType::ENTRY_PRODUCTION)
        ) {
            return std::nullopt;
        }
        lexer_generator::TokenDefinition parsed{};
        parsed.ignore_token = this->consume_if(bootstrap::TokenInfo::TokenType::IGNORE);
        const bool user_defined_priority = this->accept(bootstrap::TokenInfo::TokenType::PRIORITY_TAG);
        if (user_defined_priority) {
            const bootstrap::TokenInfo priority_tag = this->consume();
            const std::string_view priority_str = strip_ends(priority_tag.identifier); // remove <> of tag
            parsed.priority = std::stoull(std::string(priority_str)); // TODO: maybe use std::from_chars here 
        }
        this->expect(bootstrap::TokenInfo::TokenType::TOKEN);
        parsed.name = this->consume().identifier;
        this->consume(bootstrap::TokenInfo::TokenType::EQ);

        this->expect(bootstrap::TokenInfo::TokenType::REGEX);
        const std::string_view regex = strip_ends(this->curr_token().identifier); // remove "" of regex
        try {
            parsed.token_regex = regex::RegexParser(regex).parse_regex();
        } catch (const palex_except::ParserError& err) {
            std::stringstream error_message;
            error_message << this->curr_token().begin << " " << err.what();
            throw palex_except::ParserError(error_message.str());
        }
        this->consume();

        this->consume(bootstrap::TokenInfo::TokenType::EOL);
        if (!user_defined_priority) {
            parsed.priority = parsed.token_regex->get_priority();
        }
        return parsed;
    }

    std::optional<parser_generator::Production> PalexRuleParser::try_parse_production() {
        if (this->accept(bootstrap::TokenInfo::TokenType::END_OF_FILE)) {
            return std::nullopt;
        }    
        if (
            this->accept(bootstrap::TokenInfo::TokenType::TOKEN) ||
            this->accept(bootstrap::TokenInfo::TokenType::IGNORE) ||
            this->accept(bootstrap::TokenInfo::TokenType::PRIORITY_TAG)
        ) {
            this->throw_error(
                "Found the beginning of a token definition in the production block!."
                " All token definitions have to stand before the first production."
            );
        }
        parser_generator::Production parsed{};
        if (!this->accept(bootstrap::TokenInfo::TokenType::PRODUCTION) && !this->accept(bootstrap::TokenInfo::TokenType::ENTRY_PRODUCTION)) {
            this->expect(bootstrap::TokenInfo::TokenType::PRODUCTION); // error because no production was given
        }
        parsed.name = std::string(this->consume().identifier);
        this->consume(bootstrap::TokenInfo::TokenType::EQ);
        parsed.symbols = this->parse_symbol_sequence();
        this->consume(bootstrap::TokenInfo::TokenType::EOL);
        return parsed;
    }

    PalexRuleParser::~PalexRuleParser() {
    }

    std::vector<parser_generator::Symbol> PalexRuleParser::parse_symbol_sequence() {
        using parser_generator::Symbol;

        std::vector<Symbol> symbol_sequence;
        while (this->accept(bootstrap::TokenInfo::TokenType::PRODUCTION) || this->accept(bootstrap::TokenInfo::TokenType::TOKEN)) {
            symbol_sequence.push_back(
                parser_generator::Symbol{
                    (this->accept(bootstrap::TokenInfo::TokenType::TOKEN)) ? Symbol::SymbolType::TERMINAL : Symbol::SymbolType::NONTERMINAL,
                    std::string(this->curr_token().identifier)
                }
            );
            this->consume();
        }
        return symbol_sequence;
    }

    void PalexRuleParser::expect(const bootstrap::TokenInfo::TokenType to_expect) const {
        if (!this->accept(to_expect)) {
            std::stringstream error_message;
            error_message << "Invalid token '" << this->curr_token().identifier 
                    << "' of type " << this->curr_token().type << ". Expected token of type " << to_expect << "!";
            this->throw_error(error_message.str());
        }
    }

    bool PalexRuleParser::accept(const bootstrap::TokenInfo::TokenType to_check) const {
        return this->curr_token().type == to_check;
    }

    bootstrap::TokenInfo PalexRuleParser::consume() {
        bootstrap::TokenInfo consumed = this->curr_token();
        this->next_token();
        return consumed;
    }

    bootstrap::TokenInfo PalexRuleParser::consume(const bootstrap::TokenInfo::TokenType to_expect) {
        this->expect(to_expect);
        return this->consume();
    }

    bool PalexRuleParser::consume_if(const bootstrap::TokenInfo::TokenType to_check) {
        if (this->accept(to_check)) {
            this->consume();
            return true;
        }
        return false;
    }

    void PalexRuleParser::throw_error(const std::string& message) const {
        std::stringstream error_message;
        error_message << this->curr_token().begin << " " << message << std::endl;
        throw palex_except::ParserError(error_message.str());
    }
}
