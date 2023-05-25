#include <cstddef>
#include <vector>
#include <string>

#include "input/cmd_arguments.h"

#include "../test_utils.h"

int main() {
    const char* argv[] = {
        "palex",
        "Test.palex",
        "test/AnotherTest.palex",
        "-lang",
        "c++",
        "-util-path",
        "../util",
        "-module-name",
        "a_module",
        "-output-path",
        ".",
        "-parser-type",
        "LALR",
        "--lexer",
        "--parser"
    };
    const size_t argc = sizeof(argv) / sizeof(const char*);
    const input::PalexConfig config = input::parse_config_from_args(argc, argv);

    TEST_TRUE(config.rule_files == (std::vector<std::string>{"Test.palex", "test/AnotherTest.palex"}))
    TEST_TRUE(config.output_path == ".")
    TEST_TRUE(config.util_output_path == "../util")
    TEST_TRUE(config.module_name == "a_module")
    TEST_TRUE(config.language == input::Language::CPP)
    TEST_TRUE(config.parser_type == input::ParserType::LALR)
    TEST_TRUE(config.lookahead == 0)
    TEST_TRUE(config.generate_lexer)
    TEST_FALSE(config.generate_util)
    TEST_TRUE(config.generate_parser)
    TEST_FALSE(config.lexer_fallback)
    return 0;
}