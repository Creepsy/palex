#include <sstream>

#include "templates/template_completion.h"

#include "../test_utils.h"

int main() {
    std::stringstream output;

    templates::write_template_to_stream("%%SHOULDNT_BE_REPLACED%%", output, templates::EMPTY_COMPLETER);

    TEST_TRUE(output.str() == "%SHOULDNT_BE_REPLACED%")

    return 0;
}