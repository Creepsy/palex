#include <sstream>

#include "util/utf8.h"

#include "../test_utils.h"

int main() {
    std::stringstream input("aШᢶ𐅄");

    TEST_TRUE(utf8::get_unicode_char(input) == 97)       // 1 byte char
    TEST_TRUE(utf8::get_unicode_char(input) == 1064)     // 2 byte char
    TEST_TRUE(utf8::get_unicode_char(input) == 6326)     // 3 byte char
    TEST_TRUE(utf8::get_unicode_char(input) == 65860)    // 4 byte char
 
    return 0;
}