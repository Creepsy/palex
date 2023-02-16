#include <sstream>

#include "templates/template_completion.h"

#include "../test_utils.h"

int main() {
    templates::TemplateCompleter_t completer = [](std::ostream& output, const std::string_view tag) -> void {        
        if (tag == "replace_me") {
            output << "TAG_REPLACE";
        }
    };

    std::stringstream output;

    templates::write_template_to_stream("A text with a %replace_me% in it. %This% gets not replaced!", output, completer);

    TEST_TRUE(output.str() == "A text with a TAG_REPLACE in it.  gets not replaced!")

    return 0;
}