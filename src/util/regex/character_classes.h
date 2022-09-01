#pragma once

#include <vector>

#include "regex_ast.h"
#include "util/unicode.h"

namespace regex {
    namespace character_classes {
        const std::vector<CharRange> DIGIT_CLASS = {
            CharRange{'0', '9'}
        };

        const std::vector<CharRange> NON_DIGIT_CLASS = {
            CharRange{0, '0' - 1},
            CharRange{'9' + 1, unicode::LAST_UNICODE_CHAR}
        };

        const std::vector<CharRange> WORD_CLASS = {
            CharRange{'0', '9'},
            CharRange{'A', 'Z'},
            CharRange{'_'},
            CharRange{'a', 'z'}
        };

        const std::vector<CharRange> NON_WORD_CLASS = {
            CharRange{0, '0' - 1},
            CharRange{'9' + 1, 'A' - 1},
            CharRange{'Z' + 1, '_' - 1},
            CharRange{'_' + 1, 'a' - 1},
            CharRange{'z' + 1, unicode::LAST_UNICODE_CHAR}
        };

        const std::vector<CharRange> WHITESPACE_CLASS = {
            CharRange{'\t', '\r'},
            CharRange{' '},
            CharRange{133},
            CharRange{160},
            CharRange{5760},
            CharRange{8192, 8202},
            CharRange{8232, 8233},
            CharRange{8239},
            CharRange{8297},
            CharRange{12288}
        };

        const std::vector<CharRange> NON_WHITESPACE_CLASS = {
            CharRange{0, '\t' - 1},
            CharRange{'\r' + 1, ' ' - 1},
            CharRange{' ' + 1, 132},
            CharRange{134, 159},
            CharRange{161, 5759},
            CharRange{5761, 8191},
            CharRange{8203, 8231},
            CharRange{8234, 8238},
            CharRange{8240, 8296},
            CharRange{8298, 12287},
            CharRange{12289, unicode::LAST_UNICODE_CHAR}
        };

        const std::vector<CharRange> DOT_CLASS = {
            CharRange{0, '\n' - 1},
            CharRange{'\n' + 1, '\r' - 1},
            CharRange{'\r' + 1, unicode::LAST_UNICODE_CHAR}
        };
    }
}