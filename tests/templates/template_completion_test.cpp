#include <sstream>

#include "templates/template_completion.h"

#include "../test_utils.h"

int main() {
    std::stringstream output;
    templates::write_template_to_stream(
        "A text with a %replace_me% in it. %This% is an empty replacer!",
        output,
        std::map<std::string_view, templates::TemplateCompleter_t>{
            {"replace_me", templates::constant_completer("TAG_REPLACE")},
            {"This", templates::EMPTY_COMPLETER}
        }
    );

    TEST_TRUE(output.str() == "A text with a TAG_REPLACE in it.  is an empty replacer!")

    return 0;
}