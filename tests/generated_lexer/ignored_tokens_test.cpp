#include <iostream>
#include <string_view>

#include <IgnoredTokensTestLexer.h>

int main() {
    const std::string_view input = "     some string    more spaces    another identifier";
    palex::IgnoredTokensTestLexer lexer(input);
    do {
        lexer.next_unignored_token();
        std::cout << lexer.current_token() << std::endl;
    } while (!lexer.end());
    return 0;
}