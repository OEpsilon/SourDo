#include "Token.hpp"

#include <unordered_map>
#include <ostream>

namespace sourdo
{
    std::ostream& operator<<(std::ostream& os, Token::Type type)
    {
        static std::unordered_map<Token::Type, const char*> strings = {
            //  Type                            String
            {   Token::Type::NONE,              "NONE"              },
            {   Token::Type::KEYWORD ,          "KEYWORD"           },
            {   Token::Type::IDENTIFIER ,       "IDENTIFIER"        },
            {   Token::Type::INT_LITERAL,       "INT_LITERAL"       },
            {   Token::Type::FLOAT_LITERAL,     "FLOAT_LITERAL"     },
            {   Token::Type::ADD,               "ADD"               },
            {   Token::Type::SUB,               "SUB"               },
            {   Token::Type::MUL,               "MUL"               },
            {   Token::Type::DIV,               "DIV"               },
            {   Token::Type::POW,               "POW"               },

            {   Token::Type::ASSIGN,            "ASSIGN"            },

            {   Token::Type::LPAREN,            "LPAREN"            },
            {   Token::Type::RPAREN,            "RPAREN"            },
            {   Token::Type::TK_EOF,            "EOF"               },
        };

        return os << strings[type];
    }

    std::ostream& operator<<(std::ostream& os, const Token& token)
    {
        os << "[" << token.type;
        if(token.value != "")
        {
            os << ": " << token.value;  
        }
        os << "]";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens)
    {
        os << "[";
        for(int i = 0; i < tokens.size(); i++)
        {
            os << tokens[i];
            if(i < tokens.size() - 1)
            {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }
} // namespace sourdo