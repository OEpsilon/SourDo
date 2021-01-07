#include "DataTypes/Token.hpp"

#include <unordered_map>
#include <sstream>

namespace SourDo {
    Token::Token(Type type, const Position& position, const std::string& value)
        : type(type), position(position), value(value)
    {
    }

    std::ostream& operator<<(std::ostream& os, const Token::Type& type)
    {
        static std::unordered_map<Token::Type, const char*> strings = 
        {
            {Token::Type::NONE,     "None"      },
            {Token::Type::INT,      "Int"       },
            {Token::Type::FLOAT,    "Float"     },
            {Token::Type::PLUS,     "Plus"      },
            {Token::Type::MINUS,    "Minus"     },
            {Token::Type::MULTI,    "Multi"     },
            {Token::Type::DIVIDE,   "Divide"    },
            {Token::Type::POWER,    "Power"     },
            {Token::Type::L_PAREN,  "LParen"    },
            {Token::Type::R_PAREN,  "RParen"    },
            {Token::Type::TK_EOF,   "Eof"       },
        };

        os << strings[type];
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Token& token)
    {
        if(token.value != "")
        {
            os << "[" << token.type << ": " << token.value << "]";
        }
        else
        {
            os << "[" << token.type << "]";
        }
        
        return os;
    }

    std::string operator+(const std::string& string, const Token::Type& type)
    {
        std::stringstream ss;
        ss << string << type;
        return ss.str();
    }

} // namespace SourDo
