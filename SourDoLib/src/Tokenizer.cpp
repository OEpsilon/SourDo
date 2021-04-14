#include "Interpreter.hpp"

#include <unordered_map>
#include <sstream>

#include "ConsoleColors.hpp"

namespace sourdo
{
    static const std::vector<std::string> KEYWORDS = { "var" };

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
                    if(std::isdigit(current_char))
                    {
                        number_string += current_char;
                    }
                    else
                    {
                        number_string += current_char;
                        is_float = true;
                    }
                    i++;
                    current_char = string[i]; 
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
            else if(std::isalpha(current_char) || current_char == '_')
            {
                std::string identifier_string;
                while(i < string.size() && (std::isalpha(current_char) || current_char == '_'))
                {
                    identifier_string += current_char;
                    i++;
                    current_char = string[i];
                }
                i--;

                if(std::find(KEYWORDS.begin(), KEYWORDS.end(), identifier_string) != KEYWORDS.end())
                {
                    tokens.emplace_back(Token::Type::KEYWORD, identifier_string);
                }
                else
                {
                    tokens.emplace_back(Token::Type::IDENTIFIER, identifier_string);
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
                    case ':':
                    {
                        i++;
                        current_char = string[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::ASSIGN);
                            break;
                        }
                        i--;

                        std::stringstream ss;
                        ss << "Unexpected character: '" << current_char << "'. Did you mean '='?";

                        return { {}, ss.str() };
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
                        std::stringstream ss;
                        ss << "Unexpected character: '" << current_char << "'";

                        return { {}, ss.str()};
                        break;
                    }
                }
            }
        }

        tokens.emplace_back(Token::Type::TK_EOF);

        return { std::move(tokens), {}};
    }
} // namespace sourdo
