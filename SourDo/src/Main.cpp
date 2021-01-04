#include <string>
#include <iostream>

#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"

namespace SourDo {
    Variant parse(const std::string& text)
    {
        std::vector<Token> tokens = tokenize_string(text);
        //std::cout << tokens << "\n";
        Parser parser;
        std::shared_ptr<Node> ast = parser.parse_tokens(tokens);
        //std::cout << ast << "\n";
        Interpreter interpreter;
        Variant result = interpreter.travel_node(ast);
        return result;
    }
} // namespace SourDo

int main()
{
    std::string text;
    while(true)
    {
        std::cout << "SourDo > ";
        std::getline(std::cin, text);
        SourDo::Variant result = SourDo::parse(text);
        std::cout << result << std::endl;
    }
    return 0;
}
