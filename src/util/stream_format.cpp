#include "stream_format.h"

#include <algorithm>

namespace sfmt {
    std::ostream& operator<<(std::ostream& output, const Indentation& manipulator) {
        if (!output.rdbuf()) {
            return output;
        }
        if (IndentationStreamBuffer* indentation_buffer = dynamic_cast<IndentationStreamBuffer*>(output.rdbuf())) {
            indentation_buffer->indentation_level = std::max((size_t)0, indentation_buffer->indentation_level + manipulator.level_change);
        }
        return output;
    }

    IndentationStreamBuffer::IndentationStreamBuffer(std::ostream& target_stream, const size_t indentation_width) 
     : indentation_level{0}, 
       stream_buffer_owner{&target_stream}, 
       destination{target_stream.rdbuf()}, 
       is_start_of_line(true), 
       indentation_width(indentation_width) 
     {
        this->stream_buffer_owner->rdbuf(this);
    }
    
    IndentationStreamBuffer::IndentationStreamBuffer(std::streambuf* destination, const size_t indentation_width)
     : indentation_level{0}, 
       stream_buffer_owner{nullptr}, 
       destination{destination}, 
       is_start_of_line(true), 
       indentation_width(indentation_width) 
    {
    }

    IndentationStreamBuffer::IndentationStreamBuffer(IndentationStreamBuffer&& other) noexcept
     : indentation_level{other.indentation_level},
       stream_buffer_owner{other.stream_buffer_owner},
       destination{other.destination}, 
       is_start_of_line(other.is_start_of_line),
       indentation_width(other.indentation_width)
    {
        other.stream_buffer_owner = nullptr;
        other.destination = nullptr;
    }

    IndentationStreamBuffer::~IndentationStreamBuffer() {
        if (this->stream_buffer_owner) {
            this->stream_buffer_owner->rdbuf(this->destination);
        }
    }

    int IndentationStreamBuffer::overflow(int written_char) {
        if (this->is_start_of_line && written_char != '\n') {
            const size_t whitespace_count = this->indentation_level * this->indentation_width;
            this->destination->sputn(std::string(whitespace_count, ' ').c_str(), (std::streamsize)whitespace_count);
        }
        this->is_start_of_line = written_char == '\n';
        return (int)this->destination->sputc((char)written_char);
    }

    int IndentationStreamBuffer::sync() {
        return this->destination->pubsync();
    }
}