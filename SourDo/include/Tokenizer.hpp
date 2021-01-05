#pragma once

#include "DataTypes/Token.hpp"

#include <vector>
#include <optional>
#include "Errors/Error.hpp"

namespace SourDo {
    struct TokenizerResult 
    {
        std::vector<Token> result;
        std::optional<Error> error;
    };

    TokenizerResult tokenize_string(const std::string& text);

    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens);
} // namespace SourDo
