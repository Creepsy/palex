#include <json.h>

#include "util/config_getters.h"
#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    const nlohmann::json INPUT = nlohmann::json::parse(R"(
        {
            "abool": true,
            "notabool": 24,
            "astring": "abc",
            "notastring": false
        }
    )");

    bool bool_storage = false;
    std::string string_storage;

    TEST_TRUE(config::optional_bool(INPUT, "abool", bool_storage))
    TEST_TRUE(bool_storage)
    TEST_FALSE(config::optional_bool(INPUT, "notabool", bool_storage))
    TEST_TRUE(bool_storage)

    TEST_EXCEPT(config::require_bool(INPUT, "notabool", bool_storage, "test_input"), palex_except::ParserError)

    TEST_TRUE(config::optional_string(INPUT, "astring", string_storage))
    TEST_TRUE(string_storage == "abc")
    TEST_FALSE(config::optional_string(INPUT, "notastring", string_storage))
    TEST_TRUE(string_storage == "abc")
    
    TEST_EXCEPT(config::require_string(INPUT, "notastring", string_storage, "test_input"), palex_except::ParserError)

    return 0;
}