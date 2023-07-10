#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstddef>
#include <cassert>
#include <array>

#include "ExampleLexer.h"
#include "ExampleParser.h"

constexpr size_t INDENTATION_LEVEL = 2;
const std::array<std::string, 8> NONTERMINAL_TYPE_TO_STRING = {
    "addition",
    "assignment",
    "expression",
    "multiplication",
    "program",
    "statement",
    "value",
    "$S"
};

void print_ast(const palex::AstNode* to_print, const size_t indentation = 0);

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

    palex::ExampleLexer lexer(input);
    palex::ExampleParser parser(lexer);
    try {
        const std::unique_ptr<palex::AstNode> parse_tree = parser.parse();
        std::cout << "Input accepted:" << std::endl;
        print_ast(parse_tree.get());
    } catch (const std::exception& err) {
        std::cerr << "Input rejected: " << err.what() << std::endl;
        input.close();
        return 1;
    }
    input.close();
    return 0;
}

void print_ast(const palex::AstNode* to_print, const size_t indentation) {
    const std::string indentation_str(indentation * INDENTATION_LEVEL, ' '); 
    if (const auto* collection_ptr = dynamic_cast<const palex::CollectionAstNode*>(to_print)) {
        assert(
            (size_t)collection_ptr->get_type() < NONTERMINAL_TYPE_TO_STRING.size() && 
            "BUG: nonterminal count doesn't match with the size of the print table!"
        );
        std::cout << indentation_str << NONTERMINAL_TYPE_TO_STRING.at((size_t)collection_ptr->get_type()) << ":" << std::endl;
        for (const std::unique_ptr<palex::AstNode>& child : collection_ptr->get_collection()) {
            print_ast(child.get(), indentation + 1);
        }
    } else if (const auto* token_ptr = dynamic_cast<const palex::TokenAstNode*>(to_print)) {
        std::cout << indentation_str << token_ptr->get_token() << std::endl;
    }
}