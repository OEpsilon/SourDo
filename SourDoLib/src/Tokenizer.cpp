#include "Interpreter.hpp"

#include <unordered_map>
#include <sstream>

#include "ConsoleColors.hpp"

namespace sourdo
{
    TokenizerReturn tokenize_string(const std::string& string)
    {
        std::vector<Token> tokens;
        char current_char;

        for(int i = 0; i < string.size(); i++)
        {
            current_char = string[i]; 
            if(isspace(current_char))
            {
                continue;
            }
            if(std::isdigit(current_char))
            {
                std::string number_string;
                bool is_float = false;
                while(i < string.size() && (std::isdigit(current_char) || current_char == '.'))
                {
                    current_char = string[i]; 
                    if(std::isdigit(current_char))
                    {
                        number_string += current_char;
                    }
                    else if(current_char == '.')
                    {
                        number_string += current_char;
                        is_float = true;
                    }
                    i++;
                }
                i--;
                
                if(is_float)
                {
                    tokens.emplace_back(Token::Type::FLOAT_LITERAL, number_string);
                }
                else
                {
                    tokens.emplace_back(Token::Type::INT_LITERAL, number_string);
                }
            }
            else
            {
                switch(current_char)
                {
                    case '+':
                    {
                        tokens.emplace_back(Token::Type::ADD);
                        break;
                    }
                    case '-':
                    {
                        tokens.emplace_back(Token::Type::SUB);
                        break;
                    }
                    case '*':
                    {
                        i++;
                        current_char = string[i];
                        if(current_char == '*')
                        {
                            tokens.emplace_back(Token::Type::POW);
                            i++;
                            break;
                        }
                        i--;

                        tokens.emplace_back(Token::Type::MUL);
                        break;
                    }
                    case '/':
                    {
                        tokens.emplace_back(Token::Type::DIV);
                        break;
                    }
                    case '(':
                    {
                        tokens.emplace_back(Token::Type::LPAREN);
                        break;
                    }
                    case ')':
                    {
                        tokens.emplace_back(Token::Type::RPAREN);
                        break;
                    }
                    default:
                    {
                        using namespace std::string_literals;
                        
                        std::stringstream ss;
                        ss << "Unexpected character: '" << current_char << "'";

                        return { {}, {ss.str()}};
                        break;
                    }
                }
            }
        }

        tokens.emplace_back(Token::Type::TK_EOF);

        return { std::move(tokens), {}};
    }
} // namespace sourdo
