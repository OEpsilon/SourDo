#pragma once

#include "Position.h"

namespace srdo {
    static const std::vector<std::string> KEYWORDS = {
        "let", "const", "func",
        "if", "elif", "else", "println",
        // Data Types
        "bool", "int", "float", "double",
        // Logical Operators(Keywords)
        "not", "and", "or", "xor",
    };

    enum class TType
    {
        Unknown = 0,
        Keyword, Id,
        // Literals
        Int, Float, Double, Bool,
        // Arithmetic Operators
        Plus, Minus, Mul, Div, Pow,
        // Assignment Operators
        Assign,
        // Logical Operators(Symbols)
        Not, And, Or, Xor,
        // Bitwise Operators(Symbols)
        BitNot, BitAnd, BitOr, BitXor, BitShiftL, BitShiftR,
        // Comparison Operators
        Eq, Ne, Gte, Lte,
        // Braces
        LParen, RParen, LAngle, RAngle,
        LCurly, RCurly, LSquare, RSquare,
        // Misc.
        Terminal, Eof,
    };

    std::ostream& operator<<(std::ostream& os, const TType& tt);

    class Token
    {
    public:
        Token(const Position& pos, TType type, std::string value = "")
            : pos(pos), type(type), value(value)
        {
        }

        const TType type;
        const std::string value;
        const Position pos;
    };

    std::ostream& operator<<(std::ostream& os, const Token& tok);

    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens);

}