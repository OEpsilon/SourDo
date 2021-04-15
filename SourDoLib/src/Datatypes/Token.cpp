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
            {   Token::Type::NUMBER_LITERAL,    "NUMBER_LITERAL"    },
            {   Token::Type::BOOL_LITERAL,      "BOOL_LITERAL"      },
            {   Token::Type::NULL_LITERAL,      "NULL_LITERAL"      },

            {   Token::Type::ADD,               "ADD"               },
            {   Token::Type::SUB,               "SUB"               },
            {   Token::Type::MUL,               "MUL"               },
            {   Token::Type::DIV,               "DIV"               },
            {   Token::Type::POW,               "POW"               },

            {   Token::Type::LESS_THAN,         "LESS_THAN"         },
            {   Token::Type::GREATER_THAN,      "GREATER_THAN"      },
            {   Token::Type::LESS_EQUAL,        "LESS_EQUAL"        },
            {   Token::Type::GREATER_EQUAL,     "GREATER_EQUAL"     },
            {   Token::Type::EQUAL,             "EQUAL"             },
            {   Token::Type::NOT_EQUAL,         "NOT_EQUAL"         },

            {   Token::Type::LOGIC_OR,          "LOGIC_OR"          },
            {   Token::Type::LOGIC_AND,         "LOGIC_AND"         },
            {   Token::Type::LOGIC_NOT,         "LOGIC_NOT"         },

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
