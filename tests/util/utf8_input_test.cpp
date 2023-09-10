#include <string_view>
#include <iostream>

#include "util/utf8.h"

#include "../test_utils.h"

int main() {
    const std::string_view input = "aШᢶ𐅄";
    const char* position = input.begin();
    utf8::Codepoint_t current_codepoint = 0;

    position = utf8::advance_codepoint(position, input.end(), &current_codepoint);
    TEST_TRUE(current_codepoint == 97)       // 1 byte char
    position = utf8::advance_codepoint(position, input.end(), &current_codepoint);
    TEST_TRUE(current_codepoint == 1064)     // 2 byte char
    position = utf8::advance_codepoint(position, input.end(), &current_codepoint);
    TEST_TRUE(current_codepoint == 6326)     // 3 byte char
    position = utf8::advance_codepoint(position, input.end(), &current_codepoint);
    TEST_TRUE(current_codepoint == 65860)    // 4 byte char
    
    return 0;
}