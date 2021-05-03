#include "Interpreter.hpp"

#include <unordered_map>
#include <sstream>

namespace sourdo
{
    static const std::vector<std::string> KEYWORDS = 
    {
        "var", "func",
        "return", "break", "continue",
        "if", "elif", "else", 
        "for", "while", "loop",
        "then", "do", "end" 
    };

    TokenizerReturn tokenize_string(const std::string& text, const std::string& file_name)
    {
        std::vector<Token> tokens;
        Position file_position = Position(1, 0, file_name);
        char current_char;

        for(int i = 0; i < text.size(); i++)
        {
            current_char = text[i];
            file_position.column++;
            if(current_char == '\n')
            {
                tokens.emplace_back(Token::Type::NEW_LINE, file_position);
                file_position.line++;
                file_position.column = 0;
            }
            if(isspace(current_char))
            {
                continue;
            }
            if(std::isdigit(current_char))
            {
                Position saved_position = file_position;
                std::string number_string;
                bool has_dot = false;
                while(i < text.size() && (std::isdigit(current_char) || current_char == '.'))
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
                    file_position.column++;
                    current_char = text[i]; 
                }
                i--;
                file_position.column--;
                
                tokens.emplace_back(Token::Type::NUMBER_LITERAL, saved_position, number_string);
            }
            else if(std::isalpha(current_char) || current_char == '_')
            {
                Position saved_position = file_position;
                std::string identifier_string;
                while(i < text.size() && (std::isalpha(current_char) || std::isdigit(current_char) || current_char == '_'))
                {
                    identifier_string += current_char;
                    i++;
                    file_position.column++;
                    current_char = text[i];
                }
                i--;
                file_position.column--;

                if(identifier_string == "or")
                {
                    tokens.emplace_back(Token::Type::LOGIC_OR, saved_position);
                }
                else if(identifier_string == "and")
                {
                    tokens.emplace_back(Token::Type::LOGIC_AND, saved_position);
                }
                else if(identifier_string == "not")
                {
                    tokens.emplace_back(Token::Type::LOGIC_NOT, saved_position);
                }
                else if(identifier_string == "false")
                {
                    tokens.emplace_back(Token::Type::BOOL_LITERAL, saved_position, "false");
                }
                else if(identifier_string == "true")
                {
                    tokens.emplace_back(Token::Type::BOOL_LITERAL, saved_position, "true");
                }
                else if(identifier_string == "null")
                {
                    tokens.emplace_back(Token::Type::NULL_LITERAL, saved_position);
                }
                else if(std::find(KEYWORDS.begin(), KEYWORDS.end(), identifier_string) != KEYWORDS.end())
                {
                    tokens.emplace_back(Token::Type::KEYWORD, saved_position, identifier_string);
                }
                else
                {
                    tokens.emplace_back(Token::Type::IDENTIFIER, saved_position, identifier_string);
                }
            }
            else
            {
                switch(current_char)
                {
                    case '"':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;
                        current_char = text[i];
                        std::string str;
                        while(i < text.size() && current_char != '"' && current_char != '\n')
                        {
                            str += current_char;
                            i++;
                            file_position.column++;
                            current_char = text[i];
                        }
                        if(current_char == '\n')
                        {
                            std::stringstream ss;
                            ss << file_position << "Strings cannot be multiline";
                            return {{}, ss.str()};
                        }
                        else if(current_char != '"')
                        {
                            std::stringstream ss;
                            ss << file_position << "Expected '\"' to close the string that was begun at "
                                    << "(Ln " << saved_position.line << ", Col " << saved_position.column << ")";
                            return {{}, ss.str()};
                        }
                        
                        tokens.emplace_back(Token::Type::STRING_LITERAL, str);
                        break;
                    }
                    case '+':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;
                        current_char = text[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::ASSIGN_ADD, saved_position);
                            break;
                        }
                        i--;
                        file_position.column--;
                        
                        tokens.emplace_back(Token::Type::ADD, saved_position);
                        break;
                    }
                    case '-':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;
                        current_char = text[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::ASSIGN_SUB, saved_position);
                            break;
                        }
                        else if(current_char == '-')
                        {
                            i++;
                            file_position.column++;
                            current_char = text[i];
                            while(i < text.size() && current_char != '\n')
                            {
                                i++;
                                file_position.column++;
                                current_char = text[i];
                            }
                            break;
                        }
                        else if(current_char == '*')
                        {
                            do
                            {
                                i++;
                                file_position.column++;
                                current_char = text[i];

                                if(current_char == '\n')
                                {
                                    file_position.column = 0;
                                    file_position.line++;
                                }

                                if(current_char == '*')
                                {
                                    i++;
                                    file_position.column++;
                                    current_char = text[i];
                                    if(i < text.size() && text[i] == '-')
                                    {
                                        break;
                                    }
                                }
                            }
                            while(i < text.size());

                            if(current_char != '-')
                            {
                                std::stringstream ss;
                                ss << file_position << "Expected '*-' to close comment that was begun at (Ln " << saved_position.line << ", Col " << saved_position.column << ") ";
                                return { {}, ss.str()};
                            }
                            i++;
                            file_position.column++;
                            current_char = text[i];

                            break;
                        }
                        i--;
                        file_position.column--;
                        
                        tokens.emplace_back(Token::Type::SUB, saved_position);
                        break;
                    }
                    case '*':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;
                        current_char = text[i];
                        if(current_char == '*')
                        {
                            tokens.emplace_back(Token::Type::POW, saved_position);
                            break;
                        }
                        else if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::ASSIGN_MUL, saved_position);
                            break;
                        }
                        i--;
                        file_position.column--;

                        tokens.emplace_back(Token::Type::MUL, saved_position);
                        break;
                    }
                    case '/':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;
                        current_char = text[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::ASSIGN_DIV, saved_position);
                            break;
                        }
                        i--;
                        file_position.column--;
                        
                        tokens.emplace_back(Token::Type::DIV, saved_position);
                        break;
                    }
                    case ',':
                    {
                        tokens.emplace_back(Token::Type::COMMA, file_position);
                        break;
                    }
                    case '.':
                    {
                        tokens.emplace_back(Token::Type::DOT, file_position);
                        break;
                    }
                    case ':':
                    {
                        tokens.emplace_back(Token::Type::COLON, file_position);
                        break;
                    }
                    case '>':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;

                        current_char = text[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::GREATER_EQUAL, saved_position);
                            break;
                        }
                        i--;
                        file_position.column--;

                        tokens.emplace_back(Token::Type::GREATER_THAN, saved_position);
                        break;
                    }
                    case '<':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;

                        current_char = text[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::LESS_EQUAL, saved_position);
                            break;
                        }
                        i--;
                        file_position.column--;

                        tokens.emplace_back(Token::Type::LESS_THAN, saved_position);
                        break;
                    }
                    case '=':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;
                        current_char = text[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::EQUAL, saved_position);
                            break;
                        }
                        i--;
                        file_position.column--;

                        tokens.emplace_back(Token::Type::ASSIGN, saved_position);
                        break;
                    }
                    case '!':
                    {
                        Position saved_position = file_position;
                        i++;
                        file_position.column++;
                        current_char = text[i];
                        if(current_char == '=')
                        {
                            tokens.emplace_back(Token::Type::NOT_EQUAL, saved_position);
                            break;
                        }
                        i--;

                        std::stringstream ss;
                        ss << file_position << " Unexpected character: '" << current_char << "'";

                        return { {}, ss.str()};
                        break;
                    }
                    case '(':
                    {
                        tokens.emplace_back(Token::Type::LPAREN, file_position);
                        break;
                    }
                    case ')':
                    {
                        tokens.emplace_back(Token::Type::RPAREN, file_position);
                        break;
                    }
                    case '[':
                    {
                        tokens.emplace_back(Token::Type::LBRACKET, file_position);
                        break;
                    }
                    case ']':
                    {
                        tokens.emplace_back(Token::Type::RBRACKET, file_position);
                        break;
                    }
                    case '{':
                    {
                        tokens.emplace_back(Token::Type::LBRACE, file_position);
                        break;
                    }
                    case '}':
                    {
                        tokens.emplace_back(Token::Type::RBRACE, file_position);
                        break;
                    }
                    default:
                    {
                        std::stringstream ss;
                        ss << file_position << " Unexpected character: '" << current_char << "'";

                        return { {}, ss.str()};
                        break;
                    }
                }
            }
        }

        tokens.emplace_back(Token::Type::TK_EOF, file_position);

        return { std::move(tokens), {}};
    }
} // namespace sourdo
