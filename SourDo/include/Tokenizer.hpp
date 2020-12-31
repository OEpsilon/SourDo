#pragma once

#include "DataTypes/Token.hpp"

#include <vector>
#include <cctype>

namespace SourDo {
    std::vector<Token> tokenize_string(const std::string& text);

    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens);
} // namespace SourDo
