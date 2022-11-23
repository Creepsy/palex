#include "regex/regex_ast.h"

#include "../test_utils.h"

int main() {
    TEST_TRUE(regex::CharRange{'a'}.is_single_char())
    TEST_FALSE(regex::CharRange('a', 'z').is_single_char())

    TEST_TRUE(regex::CharRange{}.empty())
    TEST_FALSE(regex::CharRange{'a'}.empty())
    TEST_FALSE(regex::CharRange('a', 'z').empty())

    TEST_TRUE(regex::CharRange('a', 'g').can_prepend_to(regex::CharRange('h', 'z')))
    TEST_TRUE(regex::CharRange('a', 'g').can_prepend_to(regex::CharRange('e', 'z')))
    TEST_FALSE(regex::CharRange('a', 'f').can_prepend_to(regex::CharRange('h', 'z')))

    TEST_TRUE(regex::CharRange('h', 'z').can_append_to(regex::CharRange('a', 'g')))
    TEST_TRUE(regex::CharRange('e', 'z').can_append_to(regex::CharRange('a', 'g')))
    TEST_FALSE(regex::CharRange('h', 'z').can_append_to(regex::CharRange('a', 'f')))

    TEST_TRUE(regex::CharRange{}.is_subset_of(regex::CharRange{}))
    TEST_TRUE(regex::CharRange{}.is_subset_of(regex::CharRange('a', 'z')))
    TEST_TRUE(regex::CharRange{'f'}.is_subset_of(regex::CharRange('a', 'z')))
    TEST_TRUE(regex::CharRange('d', 'z').is_subset_of(regex::CharRange{'a', 'z'}))
    TEST_TRUE(regex::CharRange('d', 'g').is_subset_of(regex::CharRange{'a', 'z'}))
    TEST_TRUE(regex::CharRange('a', 'z').is_subset_of(regex::CharRange{'a', 'z'}))

    TEST_FALSE(regex::CharRange{'a'}.is_subset_of(regex::CharRange{}))
    TEST_FALSE(regex::CharRange{'a'}.is_subset_of(regex::CharRange{'b'}))
    TEST_FALSE(regex::CharRange('a', 'z').is_subset_of(regex::CharRange{'b'}))
    TEST_FALSE(regex::CharRange('a', 'f').is_subset_of(regex::CharRange{'d', 'z'}))

    TEST_TRUE(regex::CharRange{} == regex::CharRange{})
    TEST_TRUE(regex::CharRange{'a'} == regex::CharRange{'a'})
    TEST_TRUE(regex::CharRange('a', 'z') == regex::CharRange('a', 'z'))
    
    TEST_FALSE(regex::CharRange{} == regex::CharRange{'a'})
    TEST_FALSE(regex::CharRange{'b'} == regex::CharRange{'a'})
    TEST_FALSE(regex::CharRange('a', 'z') == regex::CharRange{'a'})

    TEST_TRUE(regex::CharRange::common_subset(regex::CharRange{}, regex::CharRange('a', 'z')) == regex::CharRange{})
    TEST_TRUE(regex::CharRange::common_subset(regex::CharRange{'a'}, regex::CharRange('a', 'z')) == regex::CharRange{'a'})
    TEST_TRUE(regex::CharRange::common_subset(regex::CharRange('a', 'z'), regex::CharRange{'a'}) == regex::CharRange{'a'})
    TEST_TRUE(regex::CharRange::common_subset(regex::CharRange('a', 'g'), regex::CharRange('c', 'z')) == regex::CharRange('c', 'g'))
    TEST_TRUE(regex::CharRange::common_subset(regex::CharRange('a', 'g'), regex::CharRange('h', 'z')) == regex::CharRange{})
    TEST_TRUE(regex::CharRange::common_subset(regex::CharRange('c', 'z'), regex::CharRange('a', 'g')) == regex::CharRange('c', 'g'))
    TEST_TRUE(regex::CharRange::common_subset(regex::CharRange('a', 'z'), regex::CharRange('c', 'm')) == regex::CharRange('c', 'm'))
    TEST_TRUE(regex::CharRange::common_subset(regex::CharRange('a', 'g'), regex::CharRange('a', 'g')) == regex::CharRange('a', 'g'))

    return 0;
}