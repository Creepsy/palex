#include "template_completion.h"

#include <cstring>
#include <algorithm>
#include <cstddef>

#include "util/palex_except.h"

void templates::write_template_to_stream(
    const char* const to_write,
    std::ostream& output, 
    TemplateCompleter_t completer, 
    const char tag_begin, 
    const char tag_end
) {
    const char* const to_write_end = to_write + strlen(to_write);

    for(size_t i = 0; to_write[i] != '\0'; i++) {
        if(to_write[i] == tag_begin) {
            const char* const tag_end_ptr = std::find(to_write + i + 1, to_write_end, tag_end);
            
            if(tag_end_ptr == to_write_end) {
                throw palex_except::ParserError("The tag is missing an end character!");
            }
            if(tag_end_ptr == to_write + i + 1) {
                throw palex_except::ParserError("Empty tags in templates are not allowed!");
            }

            const std::string_view tag = std::string_view(to_write + i + 1, (size_t)(tag_end_ptr - tag_begin - 1));
            completer(output, tag);

            i = (size_t)(tag_end_ptr - to_write);
        } else {
            output << to_write[i];
        }
    }
}