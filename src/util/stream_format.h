#pragma once

#include <streambuf>
#include <ostream>
#include <cstddef>

namespace sfmt {
    struct Indentation {
        int level_change;
    };

    std::ostream& operator<<(std::ostream& output, const Indentation& manipulator);

    class IndentationStreamBuffer : public std::streambuf {
        public:
            IndentationStreamBuffer(std::ostream& target_stream, const size_t indentation_width = 4);
            IndentationStreamBuffer(std::streambuf* destination, const size_t indentation_width = 4);
            IndentationStreamBuffer(IndentationStreamBuffer&& other) noexcept;
            IndentationStreamBuffer(const IndentationStreamBuffer& other) = delete;
            virtual ~IndentationStreamBuffer();
        protected:
            virtual int overflow(int written_char) override;
            virtual int sync() override; 
        private:
            bool is_start_of_line;
            std::ostream* stream_buffer_owner;
            std::streambuf* destination;
            size_t indentation_level;
            size_t indentation_width;
        friend std::ostream& operator<<(std::ostream& output, const Indentation& manipulator);
    };  

}