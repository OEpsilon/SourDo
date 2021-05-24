#include "Interpreter.hpp"

#include <unordered_map>
#include <sstream>

namespace sourdo
{
    static std::unordered_map<std::string, Token::Type> KEYWORDS = 
    {
        {"var",         Token::Type::VAR            }, 
        {"func",        Token::Type::FUNC           },
        {"return",      Token::Type::RETURN         }, 
        {"break",       Token::Type::BREAK          }, 
        {"continue",    Token::Type::CONTINUE       },
        {"if",          Token::Type::IF             }, 
        {"elif",        Token::Type::ELIF           }, 
        {"else",        Token::Type::ELSE           }, 
        {"for",         Token::Type::FOR            }, 
        {"while",       Token::Type::WHILE          }, 
        {"loop",        Token::Type::LOOP           },
        {"then",        Token::Type::THEN           }, 
        {"do",          Token::Type::DO             }, 
        {"end",         Token::Type::END            },

        {"is",          Token::Type::IS             },
        {"or",          Token::Type::OR             }, 
        {"and",         Token::Type::AND            }, 
        {"not",         Token::Type::NOT            }, 

        {"null",        Token::Type::NULL_LITERAL   }, 
        {"true",        Token::Type::BOOL_TRUE      }, 
        {"false",       Token::Type::BOOL_FALSE     }, 
    };

    static std::vector<std::string> BUILTIN_TYPES = 
    {
        "null_type", "number", "bool", "string", 
        "sourdo_function", "cpp_function", "function",
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
            switch(current_char)
            {
                case '\n':
                {
                    tokens.emplace_back(Token::Type::NEW_LINE);
                    file_position.column = 0;
                    file_position.line++;
                    break;
                }
                case ' ':
                case '\t':
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
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
                    break;
                }
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                case 'G':
                case 'H':
                case 'I':
                case 'J':
                case 'K':
                case 'L':
                case 'M':
                case 'N':
                case 'O':
                case 'P':
                case 'Q':
                case 'R':
                case 'S':
                case 'T':
                case 'U':
                case 'V':
                case 'W':
                case 'X':
                case 'Y':
                case 'Z':
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                case 'h':
                case 'i':
                case 'j':
                case 'k':
                case 'l':
                case 'm':
                case 'n':
                case 'o':
                case 'p':
                case 'q':
                case 'r':
                case 's':
                case 't':
                case 'u':
                case 'v':
                case 'w':
                case 'x':
                case 'y':
                case 'z':
                case '_':
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
                    if(KEYWORDS.find(identifier_string) != KEYWORDS.end())
                    {
                        tokens.emplace_back(KEYWORDS[identifier_string], saved_position);
                    }
                    else if(std::find(BUILTIN_TYPES.begin(), BUILTIN_TYPES.end(), identifier_string) != BUILTIN_TYPES.end())
                    {
                        tokens.emplace_back(Token::Type::BUILTIN_TYPE, saved_position, identifier_string);
                    }
                    else
                    {
                        tokens.emplace_back(Token::Type::IDENTIFIER, saved_position, identifier_string);
                    }
                    break;
                }
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
                case '%':
                {
                    tokens.emplace_back(Token::Type::MOD, file_position);
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

        tokens.emplace_back(Token::Type::TK_EOF, file_position);

        return { std::move(tokens), {}};
    }
} // namespace sourdo
