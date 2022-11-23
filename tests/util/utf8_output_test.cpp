#include <sstream>

#include "util/utf8.h"

#include "../test_utils.h"

int main() {
    std::u32string to_print = {
        97,     // 1 byte char
        1064,   // 2 byte char
        6326,   // 3 byte char
        65860   // 4 byte char
    };

    std::stringstream output;
    output << to_print;

    TEST_TRUE(output.str() == "aШᢶ𐅄")

    return 0;
}