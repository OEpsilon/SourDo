#include "Tokenizer.hpp"

namespace SourDo {
    std::vector<Token> tokenize_string(const std::string& text)
    {
        std::vector<Token> tokens;
        uint32_t position = 0;
        char current_char = '\0';
        while(position < text.length())
        {
            current_char = text[position];
            if(isblank(current_char))
            {
                position++;
                continue;
            }
            else if(isdigit(current_char))
            {
                // Get an int or float literal token.
                std::string number_string = "";
                uint8_t dot_count = 0;
                
                while(position < text.length() && (isdigit(current_char) || 
                        current_char == '.'))
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
                    position++;
                    current_char = text[position];
                }

                if(dot_count == 0)
                {
                    tokens.emplace_back(Token::Type::INT, number_string);
                }
                else
                {  
                    tokens.emplace_back(Token::Type::FLOAT, number_string);
                }
            }
            else
            {
                switch (current_char)
                {
                case '+':
                    tokens.emplace_back(Token::Type::PLUS);
                    position++;
                    break;
                case '-':
                    tokens.emplace_back(Token::Type::MINUS);
                    position++;
                    break;
                }
            }
            
        }
        tokens.push_back(Token::Type::TK_EOF);
        return tokens;
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
