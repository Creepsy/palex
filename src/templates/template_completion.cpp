#include "template_completion.h"

#include <cstring>
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <stdexcept>

#include "util/palex_except.h"
#include "util/stream_format.h"

templates::TemplateCompleter_t templates::EMPTY_COMPLETER = [](std::ostream& output) -> void {};

templates::TemplateCompleter_t templates::constant_completer(const std::string& completer_output) {
    return [=](std::ostream& output) { output << completer_output; };
}

templates::TemplateCompleter_t templates::conditional_completer(const bool enable_flag, const std::string& completer_output) {
    return enable_flag ? constant_completer(completer_output) : EMPTY_COMPLETER;
}

templates::TemplateCompleter_t templates::choice_completer(const bool toggle_flag, const std::string& on_true, const std::string& on_false) {
    return constant_completer(toggle_flag ? on_true : on_false);
}

void templates::write_template_to_stream(
    const char* const to_write, 
    std::ostream& output, 
    const std::map<std::string_view, TemplateCompleter_t>& completers
) {
    const char* const to_write_end = to_write + strlen(to_write);
    for (const char* curr_ptr = to_write; curr_ptr != to_write_end; curr_ptr++) {
        if (*curr_ptr == '%') {
            const char* const tag_begin_ptr = curr_ptr;
            const char* const tag_end_ptr = std::find(tag_begin_ptr + 1, to_write_end, '%');
            if (tag_end_ptr == to_write_end) {
                throw palex_except::ParserError("The tag is missing an end character!");
            }
            if (tag_end_ptr - tag_begin_ptr == 1) {
                output << '%'; // Empty tags get interpreted as % (modulo-operator)
            } else {
                const std::string_view tag = std::string_view(tag_begin_ptr + 1, (size_t)(tag_end_ptr - tag_begin_ptr - 1));
                if (completers.find(tag) == completers.end()) {
                    throw std::runtime_error("No matching completer-function for the tag '" + std::string(tag) + "' found!");
                }
                completers.at(tag)(output);
            }
            curr_ptr = tag_end_ptr;
        } else {
            output << *curr_ptr;
        }
    }
}

void templates::write_template_to_file(
    const char* const to_write, 
    const std::string& output_path, 
    const std::map<std::string_view, TemplateCompleter_t>& completers
) {
    std::ofstream output_file(output_path);
    sfmt::IndentationStreamBuffer indentation_output_file_buffer(output_file);
    if (!output_file.is_open()) {
        throw std::runtime_error("Unable to open the file '" + output_path + "'!");
    }
    try {
        write_template_to_stream(to_write, output_file, completers);
    } catch(...) {
        output_file.close();
        throw;
    }
    output_file.close();
}