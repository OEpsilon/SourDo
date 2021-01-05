#include "Tokenizer.hpp"

#include <cctype>

namespace SourDo {
    TokenizerResult tokenize_string(const std::string& text)
    {
        Position position(0, 0, 0);
        std::vector<Token> tokens;
        char current_char = '\0';
        while(position.index < text.length())
        {
            current_char = text[position.index];
            if(isblank(current_char))
            {
                position.advance(current_char);
                continue;
            }
            else if(isdigit(current_char))
            {
                // Get an int or float literal token.
                std::string number_string = "";
                uint8_t dot_count = 0;
                
                while(position.index < text.length() && (isdigit(current_char) 
                        || current_char == '.'))
                {
                    if(current_char == '.')
                    {
                        if(dot_count == 1)
                        {
                            break;
                        }
                        number_string += current_char;
                        dot_count++;
                    }
                    else
                    {
                        number_string += current_char;
                    }
                    position.advance(current_char);
                    current_char = text[position.index];
                }

                if(dot_count == 0)
                {
                    tokens.emplace_back(Token::Type::INT, position, number_string);
                }
                else
                {  
                    tokens.emplace_back(Token::Type::FLOAT, position, number_string);
                }
            }
            else
            {
                switch (current_char)
                {
                    case '+':
                        tokens.emplace_back(Token::Type::PLUS, position);
                        position.advance(current_char);
                        break;
                    case '-':
                        tokens.emplace_back(Token::Type::MINUS, position);
                        position.advance(current_char);
                        break;
                    default:
                    {
                        using namespace std::string_literals;
                        return {tokens, Error(
                                "Unexpected character '"s + current_char + "'", 
                                position)};
                        break;
                    }
                }
            }
            
        }
        tokens.emplace_back(Token::Type::TK_EOF, position);
        return {tokens};
    }

    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens)
    {
        os << '[';
        for(int i = 0; i < tokens.size(); i++)
        {
            os << tokens[i];
            if(i < tokens.size() - 1)
            {
                os << ", ";
            }
        }

        os << ']';
        return os;
    }
} // namespace SourDo
