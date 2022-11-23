#include <sstream>

#include "templates/template_completion.h"

#include "util/palex_except.h"

#include "../test_utils.h"

int main() {
    std::stringstream output;

    TEST_EXCEPT(templates::write_template_to_stream("a %template without tag end!", output, templates::EMPTY_COMPLETER), palex_except::ParserError)

    return 0;
}