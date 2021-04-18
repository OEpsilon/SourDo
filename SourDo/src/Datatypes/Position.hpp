#pragma once

#include <cstdint>
#include <string>
#include <ostream>

namespace sourdo
{
    struct Position
    {
        Position(uint32_t line = 0, uint32_t column = 0, const std::string& file_name = "Unknown")
            : line(line), column(column), file_name(file_name)
        {
        }

        uint32_t line, column;
        std::string file_name;
    };

    inline std::ostream& operator<<(std::ostream& os, const Position& position)
    {
        os << position.file_name << "(Ln " << position.line << ", Col " << position.column << ") ";
        return os;
    }
    
} // namespace sourdo
