#include "palex_except.h"

palex_except::ParserError::ParserError(const char* const message) : std::exception(), message(message) {    
}

palex_except::ParserError::ParserError(const std::string& message) : std::exception(), message(message) {    
}

const char* palex_except::ParserError::what() const noexcept {
    return this->message.c_str();
}