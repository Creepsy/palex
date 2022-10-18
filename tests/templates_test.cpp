#include <sstream>

#include "templates/template_completion.h"

#include "util/palex_except.h"

#include "TestReport.h"
#include "test_utils.h"

bool test_template_completion();
bool test_template_errors();
bool test_template_escape_character();

int main() {
    tests::TestReport report;

    report.add_test("template_completion", test_template_completion);
    report.add_test("template_errors", test_template_errors);
    report.add_test("template_escape_character", test_template_escape_character);

    report.run();

    return report.report();
}

bool test_template_completion() {
    templates::TemplateCompleter_t completer = [](std::ostream& output, const std::string_view tag) -> void {        
        if(tag == "replace_me") {
            output << "TAG_REPLACE";
        }
    };

    std::stringstream output;

    templates::write_template_to_stream("A text with a %replace_me% in it. %This% gets not replaced!", output, completer);

    TEST_TRUE(output.str() == "A text with a TAG_REPLACE in it.  gets not replaced!")

    return true;
}

bool test_template_errors() {
    std::stringstream output;

    TEST_EXCEPT(templates::write_template_to_stream("a %template without tag end!", output, templates::EMPTY_COMPLETER), palex_except::ParserError)

    return true;
}

bool test_template_escape_character() {
    std::stringstream output;

    templates::write_template_to_stream("%%SHOULDNT_BE_REPLACED%%", output, templates::EMPTY_COMPLETER);

    TEST_TRUE(output.str() == "%SHOULDNT_BE_REPLACED%")

    return true;
}

