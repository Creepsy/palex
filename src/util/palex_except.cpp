#include "palex_except.h"

#include <string>

ParserError::ParserError(const char* const message) : std::exception(), message(message) {    
}

ParserError::ParserError(const std::string& message) : std::exception(), message(message) {    
}

const char* ParserError::what() const noexcept {
    return this->message.c_str();
}