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
                bool has_dot = false;
                while(i < string.size() && (std::isdigit(current_char) || current_char == '.'))
                {
                    if(std::isdigit(current_char))
                    {
                        number_string += current_char;
                    }
                    else
                    {
                        if(has_dot)
                        {
                            break;
                        }
                        number_string += current_char;
                        has_dot = true;
                    }
                    i++;
                    current_char = string[i]; 
                }
                i--;
                
                tokens.emplace_back(Token::Type::NUMBER_LITERAL, number_string);
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

                if(identifier_string == "or")
                {
                    tokens.emplace_back(Token::Type::LOGIC_OR);
                }
                else if(identifier_string == "and")
                {
                    tokens.emplace_back(Token::Type::LOGIC_AND);
                }
                else if(identifier_string == "not")
                {
                    tokens.emplace_back(Token::Type::LOGIC_NOT);
                }
                else if(identifier_string == "false")
                {
                    tokens.emplace_back(Token::Type::BOOL_LITERAL, "false");
                }
                else if(identifier_string == "true")
                {
                    tokens.emplace_back(Token::Type::BOOL_LITERAL, "true");
                }
                else if(identifier_string == "null")
                {
                    tokens.emplace_back(Token::Type::NULL_LITERAL);
                }
                else if(std::find(KEYWORDS.begin(), KEYWORDS.end(), identifier_string) != KEYWORDS.end())
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
                    case '>':
                    {
                        i++;
                        current_char = string[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::GREATER_EQUAL);
                            break;
                        }
                        i--;

                        tokens.emplace_back(Token::Type::GREATER_THAN);
                        break;
                    }
                    case '<':
                    {
                        i++;
                        current_char = string[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::LESS_EQUAL);
                            break;
                        }
                        i--;

                        tokens.emplace_back(Token::Type::LESS_THAN);
                        break;
                    }
                    case '=':
                    {
                        i++;
                        current_char = string[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::EQUAL);
                            break;
                        }
                        i--;

                        tokens.emplace_back(Token::Type::ASSIGN);
                        break;
                    }
                    case '!':
                    {
                        i++;
                        current_char = string[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::NOT_EQUAL);
                            break;
                        }
                        i--;

                        tokens.emplace_back(Token::Type::LOGIC_NOT);
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
