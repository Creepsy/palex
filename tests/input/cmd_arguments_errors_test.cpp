#include "input/cmd_arguments.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    const char* unknown_option[] = {"palex", "-unknown-option", "parameter"};
    const char* no_parameter[] = {"palex", "-output-path"};
    const char* unknown_flag[] = {"palex", "--unknown-flag"};
    const char* invalid_module[] = {"palex", "-module-name", "99Invalid$Name"};
    const char* invalid_number[] = {"palex", "-lookahead", "ff6"};
    const char* invalid_parser_type[] = {"palex", "--parser-type", "UNKNOWN"};
    const char* invalid_language[] = {"palex", "--lang", "UNKNOWN_LANG"};
    TEST_EXCEPT(input::parse_config_from_args(sizeof(unknown_option) / sizeof(const char*), unknown_option), palex_except::ParserError)
    TEST_EXCEPT(input::parse_config_from_args(sizeof(no_parameter) / sizeof(const char*), no_parameter), palex_except::ParserError)
    TEST_EXCEPT(input::parse_config_from_args(sizeof(unknown_flag) / sizeof(const char*), unknown_flag), palex_except::ParserError)
    TEST_EXCEPT(input::parse_config_from_args(sizeof(invalid_module) / sizeof(const char*), invalid_module), palex_except::ParserError)
    TEST_EXCEPT(input::parse_config_from_args(sizeof(invalid_number) / sizeof(const char*), invalid_number), palex_except::ParserError)
    TEST_EXCEPT(input::parse_config_from_args(sizeof(invalid_parser_type) / sizeof(const char*), invalid_parser_type), palex_except::ParserError)
    TEST_EXCEPT(input::parse_config_from_args(sizeof(invalid_language) / sizeof(const char*), invalid_language), palex_except::ParserError)
    return 0;
}