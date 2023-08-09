#include "TokenInfo.h"

#include <array>

const std::array<std::string_view, 11> TOKEN_TYPE_TO_STRING {
    "UNDEFINED",
    "END_OF_FILE",
    "IGNORE",
    "REGEX",
    "PRIORITY_TAG",
    "ENTRY_PRODUCTION",
    "PRODUCTION",
    "TOKEN",
    "EQ",
    "EOL",
    "WSPACE"
};

namespace bootstrap {
    std::ostream& operator<<(std::ostream& output, const FilePosition& to_print) {
        return output << "[Ln " << to_print.line << ", Col " << to_print.column << "]";
    }

    std::ostream& operator<<(std::ostream& output, const TokenInfo& to_print) {
        return output << to_print.begin << " - " << to_print.end << " " << to_print.type << ": " << to_print.identifier;
    }

    std::ostream& operator<<(std::ostream& output, const TokenInfo::TokenType to_print) {
        return output << TOKEN_TYPE_TO_STRING.at((size_t)to_print);
    }
}