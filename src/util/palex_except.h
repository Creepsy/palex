#pragma once

#include <stdexcept>
#include <string>

namespace palex_except {
    class ParserError : public std::exception {
        public:
            ParserError(const char* const message);
            ParserError(const std::string& message);
            const char* what() const noexcept override;
        private:
            const std::string message;
    };

    class ValidationError : public std::exception {
        public:
            ValidationError(const char* const message);
            ValidationError(const std::string& message);
            const char* what() const noexcept override;
        private:
            const std::string message;
    };
}