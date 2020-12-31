#pragma once

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

        Token(Type type, std::string value = "");
        Token() = default;

        Type type;
        std::string value;
    };

    std::ostream& operator<<(std::ostream& os, const Token::Type& type);
    std::ostream& operator<<(std::ostream& os, const Token& token);
} // namespace SourDo
