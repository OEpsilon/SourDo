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
            KEYWORD, IDENTIFIER,
            NUMBER_LITERAL, BOOL_LITERAL, NULL_LITERAL,
            ADD, SUB, MUL, DIV, POW,
            LESS_THAN, GREATER_THAN, LESS_EQUAL, GREATER_EQUAL, EQUAL, NOT_EQUAL,
            LOGIC_OR, LOGIC_AND, LOGIC_NOT,
            ASSIGN,
            LPAREN, RPAREN,
            NEW_LINE, 
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
            return !(*this == other);
        }

        Type type;
        std::string value;
    };

    std::ostream& operator<<(std::ostream& os, Token::Type type);
    std::ostream& operator<<(std::ostream& os, const Token& token);
    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens);
} // namespace sourdo
