#pragma once

#include "Errors/Position.hpp"

#include <iostream>
#include <string>

namespace SourDo {
    struct Token 
    {
        enum class Type : uint32_t
        {
            NONE = 0,
            INT, FLOAT,
            PLUS, MINUS,
            TK_EOF,
        };

        Type type;
        std::string value;
        Position position;

        Token(Type type, const Position& position, const std::string& value = "");
        Token() = default;
    };

    std::ostream& operator<<(std::ostream& os, const Token::Type& type);
    std::ostream& operator<<(std::ostream& os, const Token& token);

    std::string operator+(const std::string& string, const Token::Type& type);
} // namespace SourDo
