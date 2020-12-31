#include <string>
#include <iostream>

#include "Tokenizer.hpp"
#include "Parser.hpp"

namespace SourDo {
    std::shared_ptr<Node> parse(const std::string& text)
    {
        std::vector<Token> tokens = tokenize_string(text);
        std::cout << tokens << "\n";
        Parser parser;
        std::cout << "I got the parser\n";
        std::shared_ptr<Node> ast = parser.parse_tokens(tokens);
        return ast;
    }
} // namespace SourDo

int main()
{
    std::string text;
    while(true)
    {
        std::cout << "SourDo > ";
        std::getline(std::cin, text);
        auto result = SourDo::parse(text);
        std::cout << result << std::endl;
    }
}
