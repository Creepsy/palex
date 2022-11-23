#include "regex/regex_ast.h"

#include "../test_utils.h"

int main() {
    const regex::CharRangeSet ALPHABET = regex::CharRangeSet{}.insert_char_range(regex::CharRange{'a', 'z'});

    regex::CharRangeSet first;
    regex::CharRangeSet second;

    TEST_TRUE(first.empty())

    first.insert_char_range(regex::CharRange{'a', 'e'});
    second.insert_char_range(regex::CharRange{'f', 'z'});

    first = first + second;

    TEST_TRUE(first == ALPHABET)

    first = first - second;

    TEST_TRUE(first.get_ranges().size() == 1)
    TEST_TRUE(first.get_ranges().front() == regex::CharRange('a', 'e'))
    TEST_TRUE(first != ALPHABET)

    TEST_TRUE(ALPHABET.get_intersection(first) == first)

    return 0;
}