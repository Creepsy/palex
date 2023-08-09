#pragma once

#include "bootstrap/BootstrapLexer.h"

#include "PalexRuleLexer.h"
#include "PalexRuleParser.h"

namespace input {
    class PalexLexerAdapter {
        public:
            PalexLexerAdapter(PalexRuleLexer& to_adapt);
            bootstrap::TokenInfo::TokenType next_token();
            const bootstrap::TokenInfo& get_token() const;
        private:
            PalexRuleLexer& lexer;
            bootstrap::TokenInfo current_token;
            std::string identifier_buffer; 
    };
}