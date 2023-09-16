#include <iostream>
#include <string_view>

#include <NoFallbackTestLexer.h>

int main() {
    const std::string_view input = "inte";
    palex::NoFallbackTestLexer lexer(input);
    do {
        lexer.next_unignored_token();
        std::cout << lexer.current_token() << std::endl;
    } while (!lexer.end());
    return 0;
}