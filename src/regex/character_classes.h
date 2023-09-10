#pragma once

#include <vector>

#include "regex_ast.h"
#include "util/utf8.h"

namespace regex {
    namespace character_classes {
        const std::vector<CharRange> DIGIT_CLASS = {
            CharRange{'0', '9'}, CharRange{1632, 1641},
            CharRange{1776, 1785}, CharRange{1984, 1993},
            CharRange{2406, 2415}, CharRange{2534, 2543},
            CharRange{2662, 2671}, CharRange{2790, 2799},
            CharRange{2918, 2927}, CharRange{3046, 3055},
            CharRange{3174, 3183}, CharRange{3302, 3311},
            CharRange{3430, 3439}, CharRange{3558, 3567},
            CharRange{3664, 3673}, CharRange{3792, 3801},
            CharRange{3872, 3881}, CharRange{4160, 4169},
            CharRange{4240, 4249}, CharRange{6112, 6121},
            CharRange{6160, 6169}, CharRange{6470, 6479},
            CharRange{6608, 6617}, CharRange{6784, 6793},
            CharRange{6800, 6809}, CharRange{6992, 7001},
            CharRange{7088, 7097}, CharRange{7232, 7241},
            CharRange{7248, 7257}, CharRange{42528, 42537},
            CharRange{43216, 43225}, CharRange{43264, 43273},
            CharRange{43472, 43481}, CharRange{43504, 43513},
            CharRange{43600, 43609}, CharRange{44016, 44025},
            CharRange{65296, 65305}, CharRange{66720, 66729},
            CharRange{68912, 68921}, CharRange{69734, 69743},
            CharRange{69872, 69881}, CharRange{69942, 69951},
            CharRange{70096, 70105}, CharRange{70384, 70393},
            CharRange{70736, 70745}, CharRange{70864, 70873},
            CharRange{71248, 71257}, CharRange{71360, 71369},
            CharRange{71472, 71481}, CharRange{71904, 71913},
            CharRange{72016, 72025}, CharRange{72784, 72793},
            CharRange{73040, 73049}, CharRange{73120, 73129},
            CharRange{92768, 92777}, CharRange{92864, 92873},
            CharRange{93008, 93017}, CharRange{120782, 120831},
            CharRange{123200, 123209}, CharRange{123632, 123641},
            CharRange{125264, 125273}, CharRange{130032, 130041}
        };

        const std::vector<CharRange> NON_DIGIT_CLASS = {
            CharRange{0, '0' - 1}, CharRange{'9' + 1, 1631},
            CharRange{1642, 1775}, CharRange{1786, 1983},
            CharRange{1994, 2405}, CharRange{2416, 2533},
            CharRange{2544, 2661}, CharRange{2672, 2789},
            CharRange{2800, 2917}, CharRange{2928, 3045},
            CharRange{3056, 3173}, CharRange{3184, 3301},
            CharRange{3312, 3429}, CharRange{3440, 3557},
            CharRange{3568, 3663}, CharRange{3674, 3791},
            CharRange{3802, 3871}, CharRange{3882, 4159},
            CharRange{4170, 4239}, CharRange{4250, 6111},
            CharRange{6122, 6159}, CharRange{6170, 6469},
            CharRange{6480, 6607}, CharRange{6618, 6783},
            CharRange{6794, 6799}, CharRange{6810, 6991},
            CharRange{7002, 7087}, CharRange{7098, 7231},
            CharRange{7242, 7247}, CharRange{7258, 42527},
            CharRange{42538, 43215}, CharRange{43226, 43263},
            CharRange{43274, 43471}, CharRange{43482, 43503},
            CharRange{43514, 43599}, CharRange{43610, 44015},
            CharRange{44026, 65295}, CharRange{65306, 66719},
            CharRange{66730, 68911}, CharRange{68922, 69733},
            CharRange{69744, 69871}, CharRange{69882, 69941},
            CharRange{69952, 70095}, CharRange{70106, 70383},
            CharRange{70394, 70735}, CharRange{70746, 70863},
            CharRange{70874, 71247}, CharRange{71258, 71359},
            CharRange{71370, 71471}, CharRange{71482, 71903},
            CharRange{71914, 72015}, CharRange{72026, 72783},
            CharRange{72794, 73039}, CharRange{73050, 73119},
            CharRange{73130, 92767}, CharRange{92778, 92863},
            CharRange{92874, 93007}, CharRange{93018, 120781},
            CharRange{120832, 123199}, CharRange{123210, 123631},
            CharRange{123642, 125263}, CharRange{125274, 130031},
            CharRange{130042, utf8::LAST_4_BYTE_CODEPOINT}
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
            CharRange{'z' + 1, utf8::LAST_4_BYTE_CODEPOINT}
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
            CharRange{12289, utf8::LAST_4_BYTE_CODEPOINT}
        };

        const std::vector<CharRange> DOT_CLASS = {
            CharRange{0, '\n' - 1},
            CharRange{'\n' + 1, '\r' - 1},
            CharRange{'\r' + 1, utf8::LAST_4_BYTE_CODEPOINT}
        };
    }
}