#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstddef>
#include <cassert>
#include <array>
#include <functional>

#include <ExampleLexer.h>
#include <ExampleParser.h>
#include <ExampleASTBuilderBase.h>

class ExampleASTBuilder : public palex::ExampleASTBuilderBase {
    public:
        ExampleASTBuilder() : palex::ExampleASTBuilderBase() {}
        void shift_token(const palex::ExampleToken to_shift) override {
            std::cout << "shift " << to_shift << std::endl;
        }

        void reduce_addition(const size_t child_count) override {
            std::cout << "reduce addition" << std::endl;
        }

        void reduce_assignment(const size_t child_count) override {
            std::cout << "reduce assignment" << std::endl;
        }

        void reduce_expression(const size_t child_count) override {
            std::cout << "reduce expression" << std::endl;
        }

        void reduce_multiplication(const size_t child_count) override {
            std::cout << "reduce multiplication" << std::endl;
        }

        void reduce_program(const size_t child_count) override {
            std::cout << "reduce program" << std::endl;
        }

        void reduce_statement(const size_t child_count) override {
            std::cout << "reduce statement" << std::endl;
        }

        void reduce_value(const size_t child_count) override {
            std::cout << "reduce value" << std::endl;
        }    
};

std::string load_stream(const std::istream& to_load);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "No arguments supplied!" << std::endl;
        return 1;
    }
    const std::string input_file_path = argv[1];
    std::ifstream input(input_file_path);
    if (!input.is_open()) {
        std::cerr << "Unable to open file '" << input_file_path << "'!";
        return 1;
    }
    const std::string file_contents = load_stream(input);
    input.close();
    palex::ExampleLexer lexer(file_contents);
    ExampleASTBuilder builder{};
    palex::ExampleParser parser(
        builder,
        std::bind(&palex::ExampleLexer::next_unignored_token, &lexer),
        std::bind(&palex::ExampleLexer::current_token, &lexer),
        palex::make_default_parse_error_handler(input_file_path, file_contents)
    );
    try {
        parser.parse();
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
    std::cout << "Successfully parsed!" << std::endl;
    return 0;
}


std::string load_stream(const std::istream& to_load) {
    std::stringstream file_content;
    file_content << to_load.rdbuf();
    return file_content.str();
}