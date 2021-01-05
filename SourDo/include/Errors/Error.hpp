#pragma once

#include "Position.hpp"

#include <string>
#include <ostream>

namespace SourDo {
    class Error
    {
    public:
        Error(const std::string& issue, const Position& position)
            : issue(issue), position(position)
        {
        }
    private:
        friend std::ostream& operator<<(std::ostream& os, const Error& error);
    private:
        std::string issue;
        Position position;
    };

    inline std::ostream& operator<<(std::ostream& os, const Error& error)
    {
        os << "Error(Ln " << error.position.line + 1 << ", Col "
                << error.position.column + 1 << "): " << error.issue;
        return os;
    }
}