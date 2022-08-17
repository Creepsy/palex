#include <iostream>
#include <sstream>

#include "util/encoding.h"

int main() {    
    std::stringstream input{"1reᐁͰ3𐌈a"};
    std::u32string unicode_str;

    while(true) {
        char32_t c = encoding::get_utf8(input);
        if(input.eof()) break;

        unicode_str += c;
    } 

    std::cout << unicode_str << std::endl;

    return 0;
}