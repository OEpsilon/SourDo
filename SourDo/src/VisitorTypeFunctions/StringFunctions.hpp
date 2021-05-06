#pragma once

#include "SourDo/SourDo.hpp"
#include "SourDo/Errors.hpp"

#include <string>

namespace sourdo
{
    inline bool string_length(Data& data)
    {
        sourdo::check_arg_count(data, 1);
        sourdo::check_is_string(data, 1);
        std::string self = data.value_to_string(1);
        data.push_number(self.size());
        return true;
    }
} // namespace sourdo
