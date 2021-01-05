#include <string>
#include <iostream>

#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"

namespace SourDo {
    struct InterpretResult
    {
        Variant result;
        std::optional<Error> error;
    };

    InterpretResult interpret(const std::string& text)
    {
        auto[tokens, tokenizer_error] = tokenize_string(text);
        if(tokenizer_error)
        {
            return {NullType(), tokenizer_error};
        }
        //std::cout << tokens << "\n";
        Parser parser;
        auto[ast, parser_error] = parser.parse_tokens(tokens);
        if(parser_error)
        {
            return {NullType(), parser_error};
        }
        //std::cout << ast << "\n";
        Interpreter interpreter;
        Variant result = interpreter.travel_node(ast);
        return {result};
    }
} // namespace SourDo

int main()
{
    std::string text;
    while(true)
    {
        std::cout << "SourDo > ";
        std::getline(std::cin, text);
        auto[result, error] = SourDo::interpret(text);
        if(error)
        {
            std::cout << error.value() << std::endl;
            continue;
        }
        std::cout << result << std::endl;
    }
    return 0;
}
