#pragma once

#include <stdexcept>
#include <string>

namespace palex_except {
    class ParserError : public std::exception {
        private:
            const std::string message;
        public:
            ParserError(const char* const message);
            ParserError(const std::string& message);
            virtual const char* what() const noexcept override;
    };
}