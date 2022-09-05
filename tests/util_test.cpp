#include <sstream>

#include "util/unicode.h"

#include "TestReport.h"

bool test_unicode_input();
bool test_unicode_output();

int main() {
    tests::TestReport report;

    report.add_test("unicode_input", test_unicode_input);
    report.add_test("unicode_output", test_unicode_output);


    report.run();

    return report.report();
}

bool test_unicode_input() {
    std::stringstream input("aШᢶ𐅄");

    return unicode::get_utf8(input) == 97 &&   // 1 byte char
           unicode::get_utf8(input) == 1064 && // 2 byte char
           unicode::get_utf8(input) == 6326 && // 3 byte char
           unicode::get_utf8(input) == 65860;   // 4 byte char
}

bool test_unicode_output() {
    std::u32string to_print = {
        97,     // 1 byte char
        1064,   // 2 byte char
        6326,   // 3 byte char
        65860   // 4 byte char
    };

    std::stringstream output;
    output << to_print;

    return output.str() == "aШᢶ𐅄";
}