#pragma once

#include <string>
#include <vector>

namespace sourdo
{
    struct Token
    {
        enum class Type : uint32_t
        {
            NONE,
            INT_LITERAL, FLOAT_LITERAL,
            ADD, SUB, MUL, DIV, POW,
            LPAREN, RPAREN,
            TK_EOF,
        };

        Token(Type type = Type::NONE, const std::string& value = "")
            : type(type), value(value)
        {
        }

        bool operator==(const Token& other)
        {
            return type == other.type && value == other.value;
        }

        bool operator!=(const Token& other)
        {
            return type != other.type && value != other.value;
        }

        Type type;
        std::string value;
    };

    std::ostream& operator<<(std::ostream& os, Token::Type type);
    std::ostream& operator<<(std::ostream& os, const Token& token);
    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens);
} // namespace sourdo
