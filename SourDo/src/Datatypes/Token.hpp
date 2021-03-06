#pragma once

#include <string>
#include <vector>

#include "Position.hpp"

namespace sourdo
{
    struct Token
    {
        enum class Type : uint32_t
        {
            NONE,
            BUILTIN_TYPE,

            VAR, CONST, FUNC, 
            CLASS, PRIVATE, SET, GET, EXTENDS,
            RETURN, BREAK, CONTINUE,
            IF, ELIF, ELSE, 
            FOR, WHILE, LOOP,
            THEN, DO, END,
            ISA, OR, AND, NOT,
            
            NUMBER_LITERAL, STRING_LITERAL, BOOL_TRUE, BOOL_FALSE, NULL_LITERAL, IDENTIFIER,
            ADD, SUB, MUL, DIV, MOD, POW,
            LESS_THAN, GREATER_THAN, LESS_EQUAL, GREATER_EQUAL, EQUAL, NOT_EQUAL,
            COMMA, DOT, COLON,
            ASSIGN, ASSIGN_ADD, ASSIGN_SUB, ASSIGN_MUL, ASSIGN_DIV,
            LPAREN, RPAREN, LBRACKET, RBRACKET, LBRACE, RBRACE,
            NEW_LINE, 
            TK_EOF,
        };

        Token(Type type, Position position = {}, const std::string& value = "")
            : type(type), position(position), value(value)
        {
        }

        Token(Type type, const std::string& value)
            : type(type), position(), value(value)
        {
        }

        Token()
            : type(Type::NONE), position(), value()
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
        Position position;
        std::string value;
    };

    std::ostream& operator<<(std::ostream& os, Token::Type type);
    std::ostream& operator<<(std::ostream& os, const Token& token);
    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens);
} // namespace sourdo
