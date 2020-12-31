#include "DataTypes/Token.hpp"

#include <unordered_map>

namespace SourDo {
    Token::Token(Type type, std::string value)
        : type(type), value(value)
    {
    }

    std::ostream& operator<<(std::ostream& os, const Token::Type& type)
    {
        static std::unordered_map<Token::Type, const char*> strings = 
        {
            {Token::Type::NONE,     "None"  },
            {Token::Type::INT,      "Int"   },
            {Token::Type::FLOAT,    "Float" },
            {Token::Type::PLUS,     "Plus"  },
            {Token::Type::MINUS,    "Minus" },
            {Token::Type::TK_EOF,   "Eof"   }
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

} // namespace SourDo
